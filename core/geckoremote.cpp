/*
Copyright 2011 Bastian Loeher, Roland Wirth

This file is part of GECKO.

GECKO is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

GECKO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "geckoremote.h"
#include "runmanager.h"
#include "systeminfo.h"

#include <stdexcept>
#include <iostream>

#include <QUdpSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <QNetworkInterface>
#include <QStringList>

GeckoRemote::GeckoRemote (uint16_t localport)
    : LocalPort_ (localport)
    , TcpSock_ (NULL)
    , TcpServ_ (NULL)
    , TcpControllerSock_ (NULL)
    , UdpSock_ (NULL)
    , Remote_ (QHostAddress::Null)
    , Controller_ (QHostAddress::Null)
    , Connected_ (false)
{
    UdpSock_ = new QUdpSocket (this);
    TcpSock_ = new QTcpSocket (this);
    TcpServ_ = new QTcpServer (this);

    if (!UdpSock_->bind (LocalPort_, QUdpSocket::ShareAddress)) {
        throw std::invalid_argument (QString ("could not bind port %1").arg (LocalPort_).toStdString());
    }

    TcpServ_->listen (QHostAddress::Any, LocalPort_ + 1);

    connect (UdpSock_, SIGNAL(readyRead ()), SLOT(readUdpDatagram ()));
    connect (TcpServ_, SIGNAL(newConnection()), SLOT(tcpServerNewConnection()));

    LocalAddrs_ = getLocalAddresses ();
}

GeckoRemote::~GeckoRemote () {
    if (Connected_)
        disconnectRemote ();
}

void GeckoRemote::setRemote (QHostAddress remote) {
    if (Connected_)
        disconnectRemote ();

    Remote_ = remote;
}

void GeckoRemote::startDiscover () {
    UdpSock_->writeDatagram (QByteArray ("ping"), QHostAddress::Broadcast, LocalPort_);
}

void GeckoRemote::connectRemote () {
    if (Remote_.isNull ())
        return;

    UdpSock_->writeDatagram (QByteArray ("QUERY connect"), Remote_, LocalPort_);
}

void GeckoRemote::disconnectRemote () {
    if (Remote_.isNull ())
        return;

    UdpSock_->writeDatagram (QByteArray ("QUERY disconnect"), Remote_, LocalPort_);
}

void GeckoRemote::startUpdate () {
    if (Remote_.isNull ())
        return;

    UdpSock_->writeDatagram (QByteArray ("QUERY update"), Remote_, LocalPort_);
}

void GeckoRemote::startRemoteRun () {
    if (Remote_.isNull ())
        return;

    UdpSock_->writeDatagram (QByteArray ("QUERY start"), Remote_, LocalPort_);
}

void GeckoRemote::stopRemoteRun () {
    if (Remote_.isNull ())
        return;

    UdpSock_->writeDatagram (QByteArray ("QUERY stop"), Remote_, LocalPort_);
}

void GeckoRemote::tcpServerNewConnection () {
    // we are already connected, so close any other incoming TCP connection
    // (too bad there is no function in QTcpServer to refuse a connection)
    if (TcpControllerSock_) {
        TcpServ_->nextPendingConnection ()->deleteLater ();
        return;
    }

    TcpControllerSock_ = TcpServ_->nextPendingConnection ();
    connect (TcpControllerSock_, SIGNAL (disconnected()), SLOT(tcpDisconnected()));
    Controller_ = TcpControllerSock_->peerAddress ();
}

void GeckoRemote::tcpDisconnected () {
    Controller_ = QHostAddress::Null;
    TcpControllerSock_->deleteLater ();
    TcpControllerSock_ = NULL;
}

void GeckoRemote::readUdpDatagram () {
    while(UdpSock_->hasPendingDatagrams ()) {
        QByteArray datagram;
        datagram.resize (UdpSock_->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        UdpSock_->readDatagram (datagram.data(), datagram.size(), &sender, &senderPort);

        //std::cout << "Received datagram from " << sender.toString().toStdString() << " on port " << (uint16_t)senderPort << std::endl;

        if(!LocalAddrs_.contains (sender)) processDatagram(datagram, sender, senderPort);
    }
}

void GeckoRemote::processDatagram (QByteArray datagram, QHostAddress sender, uint16_t senderPort)
{
    std::cout << "Data: " << std::hex;
    for(int i = 0; i < datagram.size(); i++)
    {
        std::cout << datagram.at(i);
    }
    std::cout << std::dec << std::endl;

    QString data(datagram.data());

    if(data.startsWith("ping"))
    {
        QByteArray ret = "pong";
        UdpSock_->writeDatagram(ret.data(),ret.size(),sender,senderPort);
    }
    else if(data.startsWith("pong"))
    {
        std::cout << "Discovered gecko on " << sender.toString().toStdString() << std::endl;
        if(!RemoteAddrs_.contains (sender))
        {
            RemoteAddrs_.insert(sender);
            emit discovered (sender);
        }
    }
    else if(data.startsWith("QUERY"))
    {
        QStringList args = data.split(QRegExp(" "));
        processQuery(args, sender);
    }
    else if(data.startsWith("POST"))
    {
        QStringList args = data.split(QRegExp(" "));
        processPost(args, sender);
    }
}

void GeckoRemote::processQuery(QStringList query, QHostAddress sender)
{
//    std::cout << "Processing query ";
//    foreach(QString part, query)
//    {
//        std::cout << part.toStdString() << ". ";
//    }
//    std::cout << std::endl;

    if(query.size() < 2) return;

    if(query.at(1) == "update")
    {
        QByteArray datagram = "POST ";
        datagram += "update ";
        datagram += "state ";
        QByteArray state(RunManager::ref().getStateString().toAscii());
        datagram += state;
        UdpSock_->writeDatagram(datagram, sender, LocalPort_);

        datagram = "POST ";
        datagram += "update ";
        datagram += "runname \"";
        datagram += RunManager::ref ().getRunName ();
        datagram += "\"";
        UdpSock_->writeDatagram(datagram, sender, LocalPort_);

        datagram = "POST ";
        datagram += "update ";
        datagram += "start ";
        datagram += RunManager::ref ().getStartTime ().toString();
        UdpSock_->writeDatagram(datagram, sender, LocalPort_);

        datagram = "POST ";
        datagram += "update ";
        datagram += "stop ";
        datagram += RunManager::ref ().getStopTime ().toString();
        UdpSock_->writeDatagram(datagram, sender, LocalPort_);

        datagram = "POST ";
        datagram += "update ";
        datagram += "numberofevents ";
        datagram += QString::number (RunManager::ref ().getEventCount ());
        UdpSock_->writeDatagram(datagram, sender, LocalPort_);

        datagram = "POST ";
        datagram += "update ";
        datagram += "eventrate ";
        datagram += QString::number (RunManager::ref ().getEventRate (), 'f', 2);
        UdpSock_->writeDatagram(datagram, sender, LocalPort_);

        datagram = "POST ";
        datagram += "update ";
        datagram += "info ";
        datagram += RunManager::ref ().getRunInfo ();
        UdpSock_->writeDatagram(datagram, sender, LocalPort_);

        datagram = "POST ";
        datagram += "update ";
        datagram += "cpu ";
        datagram += QString::number(int (RunManager::ref().getSystemInfo()->getCpuLoad ()*100));
        UdpSock_->writeDatagram(datagram, sender, LocalPort_);

        datagram = "POST update end";
        UdpSock_->writeDatagram (datagram, sender, LocalPort_);

    }
    else if(query.at(1) == "connect")
    {
        QByteArray datagram;
        datagram = "POST ";
        datagram += "connect ";

        if(RunManager::ref().isRemoteControlled())
        {
            datagram += "failed ";
            datagram += Controller_.toString();
        }
        else
        {
            datagram += "success ";

            RunManager::ref().setRemoteControlled(true);
            Controller_ = sender;
            emit remoteControlled (true, Controller_);
        }
        UdpSock_->writeDatagram(datagram, sender, LocalPort_);
    }
    else if(query.at(1) == "disconnect")
    {
        QByteArray datagram;
        datagram = "POST ";
        datagram += "disconnect ";

        if(!RunManager::ref().isRemoteControlled() || Controller_ != sender)
        {
            datagram += "failed ";
            datagram += Controller_.toString();
        }
        else
        {
            datagram += "success ";

            RunManager::ref().setRemoteControlled(false);
            Controller_ = QHostAddress::Null;
            emit remoteControlled (false, QHostAddress::Null);
        }
        UdpSock_->writeDatagram(datagram, sender, LocalPort_);
    }
    else if(query.at(1) == "start")
    {
        QByteArray datagram;
        datagram = "POST ";
        datagram += "start ";

        if(!RunManager::ref().isRunning() && sender == Controller_) {
            datagram += "success ";
            RunManager::ref ().start (tr ("remote start"));
        }
        else if(sender != Controller_){
            datagram += "failed ";
            datagram += Controller_.toString();
        }
        else if(RunManager::ref().isRunning()) {
            datagram += "state ";
            datagram += "running ";
        }
        UdpSock_->writeDatagram(datagram, sender, LocalPort_);
    }
    else if(query.at(1) == "stop")
    {
        QByteArray datagram;
        datagram = "POST ";
        datagram += "stop ";

        if(RunManager::ref().isRunning() && sender == Controller_) {
            datagram += "success ";
            RunManager::ref ().stop ();
        }
        else if(sender != Controller_){
            datagram += "failed ";
            datagram += Controller_.toString();
        }
        else if(!RunManager::ref().isRunning()) {
            datagram += "state ";
            datagram += "not_running";
        }
        UdpSock_->writeDatagram(datagram, sender, LocalPort_);
    }
    else if(query.at(1) == "set")
    {
        if(query.size() < 4) return;
    }
}

void GeckoRemote::processPost(QStringList post, QHostAddress sender)
{
    Q_UNUSED(sender)

//    std::cout << "Processing post ";
//    foreach(QString part, post)
//    {
//        std::cout << part.toStdString() << ". ";
//    }
//    std::cout << std::endl;

    if(post.size() < 2) return;

    if(post.at(1) == "update")
    {
        if(post.size() < 3) return;

        post.removeFirst();
        post.removeFirst();

        if(post.first() == "state")
        {
            post.removeFirst();
            processRemoteState(post);
        }
        else if(post.first() == "runname")
        {
            post.removeFirst();
            RemoteState_.runname = post.join(" ");
            // remove quotes around the name
            RemoteState_.runname.remove (0, 1);
            RemoteState_.runname.chop (1);
        }
        else if(post.first() == "start")
        {
            post.removeFirst();
            RemoteState_.starttime = QDateTime::fromString(post.join(" "));
        }
        else if(post.first() == "stop")
        {
            post.removeFirst();
            RemoteState_.stoptime = QDateTime::fromString(post.join(" "));
        }
        else if(post.first() == "numberofevents")
        {
            post.removeFirst();
            RemoteState_.nofevents = post.join(" ").toLong ();
        }
        else if(post.first() == "eventrate")
        {
            post.removeFirst();
            RemoteState_.eventrate = post.join(" ").toFloat ();
        }
        else if(post.first() == "info")
        {
            post.removeFirst();
            RemoteState_.runinfo = post.join(" ");
        }
        else if(post.first() == "cpu")
        {
            post.removeFirst();
            RemoteState_.cpuload = post.join(" ").toInt ();
        }
        else if(post.first() == "end")
        {
            emit updateComplete ();
        }
        else
        {
            post.clear();
            std::cerr << "Unknown update parameter" << std::endl;
        }
    }

    else if(post.at(1) == "connect")
    {
        if(post.size() < 3) return;

        post.removeFirst();
        post.removeFirst();

        if(post.first() == "failed")
        {
            if(post.size() > 1)
            {
                std::cout << "Error: Connection failed. Already connected to " << post.at(1).toStdString() << std::endl;
                emit connected (QHostAddress(post.at(1)));
            }
        }
        else if(post.first() == "success")
        {
            emit connected (QHostAddress::Null);
            TcpSock_->connectToHost (Remote_, LocalPort_ + 1, QAbstractSocket::ReadWrite);
        }
        else
        {
            post.clear();
            std::cerr << "Unknown connection reply from host." << std::endl;
        }
    }

    else if(post.at(1) == "disconnect")
    {
        if(post.size() < 3) return;

        post.removeFirst();
        post.removeFirst();

        if(post.first() == "failed")
        {
            if(post.size() > 1)
            {
                std::cout << "Error: Disconnecting failed. Only " << post.at(1).toStdString() << " can do that." << std::endl;
                emit disconnected (QHostAddress (post.at(1)));
            }
        }
        else if(post.first() == "success")
        {
            TcpSock_->disconnectFromHost ();
            emit disconnected (QHostAddress::Null);
        }
        else
        {
            post.clear();
            std::cerr << "Unknown disconnect reply from host." << std::endl;
        }
    }

    else if(post.at(1) == "start") {
        if(post.size() < 3) return;

        post.removeFirst();
        post.removeFirst();

        if(post.first() == "failed") {
            if(post.size() > 1) {
                std::cout << "Error: Starting failed. Only " << post.at(1).toStdString() << " can do that." << std::endl;
                emit remoteStarted (NotContr);
            }
        }
        else if(post.first() == "success") {
        }
        else if(post.first() == "state") {
            if(post.size() > 1 && post.at(1) == "running") {
               emit remoteStarted (Already);
            }
        }
        else {
            post.clear();
            std::cerr << "Unknown start reply from host." << std::endl;
        }
    }

    else if(post.at(1) == "stop") {
        if(post.size() < 3) return;

        post.removeFirst();
        post.removeFirst();

        if(post.first() == "failed") {
            if(post.size() > 1) {
                std::cout << "Error: Stopping failed. Only " << post.at(1).toStdString() << " can do that." << std::endl;
                emit remoteStopped (NotContr);
            }
        }
        else if(post.first() == "success") {
        }
        else if(post.first() == "state") {
            if(post.size() > 1 && post.at(1) == "not_running") {
                emit remoteStopped (Already);
            }
        }
        else {
            post.clear();
            std::cerr << "Unknown stop reply from host." << std::endl;
        }
    }
}

void GeckoRemote::processRemoteState(QStringList state)
{
    for(QStringList::iterator i = state.begin (); i != state.end (); ++i)
    {
        QString item = *i;
        if(item == "running") { RemoteState_.running = true; }
        else if(item == "not_running") { RemoteState_.running = false; }
        else if(item == "remoteControlled") {
            RemoteState_.controlled = true;
            ++i;
            if (i != state.end ())
                RemoteState_.controller = QHostAddress (*i);
        }
        else if(item == "not_remoteControlled") { RemoteState_.controlled = false; }
    }
}


GeckoRemote::AddrSet GeckoRemote::getLocalAddresses () const {
    AddrSet addrs = AddrSet::fromList (QNetworkInterface::allAddresses ());

    for (AddrSet::iterator i = addrs.begin (); i != addrs.end ();)
    {
        // Only return ip V4 addresses and omit local host
        if(*i == QHostAddress::LocalHost || i->protocol() == QAbstractSocket::IPv6Protocol)
        {
            i = addrs.erase (i);
        }
        else
        {
           ++i;
        }
    }

    return addrs;
}


bool GeckoRemote::isConnected () const {
    return Connected_;
}

bool GeckoRemote::isControlled () const {
    return !Controller_.isNull ();
}

const RemoteGeckoState & GeckoRemote::getRemoteState () const {
    return RemoteState_;
}
