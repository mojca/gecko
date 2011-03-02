#ifndef SCOPEMAINWINDOW_H
#define SCOPEMAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QObject>
#include <QSettings>
#include <QCoreApplication>
#include <QApplication>

#include <QLabel>

#include <QGridLayout>

#include <QFileDialog>
#include <QDateTimeEdit>
#include <QDateTime>

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>

#include <QStatusBar>

#include <QDockWidget>
#include <QStackedWidget>

#include <QTreeWidget>
#include <QTreeView>
#include <QStandardItemModel>

#include <QTimer>
#include <QTextStream>

#include <QUdpSocket>
#include <QTcpSocket>
#include <QTcpServer>

#include <typeinfo>

// Include external headers
#include "runthread.h"
#include "pluginthread.h"

#include "pluginmanager.h"
#include "baseplugin.h"

#include "runmanager.h"

#include "modulemanager.h"
#include "abstractinterface.h"

class QStandardItem;
class QTreeWidget;
class QCheckBox;
class QHostAddress;
class QTextEdit;
class QComboBox;

class SystemInfo;

Q_DECLARE_METATYPE(QWidget*)
Q_DECLARE_METATYPE(ScopeChannel*)
Q_DECLARE_METATYPE(QHostAddress)
Q_DECLARE_METATYPE(QAbstractSocket::SocketError)

// Class definition

class ScopeMainWindow : public QMainWindow
{
        Q_OBJECT

protected:
    void createUI();
    void createTreeView();
    void createActions();
    void createMenu();
    void createConnections();
    void createStatusBar();

    void createRunSetupPage();
    void createRunControlPage();
    void createRemoteControlPage();
    void createUdpSocket();
    void createTcpSocket();
    void loadChannelList();

    QStandardItem *addTabToTree(QWidget* newTab);
    void addRunPageToTree(QWidget* newWidget);

    void closeEvent(QCloseEvent *event);
    void setRunName(QString);

    void writeRunStartFile();
    void writeRunStopFile();
    void writeDotFile (QTextStream &out);

    void saveConfig (QSettings *);
    void loadConfig (QSettings *);

    void setConfigEnabled (bool enabled);
    void setLocalAddress();

    QList<QHostAddress> findOutIpAddresses();
    void processDatagram(QByteArray, QHostAddress sender, quint16 serverPort);
    void processQuery(QStringList query, QHostAddress sender);
    void processPost(QStringList post, QHostAddress sender);
    void processRemoteState(QStringList state);

public:
    explicit ScopeMainWindow(QWidget *parent = 0);
    ~ScopeMainWindow();
    ModuleManager *mmgr;
    PluginManager *pmgr;
    RunManager    *rmgr;

    void saveSettingsToFile (const QString file);

signals:

public slots:
    void treeViewClicked(const QModelIndex & idx, const QModelIndex & prev);
    void triggerListChanged(QTreeWidgetItem*,int);
    void channelListChanged(QTreeWidgetItem*,int);
    void setStatusText(QString);
    void runNameButtonClicked();
    void applySettings();
    void saveSettings();
    bool saveSettingsQuery();

    void startAcquisition();
    void stopAcquisition();
    void runStarted ();
    void runStopping ();
    void runStopped ();
    void updateRunPage(float evspersec, unsigned evs);

    void addModuleToTree(AbstractModule* newModule);
    void addInterfaceToTree(AbstractInterface* newModule);
    void addPluginToTree(AbstractPlugin* newPlugin);
    void removeModuleFromTree(AbstractModule* newModule);
    void removeInterfaceFromTree(AbstractInterface* newModule);
    void removePluginFromTree(AbstractPlugin* newPlugin);
    void moduleNameChanged(AbstractModule *m, QString oldname);
    void interfaceNameChanged(AbstractInterface *m, QString oldname);
    void pluginNameChanged(AbstractPlugin *p, QString oldname);

    void open();
    bool save();
    bool saveAs();

    void exportDot ();

    void createModule ();
    void editModule ();
    void removeModule ();

    void createInterface ();
    void editInterface ();
    void removeInterface ();

    void createPlugin ();
    void editPlugin ();
    void removePlugin ();

    void createComponent ();
    void editComponent ();
    void removeComponent ();

    void makeMainInterface ();

    void jumpToPlugin (AbstractPlugin *);

    void readUdpDatagram();
    void remoteDiscoverClicked();
    void remoteConnectClicked();
    void remoteRunStartClicked();
    void remoteUpdateClicked();
    void remoteIpAddressChanged(int);
    void remoteIpAddressTextChanged();
    void setCurrentRemoteAddress(QHostAddress);

    void oneSecondTimerTimeout();

    // TCP communication
    void connectTcp();
    void disconnectTcp();
    void tcpConnected();
    void tcpDisconnected();
    void tcpError(QAbstractSocket::SocketError);
    void readTcpSocket();
    void tcpServerConnected();
    void tcpServerDisconnected();
    void tcpServerError(QAbstractSocket::SocketError);
    void readTcpServerSocket();
    void tcpServerNewConnection();

private:
    const QString defaultIni;
    QString fileName;

    QMenuBar *menubar;
    QMenu *fileMenu;
    QMenu *modulesMenu;
    QMenu *pluginsMenu;
    QMenu *interfacesMenu;

    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *exportDotAct;

    QAction *createModAct;
    QAction *editModAct;
    QAction *removeModAct;

    QAction *createIfAct;
    QAction *editIfAct;
    QAction *removeIfAct;

    QAction *createPlugAct;
    QAction *editPlugAct;
    QAction *removePlugAct;

    QAction *createAct;
    QAction *editAct;
    QAction *removeAct;

    QAction *makeMainIfaceAct;

    QStatusBar *statusBar;
    QLabel *statusLabel;
    QLabel *statusCpuLabel;
    QLabel *statusNetLabel;

    QDockWidget *treeDock;
    QTreeWidget *treeWidget;
    QTreeView *treeView;
    QStandardItemModel *treeModel;

    // Run setup
    QTreeWidget* triggerList;
    QTreeWidget* channelList;

    // Run control
    QPushButton* runStartButton;
    QPushButton* runNameButton;
    QLineEdit* runNameEdit;
    QTextEdit* runInfoEdit;
    QDateTimeEdit* startTimeEdit;
    QDateTimeEdit* stopTimeEdit;
    QLineEdit* nofEventsEdit;
    QLineEdit* eventsPerSecondEdit;
    QCheckBox *singleEventModeBox;

    // Remote control
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

    // Timers
    QTimer* remoteUpdateTimer;
    QTimer* oneSecondTimer;

    // Network
    QUdpSocket* udpSocket;
    QTcpSocket* tcpSocket;
    QTcpSocket* tcpServerSocket;
    QTcpServer* tcpServer;
    int localPort;
    QHostAddress localAddress;
    QHostAddress currentRemoteAddress;
    QHostAddress controllerAddress;
    QList<QHostAddress> remoteAddresses;

    // System
    SystemInfo* sysinfo;

    // Layout
    QStackedWidget *mainArea;
    QStandardItem  *runItem;
    QStandardItem  *pluginItem;
    QStandardItem  *ifaceItem;
    QStandardItem  *moduleItem;

    QSettings* settings;

    bool configEditAllowed;    // Configuration can be changed
    bool connectedToRemote;    // This client is connected to a remote peer via udp
    bool tcpIsConnected;       // A tcp connection to the remote peer has been established
    bool tcpServerIsConnected; // A tcp connection to the local server has been established
    bool remoteIsRunning;      // The remote peer is running
    bool remoteIsControlled;   // The remote peer is being controlled by a peer
};



class SystemInfo : public QObject
{
    Q_OBJECT

public:
    SystemInfo() : cpuLoad(0),netLoad(0),lastCpuLoad(0),lastNetLoad(0),nofCores(QThread::idealThreadCount()) {
        if(nofCores == -1) nofCores = 1;
        //std::cout << "Detected " << nofCores << " CPUs in this system" << std::endl;
        oneSecondTimer = new QTimer();
        oneSecondTimer->start(1000);
        connect(oneSecondTimer, SIGNAL(timeout()), this, SLOT(oneSecondTimerTimeout()));
    }
    ~SystemInfo() { oneSecondTimer->stop(); }

    int getNofCores() { return nofCores; }
    double getCpuLoad() { return cpuLoad-lastCpuLoad; }

    void parseProcStat() {
        QFile file("/proc/stat");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        QTextStream in(&file);
        QString line = in.readLine();
        bool stop = false;
        while (!line.isNull() && stop == false) {
            if(line.startsWith("cpu  ")) {
                QStringList parts = line.split(QChar(' '));
                // Order is: 1 user, 2 system, 3 nice, 4 idle (1/100 seconds)
                double user   = parts.at(2).toDouble();
                double system = parts.at(3).toDouble();
                double nice   = parts.at(4).toDouble();
                //double idle   = parts.at(5).toDouble();
                lastCpuLoad = cpuLoad;
                cpuLoad = (user+system+nice)/100./nofCores;
                //std::cout << "user: " << std::dec << user << " sys: " << system << " nice: " << nice << std::endl;
                //std::cout << "Current CPU load: " << std::dec << cpuLoad-lastCpuLoad << std::endl;
                stop = true;
            }

            line = in.readLine();
        }
    }

    void parseProcNetDev() {

    }

public slots:
    void oneSecondTimerTimeout() {
        parseProcStat();
        parseProcNetDev();
    }

private:
    QTimer* oneSecondTimer;

    double cpuLoad;
    double netLoad;
    double lastCpuLoad;
    double lastNetLoad;
    int nofCores;
};

#endif // SCOPEMAINWINDOW_H
