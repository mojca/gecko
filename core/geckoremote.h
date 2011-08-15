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

#ifndef GECKOREMOTE_H
#define GECKOREMOTE_H

#include <stdint.h>

#include <QObject>
#include <QHostAddress>
#include <QSet>
#include <QString>
#include <QDateTime>
#include <QMetaType>

class QTcpSocket;
class QTcpServer;
class QUdpSocket;

struct RemoteGeckoState {
    bool running;
    bool controlled;
    QHostAddress controller;
    QString runname;
    QDateTime starttime;
    QDateTime stoptime;
    long nofevents;
    float eventrate;
    QString runinfo;
    int cpuload;

    RemoteGeckoState ()
    : running (false)
    , controlled (false)
    , controller (QHostAddress::Null)
    , nofevents (-1)
    , eventrate (-1)
    , cpuload (-1)
    {}
};

class GeckoRemote : public QObject {
    Q_OBJECT
public:
    enum StartStopResult {Ok, NotContr, Already};
    typedef QSet<QHostAddress> AddrSet;

    GeckoRemote (uint16_t localport);
    ~GeckoRemote ();

    AddrSet getDiscoveredInstances ();
    QHostAddress getRemote () const;
    uint16_t getLocalPort () const;

    const RemoteGeckoState& getRemoteState () const;

    bool isControlled () const;
    bool isConnected () const;

signals:
    void remoteControlled (bool, QHostAddress);
    void discovered (QHostAddress);
    void connected (QHostAddress);
    void disconnected (QHostAddress);
    void updateComplete ();
    void remoteStarted (StartStopResult);
    void remoteStopped (StartStopResult);

public slots:
    void setRemote (QHostAddress);
    void startDiscover ();
    void connectRemote ();
    void disconnectRemote ();
    void startUpdate ();
    void startRemoteRun ();
    void stopRemoteRun ();

private:
    AddrSet getLocalAddresses () const;
    void processDatagram (QByteArray datagram, QHostAddress sender, uint16_t senderPort);
    void processQuery (QStringList query, QHostAddress sender);
    void processPost (QStringList post, QHostAddress sender);
    void processRemoteState (QStringList post);

private slots:
    void readUdpDatagram ();

    void tcpDisconnected ();
    void tcpServerNewConnection ();

private:
    uint16_t LocalPort_;

    QTcpSocket *TcpSock_;
    QTcpServer *TcpServ_;
    QTcpSocket *TcpControllerSock_;
    QUdpSocket *UdpSock_;

    AddrSet LocalAddrs_;
    AddrSet RemoteAddrs_;

    QHostAddress Remote_;
    QHostAddress Controller_;

    bool Connected_;

    RemoteGeckoState RemoteState_;
};

Q_DECLARE_METATYPE (GeckoRemote::StartStopResult);

#endif // GECKOREMOTE_H
