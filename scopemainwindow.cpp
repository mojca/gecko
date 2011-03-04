#include "scopemainwindow.h"
#include "addeditdlgs.h"
#include "scopechannel.h"
#include "baseui.h"
#include "systeminfo.h"
#include "geckoremote.h"

#include <QThreadPool>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QCloseEvent>

#include <stdexcept>

ScopeMainWindow::ScopeMainWindow(QWidget *parent) :
    QMainWindow(parent),defaultIni("gecko.ini")
{
    oneSecondTimer = new QTimer();
    oneSecondTimer->start(1000);

    geckoremote = new GeckoRemote (43256);

    fileName = defaultIni;

    mmgr = ModuleManager::ptr();
    pmgr = PluginManager::ptr();
    rmgr = RunManager::ptr();
    rmgr->setMainWindow (this);

    settings = new QSettings(fileName,QSettings::IniFormat);

    createActions();
    createUI();
    createConnections();

    applySettings();

    setGeometry(QRect(QPoint(0,0),QSize(640,480)));
    setStatusText(tr("Idle"));

    configEditAllowed = true;
}

ScopeMainWindow::~ScopeMainWindow()
{
    stopAcquisition ();
    mmgr->clear();
    pmgr->clear();

    remoteUpdateTimer->stop();

    delete settings;
}

void ScopeMainWindow::createUI()
{
    mainArea = new QStackedWidget(this);

    createMenu();
    createStatusBar();
    createTreeView();

    connect (ModuleManager::ptr (), SIGNAL (moduleAdded(AbstractModule*)), SLOT (addModuleToTree(AbstractModule*)));
    connect (ModuleManager::ptr (), SIGNAL (moduleRemoved(AbstractModule*)), SLOT (removeModuleFromTree(AbstractModule*)));
    connect (ModuleManager::ptr (), SIGNAL (moduleNameChanged(AbstractModule*,QString)),
             SLOT (moduleNameChanged(AbstractModule*,QString)));

    connect (InterfaceManager::ptr (), SIGNAL (interfaceAdded(AbstractInterface*)),
             SLOT (addInterfaceToTree(AbstractInterface*)));
    connect (InterfaceManager::ptr (), SIGNAL (interfaceRemoved(AbstractInterface*)),
             SLOT (removeInterfaceFromTree(AbstractInterface*)));
    connect (InterfaceManager::ptr (), SIGNAL (interfaceNameChanged(AbstractInterface*,QString)),
             SLOT (interfaceNameChanged(AbstractInterface*,QString)));

    connect (PluginManager::ptr (), SIGNAL (pluginAdded(AbstractPlugin*)), SLOT (addPluginToTree(AbstractPlugin*)));
    connect (PluginManager::ptr (), SIGNAL (pluginRemoved(AbstractPlugin*)), SLOT (removePluginFromTree(AbstractPlugin*)));
    connect (PluginManager::ptr (), SIGNAL (pluginNameChanged(AbstractPlugin*,QString)),
             SLOT (pluginNameChanged(AbstractPlugin*,QString)));

    connect (RunManager::ptr (), SIGNAL(runStarted()), SLOT(runStarted()));
    connect (RunManager::ptr (), SIGNAL(runStopping()), SLOT(runStopping()));
    connect (RunManager::ptr (), SIGNAL(runStopped()), SLOT(runStopped()));
    connect (RunManager::ptr (), SIGNAL(runUpdate(float,uint)), SLOT(updateRunPage(float,uint)));

    createRunSetupPage();
    createRunControlPage();
    createRemoteControlPage();

    treeView->expandAll();

    setCentralWidget(mainArea);
}

void ScopeMainWindow::createTreeView()
{
    treeModel = new QStandardItemModel();
    treeModel->setHorizontalHeaderLabels(QStringList(""));

    treeView = new QTreeView(this);
    treeView->setModel(treeModel);

    treeDock = new QDockWidget(tr("Settings"),this);
    treeDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    treeDock->setMinimumWidth(200);
    treeDock->setMaximumWidth(200);
    treeDock->setGeometry(0,0,200,200);
    treeDock->setWidget(treeView);
    treeDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea,treeDock);

    // Create tabs
    runItem = addTabToTree(new QLabel(tr("Run")));
    ifaceItem = addTabToTree(new QLabel(tr("Interfaces")));
    moduleItem = addTabToTree(new QLabel(tr("Modules")));
    pluginItem = addTabToTree(new QLabel(tr("Plugins")));

    treeView->addAction (createAct);
    treeView->addAction (editAct);
    treeView->addAction (removeAct);
    treeView->addAction (makeMainIfaceAct);
    treeView->setContextMenuPolicy (Qt::ActionsContextMenu);
}

void ScopeMainWindow::createActions()
{
    openAct = new QAction(tr("&Open..."),this);
    openAct->setShortcuts(QKeySequence::Open);
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(tr("&Save"),this);
    saveAct->setShortcuts(QKeySequence::Save);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &as..."),this);
#if QT_VERSION >= 0x040600
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
#endif
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    exitAct = new QAction(tr("&Exit"),this);
#if QT_VERSION >= 0x040600
    exitAct->setShortcuts(QKeySequence::Quit);
#endif
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    createModAct = new QAction(tr("Add Module..."), this);
    createModAct->setStatusTip (tr("Add a module to the configuration"));
    connect (createModAct, SIGNAL(triggered()), SLOT(createModule()));

    editModAct = new QAction(tr("Edit Module..."), this);
    editModAct->setStatusTip(tr("Edit the selected module"));
    editModAct->setEnabled (false);
    connect (editModAct, SIGNAL(triggered()), SLOT(editModule()));

    removeModAct = new QAction(tr("Remove Module"), this);
    removeModAct->setStatusTip(tr("Remove the selected module"));
    removeModAct->setEnabled (false);
    connect (removeModAct, SIGNAL(triggered()), SLOT(removeModule()));

    createAct = new QAction(tr("Add..."), this);
    createAct->setStatusTip (tr("Add a component to the configuration"));
    connect (createAct, SIGNAL(triggered()), SLOT(createComponent()));

    editAct = new QAction(tr("Edit..."), this);
    editAct->setStatusTip(tr("Edit the selected component"));
    editAct->setEnabled (false);
    connect (editAct, SIGNAL(triggered()), SLOT(editComponent()));

    removeAct = new QAction(tr("Remove"), this);
    removeAct->setStatusTip(tr("Remove the selected component"));
    removeAct->setEnabled (false);
    connect (removeAct, SIGNAL(triggered()), SLOT(removeComponent()));

    createIfAct = new QAction(tr("Add Interface..."), this);
    createIfAct->setStatusTip (tr("Add a interface to the configuration"));
    connect (createIfAct, SIGNAL(triggered()), SLOT(createInterface()));

    editIfAct = new QAction(tr("Edit Interface..."), this);
    editIfAct->setStatusTip(tr("Edit the selected interface"));
    editIfAct->setEnabled (false);
    connect (editIfAct, SIGNAL(triggered()), SLOT(editInterface()));

    removeIfAct = new QAction(tr("Remove Interface"), this);
    removeIfAct->setStatusTip(tr("Remove the selected interface"));
    removeIfAct->setEnabled (false);
    connect (removeIfAct, SIGNAL(triggered()), SLOT(removeInterface()));

    createPlugAct = new QAction (tr("Add Plugin..."), this);
    createPlugAct->setStatusTip (tr("Add a plugin to the configuration"));
    connect (createPlugAct, SIGNAL(triggered()), SLOT(createPlugin()));

    editPlugAct = new QAction (tr("Edit Plugin..."), this);
    editPlugAct->setStatusTip (tr("Edit the selected plugin"));
    editPlugAct->setEnabled (false);
    connect (editPlugAct, SIGNAL(triggered()), SLOT(editPlugin()));

    removePlugAct = new QAction (tr("Remove Plugin"), this);
    removePlugAct->setStatusTip (tr("Remove the selected plugin"));
    removePlugAct->setEnabled (false);
    connect (removePlugAct, SIGNAL(triggered()), SLOT(removePlugin()));

    exportDotAct = new QAction (tr("Export to &GraphViz..."), this);
    exportDotAct->setStatusTip (tr("Export configuration to a file renderable with GraphViz"));
    connect (exportDotAct, SIGNAL(triggered()), SLOT(exportDot()));

    makeMainIfaceAct = new QAction (tr("Make main Interface"), this);
    makeMainIfaceAct->setStatusTip(tr("Makes the selected interface the main interface"));
    makeMainIfaceAct->setEnabled (false);
    connect (makeMainIfaceAct, SIGNAL(triggered()), SLOT(makeMainInterface()));
}

void ScopeMainWindow::createMenu()
{
    fileMenu = new QMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exportDotAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);
    modulesMenu = new QMenu(tr("&Modules"));
    modulesMenu->addAction(createModAct);
    modulesMenu->addAction(editModAct);
    modulesMenu->addAction(removeModAct);
    pluginsMenu = new QMenu(tr("&Plugins"));
    pluginsMenu->addAction(createPlugAct);
    pluginsMenu->addAction(editPlugAct);
    pluginsMenu->addAction(removePlugAct);
    interfacesMenu = new QMenu(tr("&Interfaces"));
    interfacesMenu->addAction(createIfAct);
    interfacesMenu->addAction(editIfAct);
    interfacesMenu->addAction(removeIfAct);
    interfacesMenu->addSeparator();
    interfacesMenu->addAction(makeMainIfaceAct);

    menubar = this->menuBar();
    menubar->addMenu(fileMenu);
    menubar->addMenu(modulesMenu);
    menubar->addMenu(pluginsMenu);
    menubar->addMenu(interfacesMenu);
}

void ScopeMainWindow::createConnections()
{
    connect(treeView->selectionModel (),SIGNAL(currentChanged (QModelIndex,QModelIndex)), SLOT(treeViewClicked(QModelIndex,QModelIndex)));
    connect(oneSecondTimer, SIGNAL(timeout()), this, SLOT(oneSecondTimerTimeout()));
}

void ScopeMainWindow::createStatusBar()
{
    statusBar = new QStatusBar(this);
    statusLabel = new QLabel(tr("Initializing..."));
    statusCpuLabel = new QLabel(tr("CPU: 0 %"));
    statusCpuLabel->setAlignment(Qt::AlignRight);
    statusBar->addWidget(statusLabel,1);
    statusBar->addWidget(statusCpuLabel,1);
    setStatusBar(statusBar);
}

QStandardItem *ScopeMainWindow::addTabToTree(QWidget* newTab)
{
    mainArea->addWidget(newTab);
    QStandardItem* newItem = new QStandardItem(dynamic_cast<QLabel*>(newTab)->text());
    newItem->setEditable(false);
    newItem->setData(QVariant::fromValue(newTab));
    treeModel->appendRow(newItem);
    return newItem;
}

void ScopeMainWindow::addModuleToTree(AbstractModule* newModule)
{
    QWidget* newWidget = newModule->getUI();
    QStandardItem* item = new QStandardItem(newModule->getName());
    item->setEditable(false);
    item->setData(QVariant::fromValue(newWidget));
    mainArea->addWidget(newWidget);
    moduleItem->appendRow(item);
    loadChannelList ();
}

void ScopeMainWindow::removeModuleFromTree(AbstractModule* newModule)
{
    QWidget* newWidget = newModule->getUI();
    mainArea->removeWidget (newWidget);
    QList<QStandardItem*> modList = treeModel->findItems(newModule->getName (), Qt::MatchExactly | Qt::MatchRecursive);
    if (modList.empty())
        return;
    foreach (QStandardItem *it, modList) {
        if (it->parent () != moduleItem) continue;
        moduleItem->removeRow (it->row ());
        break;
    }
    loadChannelList ();
}

void ScopeMainWindow::moduleNameChanged(AbstractModule *m, QString oldname) {
    QList<QStandardItem*> modList = treeModel->findItems(oldname, Qt::MatchExactly | Qt::MatchRecursive);
    if (modList.empty())
        return;
    foreach (QStandardItem *it, modList) {
        if (it->parent () != moduleItem) continue;
        it->setText (m->getName());
        break;
    }
    loadChannelList ();
}

void ScopeMainWindow::addInterfaceToTree(AbstractInterface* newIf)
{
    QWidget* newWidget = newIf->getUI();
    QStandardItem* item = new QStandardItem(newIf->getName());
    item->setEditable(false);
    item->setData(QVariant::fromValue(newWidget));
    mainArea->addWidget(newWidget);
    ifaceItem->appendRow(item);
}

void ScopeMainWindow::removeInterfaceFromTree(AbstractInterface* newIf)
{
    QWidget* newWidget = newIf->getUI();
    mainArea->removeWidget (newWidget);
    QList<QStandardItem*> ifList = treeModel->findItems(newIf->getName (), Qt::MatchExactly | Qt::MatchRecursive);
    if (ifList.empty())
        return;
    foreach (QStandardItem *it, ifList) {
        if (it->parent () != ifaceItem) continue;
        ifaceItem->removeRow (it->row ());
        break;
    }
}

void ScopeMainWindow::interfaceNameChanged(AbstractInterface *m, QString oldname) {
    QList<QStandardItem*> modList = treeModel->findItems(oldname, Qt::MatchExactly | Qt::MatchRecursive);
    if (modList.empty())
        return;
    foreach (QStandardItem *it, modList) {
        if (it->parent () != moduleItem) continue;
        it->setText (m->getName());
        break;
    }
}

void ScopeMainWindow::pluginNameChanged(AbstractPlugin *p, QString oldname) {
    QList<QStandardItem*> plugList = treeModel->findItems(oldname, Qt::MatchExactly | Qt::MatchRecursive);
    if (plugList.empty())
        return;
    foreach (QStandardItem *it, plugList) {
        if (it->parent () != pluginItem) continue;
        it->setText (p->getName());
        break;
    }
}

void ScopeMainWindow::addPluginToTree(AbstractPlugin* newPlugin)
{
    QWidget* newWidget = newPlugin;
    mainArea->addWidget(newWidget);

    connect (newPlugin, SIGNAL(jumpToPluginRequested(AbstractPlugin*)), SLOT(jumpToPlugin(AbstractPlugin*)));

    QStandardItem* item = new QStandardItem(newPlugin->getName());
    item->setEditable(false);
    item->setData(QVariant::fromValue(newWidget));
    pluginItem->appendRow(item);
}

void ScopeMainWindow::removePluginFromTree(AbstractPlugin* newPlugin)
{
    QWidget* newWidget = newPlugin;
    mainArea->removeWidget (newWidget);
    QList<QStandardItem*> plugList = treeModel->findItems(newPlugin->getName (), Qt::MatchExactly | Qt::MatchRecursive);
    if (plugList.empty())
        return;
    foreach (QStandardItem *it, plugList) {
        if (it->parent () != pluginItem) continue;
        pluginItem->removeRow (it->row ());
        break;
    }
}

void ScopeMainWindow::jumpToPlugin(AbstractPlugin *p) {
    for (int i= 0; i < pluginItem->rowCount (); ++i) {
        if (pluginItem->child (i, 0)->data().value<QWidget*> () == static_cast<QWidget*> (p)) {
            treeView->setCurrentIndex (pluginItem->child (i, 0)->index ());
        }
    }

}

void ScopeMainWindow::createModule () {
    AddEditModuleDlg dlg (this, NULL);
    dlg.exec ();
    loadChannelList();
}

void ScopeMainWindow::editModule () {
    if (!treeView->currentIndex ().isValid ())
        return;

    QStandardItem *it = treeModel->itemFromIndex (treeView->currentIndex ());
    if (it->parent () != moduleItem)
        return;
    AbstractModule *m = ModuleManager::ref().get(it->text ());
    AddEditModuleDlg dlg (this, m);
    dlg.exec ();
    loadChannelList();
}

void ScopeMainWindow::removeModule () {
    if (!treeView->currentIndex ().isValid ())
        return;

    QStandardItem *it = treeModel->itemFromIndex (treeView->currentIndex ());
    if (it->parent () != moduleItem)
        return;
    ModuleManager::ref().remove(it->text ());
    loadChannelList();
}

void ScopeMainWindow::createInterface () {
    AddEditInterfaceDlg dlg (this, NULL);
    dlg.exec ();
}

void ScopeMainWindow::editInterface () {
    if (!treeView->currentIndex ().isValid ())
        return;

    QStandardItem *it = treeModel->itemFromIndex (treeView->currentIndex ());
    if (it->parent () != ifaceItem)
        return;
    AbstractInterface *m = InterfaceManager::ref().get (it->text ());
    AddEditInterfaceDlg dlg (this, m);
    dlg.exec ();
}

void ScopeMainWindow::removeInterface () {
    if (!treeView->currentIndex ().isValid ())
        return;

    QStandardItem *it = treeModel->itemFromIndex (treeView->currentIndex ());
    if (it->parent () != ifaceItem)
        return;
    InterfaceManager::ref ().remove (it->text ());
}

void ScopeMainWindow::createPlugin () {
    AddEditPluginDlg dlg (this, NULL);
    dlg.exec ();
}

void ScopeMainWindow::editPlugin () {
    if (!treeView->currentIndex ().isValid ())
        return;

    QStandardItem *it = treeModel->itemFromIndex (treeView->currentIndex ());
    if (it->parent () != pluginItem)
        return;
    AbstractPlugin *p = PluginManager::ref().get(it->text ());
    AddEditPluginDlg dlg (this, p);
    dlg.exec ();
}

void ScopeMainWindow::removePlugin () {
    if (!treeView->currentIndex ().isValid ())
        return;

    QStandardItem *it = treeModel->itemFromIndex (treeView->currentIndex ());
    if (it->parent () != pluginItem)
        return;
    PluginManager::ref().remove(it->text ());
}

void ScopeMainWindow::createComponent () {
    if (!treeView->currentIndex ().isValid ())
        return;

    QStandardItem *it = treeModel->itemFromIndex (treeView->currentIndex ());
    QStandardItem *parent = it->parent ();

    if (it == ifaceItem || parent == ifaceItem)
        createInterface ();
    else if (it == moduleItem || parent == moduleItem)
        createModule ();
    else if (it == pluginItem || parent == pluginItem)
        createPlugin ();
}

void ScopeMainWindow::editComponent () {
    if (!treeView->currentIndex ().isValid ())
        return;

    QStandardItem *it = treeModel->itemFromIndex (treeView->currentIndex ());
    QStandardItem *parent = it->parent ();

    if (it == ifaceItem || parent == ifaceItem)
        editInterface ();
    else if (it == moduleItem || parent == moduleItem)
        editModule ();
    else if (it == pluginItem || parent == pluginItem)
        editPlugin ();
}

void ScopeMainWindow::removeComponent () {
    if (!treeView->currentIndex ().isValid ())
        return;

    QStandardItem *it = treeModel->itemFromIndex (treeView->currentIndex ());
    QStandardItem *parent = it->parent ();

    if (it == ifaceItem || parent == ifaceItem)
        removeInterface ();
    else if (it == moduleItem || parent == moduleItem)
        removeModule ();
    else if (it == pluginItem || parent == pluginItem)
        removePlugin ();
}

void ScopeMainWindow::makeMainInterface () {
    if (!treeView->currentIndex ().isValid ())
        return;

    QStandardItem *it = treeModel->itemFromIndex (treeView->currentIndex ());
    if (it->parent () != ifaceItem)
        return;

    InterfaceManager::ref().setMainInterface (InterfaceManager::ref().get (it->text ()));
}

void ScopeMainWindow::addRunPageToTree(QWidget* newWidget)
{
    mainArea->addWidget(newWidget);
    QStandardItem* item = new QStandardItem(newWidget->accessibleName());
    item->setEditable(false);
    item->setData(QVariant::fromValue(newWidget));
    runItem->appendRow(item);
}

void ScopeMainWindow::createRunSetupPage()
{
    QWidget* runSetup = new QGroupBox(tr("Run Setup"));
    runSetup->setAccessibleName(tr("Run Setup"));

    QGridLayout* layout = new QGridLayout();
    QGridLayout* boxLayout = new QGridLayout();
    QGridLayout* boxLayout2 = new QGridLayout();

    QGroupBox* triggerBox = new QGroupBox(tr("Triggers"));
    QGroupBox* channelBox = new QGroupBox(tr("Channels"));

    triggerList = new QTreeWidget();
    triggerList->setColumnCount(2);
    QStringList headerLabels;
    headerLabels.append("Module");
    headerLabels.append("Channel");
    triggerList->setHeaderLabels(headerLabels);
    headerLabels.clear();

    channelList = new QTreeWidget();
    channelList->setColumnCount(3);
    headerLabels.append("Module");
    headerLabels.append("Channel");
    headerLabels.append("Type");
    channelList->setHeaderLabels(headerLabels);
    headerLabels.clear();

    loadChannelList();

    connect(triggerList,SIGNAL(itemChanged(QTreeWidgetItem*,int)),SLOT(triggerListChanged(QTreeWidgetItem*,int)));
    connect(channelList,SIGNAL(itemChanged(QTreeWidgetItem*,int)),SLOT(channelListChanged(QTreeWidgetItem*,int)));

    boxLayout->addWidget(triggerList,0,0,1,1);
    boxLayout2->addWidget(channelList,1,0,1,1);
    triggerBox->setLayout(boxLayout);
    channelBox->setLayout(boxLayout2);

    layout->addWidget(triggerBox,0,0,1,1);
    layout->addWidget(channelBox,1,0,1,1);

    singleEventModeBox = new QCheckBox (tr ("Single event mode"));
    connect (singleEventModeBox, SIGNAL(toggled(bool)), RunManager::ptr (), SLOT(setSingleEventMode(bool)));
    layout->addWidget (singleEventModeBox,2,0,1,1);

    runSetup->setLayout(layout);
    addRunPageToTree(runSetup);
}

void ScopeMainWindow::createRunControlPage()
{
    QWidget* runControl = new QGroupBox(tr("Run Control"));
    runControl->setAccessibleName(tr("Run Control"));
    QGridLayout* layout = new QGridLayout();

    QLabel* runNameLabel = new QLabel(tr("Run name:"));
    runNameEdit = new QLineEdit(RunManager::ptr ()->getRunName ());

    QGroupBox* box1 = new QGroupBox(tr("Timing:"));
        QGridLayout* box1l = new QGridLayout();
        QLabel* runStartTimeLabel = new QLabel(tr("Start:"));
        QLabel* runStopTimeLabel = new QLabel(tr("Stop:"));
        startTimeEdit = new QDateTimeEdit();
        stopTimeEdit = new QDateTimeEdit();
        startTimeEdit->setDisplayFormat("dd.MM. yyyy hh:mm:ss");
        stopTimeEdit->setDisplayFormat("dd.MM. yyyy hh:mm:ss");
        startTimeEdit->setReadOnly(true);
        stopTimeEdit->setReadOnly(true);        
        box1l->addWidget(runStartTimeLabel,0,0,1,1);
        box1l->addWidget(runStopTimeLabel,1,0,1,1);
        box1l->addWidget(startTimeEdit,0,1,1,1);
        box1l->addWidget(stopTimeEdit,1,1,1,1);
    box1->setLayout(box1l);

    QGroupBox* box2 = new QGroupBox(tr("Events:"));
        QGridLayout* box2l = new QGridLayout();
        QLabel* nofEventsLabel = new QLabel(tr("Events:"));
        QLabel* eventsPerSecondLabel = new QLabel(tr("Ev/s:"));
        nofEventsEdit = new QLineEdit(0);
        nofEventsEdit->setReadOnly(true);
        eventsPerSecondEdit = new QLineEdit(0);
        eventsPerSecondEdit->setReadOnly(true);
        box2l->addWidget(nofEventsLabel,0,0,1,1);
        box2l->addWidget(eventsPerSecondLabel,1,0,1,1);
        box2l->addWidget(nofEventsEdit,0,1,1,1);
        box2l->addWidget(eventsPerSecondEdit,1,1,1,1);
    box2->setLayout(box2l);

    runStartButton = new QPushButton(tr("Start Run"));
    connect(runStartButton,SIGNAL(clicked()), SLOT(startAcquisition()));
    runNameButton = new QPushButton(tr("..."));
    connect(runNameButton,SIGNAL(clicked()),this,SLOT(runNameButtonClicked()));
    connect(runNameEdit, SIGNAL(textChanged(QString)), RunManager::ptr (), SLOT(setRunName(QString)));

    QGroupBox* box3 = new QGroupBox(tr("Notes:"));
        QGridLayout* box3l = new QGridLayout();
        runInfoEdit = new QTextEdit();
        box3l->addWidget(runInfoEdit,0,0,1,1);
    box3->setLayout(box3l);


    layout->addWidget(runNameLabel,     0,0,1,1);
    layout->addWidget(runNameEdit,      0,1,1,2);
    layout->addWidget(runNameButton,    0,3,1,1);
    layout->addWidget(box1,             1,0,1,2);
    layout->addWidget(box2,             1,2,1,2);
    layout->addWidget(box3,             2,0,1,3);
    layout->addWidget(runStartButton,   2,3,1,1);

    runControl->setLayout(layout);

    addRunPageToTree(runControl);
}

void ScopeMainWindow::createRemoteControlPage()
{
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

    remoteRunStartButton->setEnabled(false);
    remoteRunNameButton->setEnabled(false);
    remoteRunInfoEdit->setEnabled(false);
    remoteRunNameEdit->setEnabled(false);

    remoteControl->setLayout(layout);

    addRunPageToTree(remoteControl);
}

void ScopeMainWindow::remoteRunStartClicked()
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

void ScopeMainWindow::remoteConnectClicked()
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

void ScopeMainWindow::setCurrentRemoteAddress(QHostAddress newAddress)
{
    geckoremote->setRemote (newAddress);
    std::cout << "Changed current remote address to " << newAddress.toString().toStdString() << std::endl;
}

void ScopeMainWindow::remoteIpAddressChanged(int idx)
{
    if(idx != -1)
    {
        QVariant var = remoteIpAddressEdit->itemData(idx);
        setCurrentRemoteAddress(var.value<QHostAddress>());
    }
}

void ScopeMainWindow::remoteIpAddressTextChanged()
{
    QString newIp = remoteIpAddressEdit->lineEdit()->text();
    QHostAddress newAddr (newIp);
    if(!newAddr.isNull () && remoteIpAddressEdit->findData (QVariant::fromValue (newAddr)) == -1)
    {
        remoteIpAddressEdit->addItem(newAddr.toString(),QVariant::fromValue (newAddr));
    }
}

void ScopeMainWindow::discoveredRemote (QHostAddress remote) {
    if (remoteIpAddressEdit->findData (QVariant::fromValue (remote)) == -1) {
        remoteIpAddressEdit->addItem(remote.toString (), QVariant::fromValue (remote));
    }
}

void ScopeMainWindow::remoteUpdateComplete () {
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

void ScopeMainWindow::remoteConnected (QHostAddress controller) {
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

void ScopeMainWindow::remoteDisconnected (QHostAddress controller) {
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

void ScopeMainWindow::remoteStarted (GeckoRemote::StartStopResult res) {
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

void ScopeMainWindow::remoteStopped (GeckoRemote::StartStopResult res) {
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

void ScopeMainWindow::runNameButtonClicked()
{
    setRunName(QFileDialog::getExistingDirectory(this,tr("Choose run name"),
                                                 RunManager::ref().getRunName(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
}

void ScopeMainWindow::setRunName(QString _runName)
{
    runNameEdit->setText(_runName);
    RunManager::ref().setRunName(_runName);
}


void ScopeMainWindow::treeViewClicked(const QModelIndex & idx, const QModelIndex &)
{
    QStandardItem* item = treeModel->itemFromIndex(idx);
    if(!item) return;

    mainArea->setCurrentWidget(item->data().value<QWidget*>());

    if (item->parent () == moduleItem) {
        editModAct->setEnabled (configEditAllowed);
        removeModAct->setEnabled (configEditAllowed);
    } else {
        editModAct->setEnabled (false);
        removeModAct->setEnabled (false);
    }

    if (item->parent () == ifaceItem) {
        editIfAct->setEnabled (configEditAllowed);
        removeIfAct->setEnabled (configEditAllowed);
    } else {
        editIfAct->setEnabled (false);
        removeIfAct->setEnabled (false);
    }

    if (item->parent () == pluginItem) {
        editPlugAct->setEnabled (configEditAllowed);
        removePlugAct->setEnabled (configEditAllowed);
    } else {
        editPlugAct->setEnabled (false);
        removePlugAct->setEnabled (false);
    }

    if (item->parent () == moduleItem || item->parent () == ifaceItem || item->parent () == pluginItem) {
        editAct->setEnabled (configEditAllowed);
        removeAct->setEnabled (configEditAllowed);
    } else {
        editAct->setEnabled (false);
        removeAct->setEnabled (false);
    }

    createAct->setEnabled (configEditAllowed && item->parent () != runItem && item != runItem);
    makeMainIfaceAct->setEnabled(configEditAllowed && item->parent () == ifaceItem);
}

void ScopeMainWindow::triggerListChanged(QTreeWidgetItem* item,int col)
{
    if (!item || col != 0)
        return;

    ScopeChannel* ch = item->data(0,Qt::UserRole + 1).value<ScopeChannel*>();
    ch->setEnabled(item->checkState (0) == Qt::Checked);
}

void ScopeMainWindow::channelListChanged(QTreeWidgetItem* item, int col)
{
    if (!item || col != 0)
        return;

    ScopeChannel* ch = item->data(0,Qt::UserRole + 1).value<ScopeChannel*>();
    if (ch)
        ch->setEnabled(item->checkState (0) == Qt::Checked);
}

void ScopeMainWindow::setStatusText(QString newString)
{
    statusLabel->setText(newString);
}

void ScopeMainWindow::oneSecondTimerTimeout() {
    //std::cout << "CPU: " << sysinfo->getCpuLoad() << std::endl;
    statusCpuLabel->setText(tr("CPU: %1 %").arg((int)(RunManager::ref().getSystemInfo()->getCpuLoad()*100),3));
}

void ScopeMainWindow::loadChannelList()
{
    triggerList->clear();
    channelList->clear();

    QList<QTreeWidgetItem *> trgItems;
    QList<QTreeWidgetItem *> chItems;
    const QList<AbstractModule*>* mlist = mmgr->list();
    QList<ScopeChannel*>* trgCh;
    QList<AbstractModule*>::const_iterator it(mlist->begin());

    for(;it != mlist->end(); ++it)
    {
        AbstractModule* curModule = *it;
        if (!curModule) continue;

        trgCh = curModule->getChannels();
        QList<ScopeChannel*>::iterator ch(trgCh->begin());

        for(;ch != trgCh->end();++ch)
        {
            ScopeChannel* curCh = (*ch);

            QStringList vals;
            vals.append(curModule->getName());
            vals.append(curCh->getName());

            QList<QTreeWidgetItem*>* l = &chItems;
            QTreeWidget *tw = channelList;

            switch (curCh->getType()) {
            case ScopeCommon::trigger:
                l = &trgItems;
                tw = triggerList;
                break;
            case ScopeCommon::eventBuffer:
                vals.append (tr ("Event Buffer"));
                break;
            case ScopeCommon::logic:
                vals.append (tr ("Logic"));
                break;
            case ScopeCommon::trace:
                vals.append (tr ("Trace"));
                break;
            default:
                vals.append (tr ("Unknown"));
                break;
            }

            QTreeWidgetItem *item = new QTreeWidgetItem(vals);
            l->append (item);
            item->setData(0,Qt::UserRole + 1,QVariant::fromValue(curCh));
            item->setCheckState(0, curCh->isEnabled() ? Qt::Checked : Qt::Unchecked);
            tw->addTopLevelItem (item);
        }
    }

    triggerList->addTopLevelItems(trgItems);
    channelList->addTopLevelItems(chItems);
    singleEventModeBox->setChecked (RunManager::ref ().isSingleEventMode ());
}

void ScopeMainWindow::updateRunPage(float evspersec, unsigned evs)
{
    nofEventsEdit->setText(tr("%1").arg(evs));
    eventsPerSecondEdit->setText(tr("%1").arg(evspersec, 0, 'f', 1));
}

void ScopeMainWindow::runStarted () {
    nofEventsEdit->setText ("0");
    eventsPerSecondEdit->setText ("0");

    runStartButton->disconnect ();
    connect (runStartButton, SIGNAL(clicked()), SLOT(stopAcquisition()));
    runStartButton->setText (tr("Stop Run"));

    statusLabel->setText (tr("Run started"));
    startTimeEdit->setDateTime (RunManager::ptr ()->getStartTime ());
    stopTimeEdit->setDateTime (QDateTime ());

    setConfigEnabled (false);
}

void ScopeMainWindow::runStopping () {
    runStartButton->setEnabled(false);
    runStartButton->setText(tr("Stopping Run"));
}

void ScopeMainWindow::runStopped () {
    runStartButton->disconnect ();
    runStartButton->setText (tr("Start Run"));
    connect (runStartButton, SIGNAL(clicked()), SLOT(startAcquisition()));
    runStartButton->setEnabled (true);

    setConfigEnabled (true);

    stopTimeEdit->setDateTime (RunManager::ptr ()->getStopTime());
    statusLabel->setText(tr("Run stopped"));
}

void ScopeMainWindow::setConfigEnabled (bool enabled) {
    triggerList->setEnabled (enabled);
    channelList->setEnabled (enabled);

    runNameEdit->setEnabled (enabled);
    runNameButton->setEnabled (enabled);

    foreach (AbstractPlugin *p, *PluginManager::ptr ()->list ())
        p->setConfigEnabled (enabled);

    foreach (AbstractModule *m, *ModuleManager::ptr ()->list ())
        m->getUI ()->setEnabled (enabled);

    // update treeview actions
    configEditAllowed = enabled;
    treeViewClicked (treeView->currentIndex (), QModelIndex ());
}

void ScopeMainWindow::startAcquisition () {
    RunManager::ptr()->start (runInfoEdit->toPlainText ());
}

void ScopeMainWindow::stopAcquisition () {
    RunManager::ptr ()->stop ();
}

void ScopeMainWindow::applySettings()
{
    loadConfig (settings);
    mmgr->applySettings(settings);
    pmgr->applySettings(settings);

    loadChannelList();

    setWindowTitle("GECKO (" + fileName + ")");
}

void ScopeMainWindow::saveSettings()
{
    settings->clear ();
    saveConfig (settings);
    mmgr->saveSettings(settings);
    pmgr->saveSettings(settings);
    setWindowTitle("GECKO (" + fileName + ")");
}

void ScopeMainWindow::saveSettingsToFile (QString file) {
    QSettings s (file, QSettings::IniFormat);
    saveConfig (&s);
    ModuleManager::ptr()->saveSettings (&s);
    PluginManager::ptr()->saveSettings (&s);
    s.sync ();
}

bool ScopeMainWindow::saveSettingsQuery()
{
    if(fileName != defaultIni)
    {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("GECKO"),
                                   tr("Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void ScopeMainWindow::closeEvent(QCloseEvent *event)
 {
    if (saveSettingsQuery())
    {
        foreach (QWidget *w, QApplication::topLevelWidgets ())
            if (w != this)
                w->close ();
        event->accept();
    } else
    {
        event->ignore();
    }
 }

void ScopeMainWindow::open()
{
    if(saveSettingsQuery())
    {
        QString fileName = QFileDialog::getOpenFileName(this,"Open settings file","","Config files (*.ini)");
        if(!fileName.isEmpty())
        {
            this->fileName = fileName;
            delete settings;
            settings = new QSettings(this->fileName,QSettings::IniFormat);
            applySettings();
        }
    }
}

bool ScopeMainWindow::save()
{
    if(fileName == defaultIni)
    {
        return saveAs();
    }
    else
    {
        saveSettings();
        settings->sync();
        return true;
    }
}

bool ScopeMainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,"Save settings file as...","","Config files (*.ini)");
    if (fileName.isEmpty())
        return false;
    this->fileName = fileName;

    delete settings;
    settings = new QSettings(this->fileName,QSettings::IniFormat);

    saveSettings();
    settings->sync();

    return true;
}

void ScopeMainWindow::exportDot () {
    QString filename = QFileDialog::getSaveFileName (this, tr("Export to GraphViz File..."), "", "GraphViz files (*.gv)");
    if (filename.isEmpty())
        return;

    QFile file (filename);
    if (!file.open (QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning (this, tr("GECKO"), tr("Cannot write to file \"%1\"!").arg(filename));
        return;
    }

    QTextStream out (&file);
    writeDotFile (out);
}

QString dotEscape (QString str) {
    return str.replace ('<', "\\<").replace ('>', "\\>").replace ('|', "\\|").replace (' ', "\\ ");
}

void ScopeMainWindow::writeDotFile (QTextStream &out) {
    QMap<AbstractPlugin*,QString> outputplugins;

    out << "digraph {\n";
    out << "node [shape=record];\n"
           "labelloc=t;\n"
           "label=\"" << dotEscape (this->fileName) << "\";\n";
    foreach (AbstractInterface *m, *InterfaceManager::ref ().list ()) {
        out << "\"if_" << m->getName () << "\" [label=\"" << dotEscape (m->getName ()) << "\",shape=ellipse];\n";
    }

    foreach (AbstractModule *daq, *ModuleManager::ref ().list ()) {
        outputplugins.insert(daq->getOutputPlugin (), daq->getName ());
        QString label = dotEscape (daq->getName ());
        if (!daq->getOutputPlugin ()->getOutputs ()->empty()) {
            QStringList outs;
            foreach (PluginConnector *c, *daq->getOutputPlugin ()->getOutputs ()) {
                QString str = dotEscape (c->getName());
                outs << "<" + str + "> " + str;
            }
            label += "|{" + outs.join ("|") + "}";
        }
        out << "\"mod_" << daq->getName () << "\" [label=\"{" << label << "}\"];\n";
    }

    foreach (AbstractPlugin *p, *PluginManager::ref ().list()) {
        QString label = dotEscape (p->getName ());
        if (!p->getInputs ()->empty()) {
            QStringList ins;
            foreach (PluginConnector *c, *p->getInputs ()) {
                QString str = dotEscape (c->getName());
                ins << "<" + str + "> " + str;
            }
            label.prepend ("{" + ins.join ("|") + "}|");
        }
        if (!p->getOutputs ()->empty()) {
            QStringList outs;
            foreach (PluginConnector *c, *p->getOutputs ()) {
                QString str = dotEscape (c->getName());
                outs << "<" + str + "> " + str;
            }
            label.append ("|{" + outs.join ("|") + "}");
        }
        out << "\"plg_" << p->getName () << "\" [label=\"{" << label << "}\"];\n";
    }

    foreach (AbstractModule *daq, *ModuleManager::ref().list ()) {
        if (daq->getInterface())
            out << "\"if_" << daq->getInterface()->getName() << "\" -> \"mod_" << daq->getName () << "\";\n";
    }

    foreach (AbstractPlugin *p, *PluginManager::ref ().list ()) {
        foreach (PluginConnector *c, (*p->getInputs())) {
            if (!c->hasOtherSide()) continue;
            if (outputplugins.contains (c->getConnectedPlugin ())) {
                out << "\"mod_" << outputplugins.value (c->getConnectedPlugin ());
            } else {
                out << "\"plg_" << c->getConnectedPluginName ();
            }
            out << "\":\"" << c->getOthersideName() << "\":s -> \"plg_" << p->getName ()
                << "\":\"" << c->getName () << "\":n;\n";
        }
    }

    out << "}\n";
}

void ScopeMainWindow::saveConfig (QSettings *s) {
    QMap<AbstractPlugin*,QString> roots;
    int i = 0;
    s->beginGroup ("Configuration");

    s->setValue ("SingleEventMode", RunManager::ref ().isSingleEventMode ());
    if (InterfaceManager::ref ().getMainInterface ())
        s->setValue ("MainInterface", InterfaceManager::ref().getMainInterface()->getName ());

    s->beginWriteArray ("Interfaces");
    foreach (AbstractInterface *m, *InterfaceManager::ref ().list ()) {
        s->setArrayIndex (i++);
        s->setValue ("name", m->getName ());
        s->setValue ("type", m->getTypeName ());
    }
    s->endArray ();

    i = 0;
    s->beginWriteArray ("DAqModules");
    foreach (AbstractModule *daq, *ModuleManager::ref().list()) {
        s->setArrayIndex (i++);
        s->setValue ("name", daq->getName ());
        s->setValue ("type", daq->getTypeName ());
        if (daq->getInterface())
            s->setValue ("iface", daq->getInterface()->getName ());
        s->setValue ("baddr", daq->getBaseAddress ());

        if (daq->getOutputPlugin())
            roots.insert (daq->getOutputPlugin(), daq->getName ());

        s->beginWriteArray ("ScopeChannels");
        for (int j = 0; j < daq->getChannels ()->size (); ++j) {
            s->setArrayIndex (j);
            s->setValue("enabled", daq->getChannels ()->at (j)->isEnabled ());
        }
        s->endArray ();
    }
    s->endArray ();

    i = 0;
    s->beginWriteArray ("Plugins");
    foreach (AbstractPlugin *p, *PluginManager::ref().list()) {
        s->setArrayIndex (i++);
        s->setValue ("name", p->getName ());
        s->setValue ("type", p->getTypeName ());
        if (!p->getAttributes ().empty ())
            s->setValue ("attrs", p->getAttributes ());
    }
    s->endArray ();

    i = 0;
    s->beginWriteArray ("Channels");
    foreach (AbstractPlugin *p, *PluginManager::ref().list()) {
        foreach (PluginConnector *c, *p->getInputs ()) {
            if (c->hasOtherSide()) {
                s->setArrayIndex (i++);
                if (roots.contains (c->getConnectedPlugin ()))
                    s->setValue ("fromdaq", roots.value (c->getConnectedPlugin ()));
                else
                    s->setValue ("from", c->getConnectedPluginName ());
                s->setValue ("fromport", c->getOthersideName ());
                s->setValue ("to", p->getName ());
                s->setValue ("toport", c->getName ());
            }
        }
    }
    s->endArray ();
    s->endGroup ();
}

void ScopeMainWindow::loadConfig (QSettings *s) {
    QMap<QString,AbstractPlugin*> roots;
    int size;
    QStringList fail;

    PluginManager::ref().clear ();
    ModuleManager::ref().clear ();
    InterfaceManager::ref().clear ();

    s->beginGroup ("Configuration");
    RunManager::ref().setSingleEventMode (s->value ("SingleEventMode", false).toBool ());
    size = s->beginReadArray ("Interfaces");
    for (int i = 0; i < size; ++i) {
        s->setArrayIndex (i);
        QString name = s->value ("name").toString ();
        QString type = s->value ("type").toString ();
        AbstractInterface *mod = InterfaceManager::ref ().create (type, name);

        if (!mod)
            fail << tr ("Module type not found: %1").arg (type);
    }
    s->endArray ();

    size = s->beginReadArray ("DAqModules");
    for (int i = 0; i < size; ++i) {
        s->setArrayIndex (i);
        QString name = s->value ("name").toString ();
        QString type = s->value ("type").toString ();

        AbstractModule *daq = ModuleManager::ref().create (type, name);
        if (!daq)
            fail << tr ("Module type not found: %1").arg (type);

        if (s->contains ("iface")) {
            if (InterfaceManager::ref().get (s->value ("iface").toString ())) {
                daq->setInterface (InterfaceManager::ref().get (s->value ("iface").toString ()));
            } else {
                fail << tr ("Interface module not found: %1").arg (s->value ("iface").toString());
            }
        }
        daq->setBaseAddress (s->value ("baddr").toUInt ());

        if (daq->getOutputPlugin ())
            roots.insert (daq->getName (), daq->getOutputPlugin ());

        int chans = s->beginReadArray ("ScopeChannels");
        for (int j = 0; j < chans; ++j) {
            s->setArrayIndex (j);
            daq->getChannels ()->at (j)->setEnabled (s->value("enabled").toBool ());
        }
        s->endArray ();
    }
    s->endArray ();

    size = s->beginReadArray ("Plugins");
    for (int i = 0; i < size; ++i) {
        s->setArrayIndex (i);
        QString name = s->value ("name").toString ();
        QString type = s->value ("type").toString ();
        AbstractPlugin::Attributes attrs =
                s->value ("attrs", AbstractPlugin::Attributes ()).value<AbstractPlugin::Attributes> ();

        if (!PluginManager::ref().create (type, name, attrs))
            fail << tr ("Plugin type not found: %1").arg (type);
    }
    s->endArray ();

    size = s->beginReadArray ("Channels");
    for (int i = 0; i < size; ++i) {
        s->setArrayIndex (i);
        AbstractPlugin *from, *to;
        PluginConnector *fromc = NULL, *toc = NULL;
        QString fromport = s->value ("fromport").toString ();
        QString toport   = s->value ("toport").toString ();
        if (s->contains ("fromdaq"))
            from = roots.value (s->value ("fromdaq").toString ());
        else
            from = PluginManager::ref ().get (s->value ("from").toString ());
        to = PluginManager::ref ().get (s->value ("to").toString ());

        if (!from || !to) {
            fail << tr ("Connection %1:%2 -> %3:%4 failed: %5")
                    .arg ((s->contains ("fromdaq") ? s->value ("fromdaq") : s->value ("from")).toString ())
                    .arg (fromport)
                    .arg (s->value ("to").toString ())
                    .arg (toport)
                    .arg (tr ("Plugin does not exist"));
            continue;
        }


        foreach (PluginConnector *c, *from->getOutputs ()) {
            if (c->getName () == fromport) {
                fromc = c;
                break;
            }
        }

        foreach (PluginConnector *c, *to->getInputs ()) {
            if (c->getName () == toport) {
                toc = c;
                break;
            }
        }

        if (fromc && toc) {
            try {
                fromc->connectTo (toc);
            } catch (std::invalid_argument e) {
                fail << tr ("Connection %1:%2 -> %3:%4 failed: %5")
                        .arg ((s->contains ("fromdaq") ? s->value ("fromdaq") : s->value ("from")).toString ())
                        .arg (fromport)
                        .arg (s->value ("to").toString ())
                        .arg (toport)
                        .arg (e.what ());
            }
        } else {
            fail << tr ("Connection %1:%2 -> %3:%4 failed: %5")
                    .arg ((s->contains ("fromdaq") ? s->value ("fromdaq") : s->value ("from")).toString ())
                    .arg (fromport)
                    .arg (s->value ("to").toString ())
                    .arg (toport)
                    .arg (tr ("Port does not exist"));
        }
    }
    s->endArray ();

    if (s->contains ("MainInterface")) {
        if (InterfaceManager::ref().get (s->value ("MainInterface").toString ()))
            InterfaceManager::ref().setMainInterface(
                InterfaceManager::ref().get (s->value ("MainInterface").toString ()));
        else
            fail << tr ("Main interface does not exist: %1").arg (s->value ("MainInterface").toString ());
    }

    if (!fail.empty ()) {
        QMessageBox mb (QMessageBox::Warning,
          tr ("GECKO"), tr ("The configuration could not be loaded completely!"), QMessageBox::Ok, this);
        mb.setInformativeText (tr ("You may have to fix the errors manually"));
        mb.setDetailedText (fail.join ("\n"));
        mb.exec ();
    }

    s->endGroup ();
}
