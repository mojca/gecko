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
