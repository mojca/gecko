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

#include "remotecontrolpanel.h"
#include "runmanager.h"

#include <QLabel>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QTextEdit>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QGridLayout>

#include <QMessageBox>
#include <QTimer>

#include <iostream>

Q_DECLARE_METATYPE (QHostAddress);

RemoteControlPanel::RemoteControlPanel (QWidget *parent)
    : QWidget (parent)
    , remoteUpdateTimer (new QTimer (this))
    , geckoremote (new GeckoRemote (43256)) // "GECKO" on phone keyboard
{
    setAccessibleName (tr ("Remote Control"));
    remoteUpdateTimer->setInterval (1000);
    connect (remoteUpdateTimer, SIGNAL(timeout()), geckoremote, SLOT(startUpdate()));

    createUI ();
}

RemoteControlPanel::~RemoteControlPanel () {
    remoteUpdateTimer->stop();
}

void RemoteControlPanel::createUI () {
    QWidget* remoteControl = new QGroupBox(tr("Remote Control"));
    remoteControl->setAccessibleName(tr("Remote Control"));
    QGridLayout* layout = new QGridLayout();


    QLabel* runNameLabel = new QLabel(tr("Remote Run name:"));
    remoteRunNameEdit = new QLineEdit();

    QLabel* ipAddressLabel = new QLabel(tr("Remote address:"));
    remoteIpAddressEdit = new QComboBox(this);
    //remoteIpAddressEdit->setPlaceholderText("0.0.0.0"); // QT 4.7
    remoteIpAddressEdit->setInsertPolicy(QComboBox::InsertAlphabetically);
    remoteIpAddressEdit->setEditable(true);
    remoteDiscoverButton = new QPushButton(tr("Discover..."));

    QLabel* stateLabel = new QLabel(tr("Remote state:"));
    remoteStateEdit = new QLineEdit();
    remoteStateEdit->setReadOnly(true);
    remoteUpdateButton = new QPushButton(tr("Update"));

    remoteConnectButton = new QPushButton(tr("Connect"));

    QGroupBox* box1 = new QGroupBox(tr("Remote Timing:"));
        QGridLayout* box1l = new QGridLayout();
        QLabel* runStartTimeLabel = new QLabel(tr("Start:"));
        QLabel* runStopTimeLabel = new QLabel(tr("Stop:"));
        remoteStartTimeEdit = new QDateTimeEdit();
        remoteStopTimeEdit = new QDateTimeEdit();
        remoteStartTimeEdit->setDisplayFormat("dd.MM. yyyy hh:mm:ss");
        remoteStopTimeEdit->setDisplayFormat("dd.MM. yyyy hh:mm:ss");
        remoteStartTimeEdit->setReadOnly(true);
        remoteStopTimeEdit->setReadOnly(true);
        box1l->addWidget(runStartTimeLabel,0,0,1,1);
        box1l->addWidget(runStopTimeLabel,1,0,1,1);
        box1l->addWidget(remoteStartTimeEdit,0,1,1,1);
        box1l->addWidget(remoteStopTimeEdit,1,1,1,1);
        box1l->setColumnStretch(1, 1);
    box1->setLayout(box1l);

    QGroupBox* box2 = new QGroupBox(tr("Remote Events:"));
        QGridLayout* box2l = new QGridLayout();
        QLabel* nofEventsLabel = new QLabel(tr("Events:"));
        QLabel* eventsPerSecondLabel = new QLabel(tr("Ev/s:"));
        remoteNofEventsEdit = new QLineEdit(0);
        remoteNofEventsEdit->setReadOnly(true);
        remoteEventsPerSecondEdit = new QLineEdit(0);
        remoteEventsPerSecondEdit->setReadOnly(true);
        box2l->addWidget(nofEventsLabel,0,0,1,1);
        box2l->addWidget(eventsPerSecondLabel,1,0,1,1);
        box2l->addWidget(remoteNofEventsEdit,0,1,1,1);
        box2l->addWidget(remoteEventsPerSecondEdit,1,1,1,1);
    box2->setLayout(box2l);

    remoteRunStartButton = new QPushButton(tr("Start Remote Run"));
    remoteRunStartButton->setMinimumHeight(60);
    connect(remoteRunStartButton,SIGNAL(clicked()), SLOT(remoteRunStartClicked()));
    connect(remoteConnectButton, SIGNAL(clicked()), SLOT(remoteConnectClicked()));
    remoteRunNameButton = new QPushButton(tr("Change"));
    connect(remoteRunNameButton,SIGNAL(clicked()),this,SLOT(remoteRunNameButtonClicked()));
    connect(remoteRunNameEdit, SIGNAL(textChanged(QString)), RunManager::ptr (), SLOT(setRemoteRunName(QString)));
    connect(remoteDiscoverButton, SIGNAL(clicked()), geckoremote, SLOT(startDiscover()));
    connect(remoteUpdateButton, SIGNAL(clicked()), geckoremote,  SLOT(startUpdate()));
    connect(remoteIpAddressEdit,SIGNAL(currentIndexChanged(int)),SLOT(remoteIpAddressChanged(int)));
    connect(remoteIpAddressEdit->lineEdit(),SIGNAL(editingFinished()),SLOT(remoteIpAddressTextChanged()));

    QGroupBox* box3 = new QGroupBox(tr("Remote Notes:"));
        QGridLayout* box3l = new QGridLayout();
        remoteRunInfoEdit = new QTextEdit();
        box3l->addWidget(remoteRunInfoEdit,0,0,1,1);
    box3->setLayout(box3l);

    QGroupBox* box4 = new QGroupBox(tr("Remote Load:"));
        QGridLayout* box4l = new QGridLayout();
        QLabel* cpuLabel = new QLabel(tr("CPU:"));
        QLabel* netLabel = new QLabel(tr("Net:"));
        remoteCpuEdit = new QLineEdit();
        remoteNetEdit = new QLineEdit();
        remoteCpuEdit->setReadOnly(true);
        remoteNetEdit->setReadOnly(true);
        box4l->addWidget(cpuLabel,0,0,1,1);
        box4l->addWidget(remoteCpuEdit,0,1,1,1);
        box4l->addWidget(netLabel,1,0,1,1);
        box4l->addWidget(remoteNetEdit,1,1,1,1);
    box4->setLayout(box4l);

    int row = 0;
    layout->addWidget(ipAddressLabel,       row,0,1,1);
    layout->addWidget(remoteIpAddressEdit,  row,1,1,2);
    layout->addWidget(remoteDiscoverButton, row,3,1,1);
    row++;
    layout->addWidget(remoteConnectButton,  row,3,1,1);
    row++;
    layout->addWidget(stateLabel,           row,0,1,1);
    layout->addWidget(remoteStateEdit,      row,1,1,2);
    layout->addWidget(remoteUpdateButton,   row,3,1,1);
    row++;
    layout->addWidget(runNameLabel,         row,0,1,1);
    layout->addWidget(remoteRunNameEdit,    row,1,1,2);
    layout->addWidget(remoteRunNameButton,  row,3,1,1);
    row++;
    layout->addWidget(box1,                 row,0,1,2);
    layout->addWidget(box2,                 row,2,1,2);
    row++;
    layout->addWidget(box3,                 row,0,2,3);
    layout->addWidget(remoteRunStartButton, row,3,1,1);
    row++;
    layout->addWidget(box4,                 row,3,1,1);

    layout->setColumnStretch (0, 0);
    layout->setColumnStretch (1, 1);
    layout->setColumnStretch (2, 1);
    layout->setColumnStretch (3, 0);

    remoteRunStartButton->setEnabled(false);
    remoteRunNameButton->setEnabled(false);
    remoteRunInfoEdit->setEnabled(false);
    remoteRunNameEdit->setEnabled(false);

    remoteControl->setLayout(layout);
    (new QGridLayout (this))->addWidget (remoteControl);
}

void RemoteControlPanel::remoteRunStartClicked()
{
    if(geckoremote->getRemoteState ().running == false)
    {
        geckoremote->startRemoteRun ();
    }
    else
    {
        geckoremote->stopRemoteRun ();
    }
}

void RemoteControlPanel::remoteConnectClicked()
{
    if(geckoremote->isConnected () == false)
    {
        geckoremote->connectRemote ();
    }
    else
    {
        geckoremote->disconnectRemote ();
    }
}

void RemoteControlPanel::setCurrentRemoteAddress(QHostAddress newAddress)
{
    geckoremote->setRemote (newAddress);
    std::cout << "Changed current remote address to " << newAddress.toString().toStdString() << std::endl;
}

void RemoteControlPanel::remoteIpAddressChanged(int idx)
{
    if(idx != -1)
    {
        QVariant var = remoteIpAddressEdit->itemData(idx);
        setCurrentRemoteAddress(var.value<QHostAddress>());
    }
}

void RemoteControlPanel::remoteIpAddressTextChanged()
{
    QString newIp = remoteIpAddressEdit->lineEdit()->text();
    QHostAddress newAddr (newIp);
    if(!newAddr.isNull () && remoteIpAddressEdit->findData (QVariant::fromValue (newAddr)) == -1)
    {
        remoteIpAddressEdit->addItem(newAddr.toString(),QVariant::fromValue (newAddr));
    }
}

void RemoteControlPanel::discoveredRemote (QHostAddress remote) {
    if (remoteIpAddressEdit->findData (QVariant::fromValue (remote)) == -1) {
        remoteIpAddressEdit->addItem(remote.toString (), QVariant::fromValue (remote));
    }
}

void RemoteControlPanel::remoteUpdateComplete () {
    const RemoteGeckoState &gs = geckoremote->getRemoteState ();
    remoteStateEdit->setText (
            (QStringList ()
             << (gs.running ? tr ("running") : tr ("not running"))
             << (gs.controlled ? tr ("remote controlled") : tr ("not remote controlled"))).join (tr(", ")));

    remoteRunNameEdit->setText (gs.runname);
    remoteStartTimeEdit->setDateTime (gs.starttime);
    remoteStopTimeEdit->setDateTime (gs.stoptime);
    remoteNofEventsEdit->setText (QString::number (gs.nofevents));
    remoteEventsPerSecondEdit->setText (QString::number (gs.eventrate, 'f', 2));
    remoteRunInfoEdit->setText (gs.runinfo);
    remoteCpuEdit->setText (tr("%1 %%").arg (gs.cpuload));
}

void RemoteControlPanel::remoteConnected (QHostAddress controller) {
    if (!controller.isNull ()) {
        QMessageBox::warning (this, tr ("Gecko"),
                              tr ("Connect failed! Remote already controlled by %1.").arg (controller.toString ()), QMessageBox::Ok);
        return;
    } else {
        geckoremote->startUpdate ();
        remoteUpdateTimer->start ();
        remoteConnectButton->setText(tr("Disconnect"));
        remoteRunStartButton->setEnabled(true);
        remoteRunNameButton->setEnabled(true);
        remoteRunInfoEdit->setEnabled(true);
        remoteRunNameEdit->setEnabled(true);
    }
}

void RemoteControlPanel::remoteDisconnected (QHostAddress controller) {
    if (!controller.isNull ()) {
        QMessageBox::warning (this, tr ("Gecko"),
                              tr ("Disconnect failed! Remote already controlled by %1.").arg (controller.toString ()), QMessageBox::Ok);
        return;
    } else {
        geckoremote->startUpdate ();
        remoteUpdateTimer->stop ();
        remoteConnectButton->setText (tr ("Connect"));
        remoteRunStartButton->setEnabled (false);
        remoteRunNameButton->setEnabled (false);
        remoteRunInfoEdit->setEnabled (false);
        remoteRunNameEdit->setEnabled (false);
    }
}

void RemoteControlPanel::remoteStarted (GeckoRemote::StartStopResult res) {
    switch (res) {
    case GeckoRemote::Ok:
        break;
    case GeckoRemote::NotContr:
        QMessageBox::warning (this, tr ("Gecko"),
                              tr ("Start failed! Remote is controlled by %1.")
                              .arg (geckoremote->getRemoteState ().controller.toString ()),
                              QMessageBox::Ok);
        break;
    case GeckoRemote::Already:
        QMessageBox::warning (this, tr ("Gecko"),
                              tr ("Start failed! Remote is already running.")
                              .arg (geckoremote->getRemoteState ().controller.toString ()),
                              QMessageBox::Ok);
        break;
    }
}

void RemoteControlPanel::remoteStopped (GeckoRemote::StartStopResult res) {
    switch (res) {
    case GeckoRemote::Ok:
        break;
    case GeckoRemote::NotContr:
        QMessageBox::warning (this, tr ("Gecko"),
                              tr ("Stop failed! Remote is controlled by %1.")
                              .arg (geckoremote->getRemoteState ().controller.toString ()),
                              QMessageBox::Ok);
        break;
    case GeckoRemote::Already:
        QMessageBox::warning (this, tr ("Gecko"),
                              tr ("Stop failed! Remote is not running.")
                              .arg (geckoremote->getRemoteState ().controller.toString ()),
                              QMessageBox::Ok);
        break;
    }
}

