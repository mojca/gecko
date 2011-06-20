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

#ifndef REMOTECONTROLPANEL_H
#define REMOTECONTROLPANEL_H

#include <QWidget>
#include "geckoremote.h"

class QPushButton;
class QLineEdit;
class QTextEdit;
class QComboBox;
class QCheckBox;
class QDateTimeEdit;

class RemoteControlPanel : public QWidget {
    Q_OBJECT
public:
    RemoteControlPanel (QWidget *parent);
    ~RemoteControlPanel ();

private:
    void createUI ();

private slots:
    void discoveredRemote (QHostAddress remote);
    void remoteUpdateComplete ();
    void remoteConnected (QHostAddress controller);
    void remoteDisconnected (QHostAddress controller);
    void remoteStarted (GeckoRemote::StartStopResult res);
    void remoteStopped (GeckoRemote::StartStopResult res);

    void remoteConnectClicked();
    void remoteRunStartClicked();
    void remoteIpAddressChanged(int);
    void remoteIpAddressTextChanged();
    void setCurrentRemoteAddress(QHostAddress);

private:
    QPushButton* remoteRunStartButton;
    QPushButton* remoteRunNameButton;
    QPushButton* remoteDiscoverButton;
    QPushButton* remoteUpdateButton;
    QPushButton* remoteConnectButton;
    QLineEdit* remoteRunNameEdit;
    QTextEdit* remoteRunInfoEdit;
    QDateTimeEdit* remoteStartTimeEdit;
    QDateTimeEdit* remoteStopTimeEdit;
    QLineEdit* remoteNofEventsEdit;
    QLineEdit* remoteEventsPerSecondEdit;
    QCheckBox* remoteSingleEventModeBox;
    QComboBox* remoteIpAddressEdit;
    QLineEdit* remoteStateEdit;
    QLineEdit* remoteCpuEdit;
    QLineEdit* remoteNetEdit;

    QTimer* remoteUpdateTimer;
    GeckoRemote *geckoremote;
};

#endif // REMOTECONTROLPANEL_H
