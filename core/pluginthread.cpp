#include <QtConcurrentRun>
#include <QFutureSynchronizer>

#include "pluginthread.h"
#include "abstractmodule.h"
#include "outputplugin.h"
#include "runmanager.h"
#include "eventbuffer.h"

PluginThread::PluginThread(PluginManager* _pmgr, ModuleManager* _mmgr)
        : pmgr(_pmgr), mmgr(_mmgr), nofAcqsWaiting (0), maxDepth (0)
{
    abort = false;
    moveToThread(this);

    createProcessList();

    std::cout << "PluginThread initialized." << std::endl;
}

void PluginThread::createProcessList()
{
    int level = 0;
    processList.clear();
    foreach(AbstractModule* module, (*mmgr->list ()))
    {
        processList.insert(module->getOutputPlugin(),level);
    }
    addChildrenToProcessList();

    std::cout << "ProcessList: " << std::endl;
    QMap<AbstractPlugin*, int>::const_iterator i = processList.constBegin();
    while (i != processList.constEnd())
    {
        AbstractPlugin* p = i.key();
        std::cout << p->getName().toStdString() << ": " << i.value() << std::endl;
        ++i;
    }
    std::cout << std::endl;
}

void PluginThread::addChildrenToProcessList()
{
    //std::cout << "pmgr->list()->size(): " << pmgr->list()->size() << std::endl;
    for(int level = 1; level <= pmgr->list()->size() + 1; level++)
    {
        QMap<AbstractPlugin*, int>::const_iterator i = processList.constBegin();
        while (i != processList.constEnd())
        {
            if(i.value() == level-1)
            {
                maxDepth = level;
                foreach(PluginConnector* out, (*i.key()->getOutputs()))
                {
                    AbstractPlugin* p = out->getConnectedPlugin();
                    if(p != NULL)
                    {
                        processList.insert(p,level);
                    }
                    else
                    {
                        unconnectedList.push_back(out);
                    }
                }
            }
            ++i;
        }   
    }
    if(processList.values().contains(pmgr->list()->size()+1))
    {
        std::cout << "WARNING: ProcessList might be cyclic!! " << std::endl;
    }
}

PluginThread::~PluginThread()
{
    if(currentThread() != this)
    {
        stop ();
        bool finished = wait(5000);
        if(!finished) terminate();
    }
    std::cout << "PluginThread stopped." << std::endl;
}

void PluginThread::run()
{
    std::cout << "PluginThread started." << std::endl;
    if(maxDepth == 0)
        std::cout << "No plugins connected." << std::endl;

    for(;;)
    {
        process();
        if(abort) break;
    }
}

void PluginThread::stop()
{
    if(currentThread() != this)
    {
        std::cout << "PluginThread invoking stop." << std::endl;
        //QMetaObject::invokeMethod(this, "stop",Qt::QueuedConnection);
        mutex.lock();
        abort = true;
        cond.wakeAll();
        mutex.unlock();
    }
    else
    {
        //exit(0);
        std::cout << "PluginThread stopping." << std::endl;
    }
}

void PluginThread::process()
{
    //std::cout << " ### PluginThread processing. " << std::endl;
    mutex.lock();
    if(nofAcqsWaiting > 0)
    {
        //std::cout << ".... " << q.size() << " ";
        --nofAcqsWaiting;
        mutex.unlock();

        Event *ev = RunManager::ref ().getEventBuffer ()->dequeue ();

        // pass data to the output plugins
        QList<AbstractModule *> mods (*ModuleManager::ref ().list ());
        foreach (AbstractModule *m, mods)
            m->getOutputPlugin()->latchData (ev);

        execProcessList();
        RunManager::ref ().getEventBuffer ()->releaseEvent (ev);
    }
    else
    {
        cond.wait(&mutex);
        mutex.unlock();
    }
}

void PluginThread::execProcessList()
{
    //std::cout << "PluginThread::execProcessList" << std::endl;

    int level = 1; // Level 0 plugins are never executed in this thread
    while(level <= maxDepth)
    {

        if(false && processList.keys(level).size() > 1)
        {
            QFutureSynchronizer<void> fsync;
            foreach(AbstractPlugin* p, processList.keys(level))
            {
                fsync.addFuture(QtConcurrent::run(p, &AbstractPlugin::process));
            }
        }
        else
        {
            foreach(AbstractPlugin* p, processList.keys(level))
            {
                p->process();
            }
        }

        level++;
    }

    // Use all unconnected channels
    foreach(AbstractModule* module, (*mmgr->list ()))
    {
        foreach(PluginConnector* out, (*module->getOutputPlugin()->getOutputs()))
        {
            AbstractPlugin* p = out->getConnectedPlugin();
            if(p == NULL)
            {
                //std::cout << "Trying to delete data from " << out->getName() << std::endl;
                out->getDataDummy();
                out->useData();
            }
        }
    }

    // Use all queues from unconnected plugins
    foreach(PluginConnector* c, unconnectedList)
    {
        c->useData();
    }
}

void PluginThread::acquisitionDone () {
    QMutexLocker locker (&mutex);
    ++nofAcqsWaiting;
    cond.wakeAll ();
}
