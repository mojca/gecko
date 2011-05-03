#include "runmanager.h"

#include <QThreadPool>
#include <QTimer>
#include <QDir>
#include <QFile>

#include "scopemainwindow.h"
#include "runthread.h"
#include "pluginthread.h"
#include "interfacemanager.h"
#include "abstractmodule.h"
#include "abstractinterface.h"
#include "systeminfo.h"
#include "eventbuffer.h"
#include "outputplugin.h"

#include <stdexcept>
#include <iostream>

RunManager *RunManager::inst = NULL;

RunManager *RunManager::ptr () {
    if (inst == NULL)
        inst = new RunManager();
    return inst;
}

RunManager &RunManager::ref () {
    return *ptr ();
}

RunManager::RunManager()
: singleeventmode (false)
, running (false)
, localRun (true)
, runName ("/tmp")
, evcnt (0)
, lastevcnt (0)
, mainwnd (NULL)
, runthread (NULL)
, pluginthread (NULL)
, updateTimer (new QTimer (this))
, sysinfo (new SystemInfo ())
, evbuf (new EventBuffer (10))
{
    state.resize(2);
    state.setBit(StateRunning,false);
    state.setBit(StateRemoteControlled,false);

    updateTimer->setInterval (500);
    connect (updateTimer, SIGNAL(timeout()), SLOT(sendUpdate()));
    std::cout << "Instantiated RunManager" << std::endl;
}

RunManager::~RunManager()
{

}

void RunManager::setRunName (QString newValue) {
    if(!running)
        runName = newValue;
    else
        throw std::logic_error ("cannot set run name while run is active");
}

void RunManager::start (QString info) {
    if (running)
        throw std::logic_error ("start while running");

    runInfo = info;
    running = true;
    state.setBit(StateRunning,true);

    foreach(AbstractInterface* iface, (*InterfaceManager::ref ().list ()))
    {
        if(!iface->isOpen())
            iface->open();
    }

    runthread = new RunThread ();

    // FIXME
    foreach (AbstractModule *m, *ModuleManager::ref().list ()) {
        connect(runthread,SIGNAL(acquisitionDone()), m, SLOT(prepareForNextAcquisition()));
    }

    startTime = QDateTime::currentDateTime ();
    evcnt = 0;
    lastevcnt = 0;
    evpersec = 0;
    writeRunStartFile (info);

    pluginthread = new PluginThread(PluginManager::ptr (), ModuleManager::ptr ());
    connect (runthread, SIGNAL(acquisitionDone()), pluginthread, SLOT(acquisitionDone()), Qt::DirectConnection);
    pluginthread->start(QThread::NormalPriority);

    runthread->start(QThread::TimeCriticalPriority);

    updateTimer->start ();
    emit runStarted ();
}

void RunManager::stop () {
    if (!running)
        return;

    emit runStopping ();

    updateTimer->stop ();
    runInfo = QString ();

    runthread->stop ();
    runthread->wait (1000);
    pluginthread->stop ();
    pluginthread->wait (1000);

    stopTime = QDateTime::currentDateTime ();
    writeRunStopFile ();

    delete runthread;
    runthread = NULL;

    delete pluginthread;
    pluginthread = NULL;

    // Reset buffers
    foreach(AbstractModule* m, (*ModuleManager::ref ().list ()))
    {
        foreach(PluginConnector* bpc, (*m->getOutputPlugin()->getOutputs()))
        {
            bpc->reset();
        }
    }

    while (!evbuf->empty())
        delete evbuf->dequeue ();

    // close interfaces
    foreach(AbstractInterface* iface, (*InterfaceManager::ref ().list ()))
    {
        if(iface->isOpen())
            iface->close ();
    }

    running = false;
    state.setBit(StateRunning,false);

    emit runStopped ();
}

float RunManager::getEventRate () const {
    return (1000.0 * (evcnt - lastevcnt)) / updateTimer->interval ();
}

void RunManager::sendUpdate () {
    unsigned newev = runthread->getNofEvents ();
    //float evpersec = (1000.0 * (newev - evcnt)) / updateTimer->interval ();
    //float evpersec = ((evcnt) / (double)(getRunSeconds()) ) ; // New algo
    // exponentially decaying average
    evpersec = 0.9 * evpersec + 0.1 * (1000.0 * (newev - evcnt)) / updateTimer->interval ();
    lastevcnt = evcnt;
    evcnt = newev;

    emit runUpdate(evpersec, newev);
}

QString RunManager::stateToString (State _state) const
{
    switch(_state)
    {
    case StateRunning:
        return QString("running");
    case StateRemoteControlled:
        return QString("remoteControlled");
    default:
        return QString("unknown");
    }
    return 0;
}

const QString RunManager::getStateString() const {
    QStringList list;
    for(int i = 0; i < state.size(); i++)
    {
        if(state.testBit(i) == true) list.append(stateToString((State)i));
        else list.append("not_"+stateToString((State)i));
    }
    return list.join(" ");
}

void RunManager::writeRunStartFile (QString info)
{
    QDir runDir(runName);
    if(!runDir.exists())
    {
        if(!runDir.mkpath(runName))
        {
            throw std::runtime_error ("cannot create run directory");
        }
    }

    QFile file(runName+"/start.info");
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QStringList infolines (info.trimmed().split('\n'));
        for (QStringList::iterator i = infolines.begin(); i != infolines.end (); ++i)
            i->prepend ("#  ");
        QTextStream out(&file);
        out << "# " "Run Start File generated by Scope application" << "\n"
            << "# " "Run Name: " << runName << "\n"
            << "# " "Start Time: " << startTime.toString() << "\n"
            << "# " "Single event mode: " << singleeventmode << "\n"
            << "# " "Notes: " << "\n"
            << infolines.join ("\n") << "\n"
            ;
    }


    // Save settings for run
    QString tmpFileName = runName+"/settings.ini";
    mainwnd->saveSettingsToFile (tmpFileName);
}

void RunManager::writeRunStopFile () {
    QDir runDir(runName);
    if(!runDir.exists())
    {
        if(!runDir.mkpath(runName))
        {
            throw std::runtime_error ("cannot create run directory");
        }
    }

    QFile file(runName+"/stop.info");
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << "# " << "Run Stop File generated by Scope application" << "\n"
            << "# " << "Run Name: " << runName << "\n"
            << "# " << "Stop Time: " << stopTime.toString() << "\n"
            << "# " << "Duration: " << startTime.secsTo(stopTime) << " s" << "\n"
            << "# " << "Number of recorded events: " << runthread->getNofEvents() << "\n"
            ;
    }
}
