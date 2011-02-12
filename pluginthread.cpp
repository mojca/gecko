#include <QtConcurrentRun>
#include <QFutureSynchronizer>

#include "pluginthread.h"
#include "abstractmodule.h"
#include "scopechannel.h"

PluginThread::PluginThread(PluginManager* _pmgr, ModuleManager* _mmgr)
        : pmgr(_pmgr), mmgr(_mmgr), nofAcqsWaiting (0)
{
    abort = false;
    moveToThread(this);

    addBuffers();

    createProcessList();

    std::cout << "PluginThread initialized." << std::endl;
}

void PluginThread::addBuffers()
{
    std::cout << "PluginThread adding buffers." << std::endl;
    foreach(AbstractModule* module, (*mmgr->list ()))
    {
        //pmgr->addBuffer(module->getBuffer());

        AbstractPlugin* p = module->getOutputPlugin();
        if(p != NULL)
        {
            int i = 0;
            foreach(PluginConnector* pc, (*p->getOutputs()))
            {
                PluginConnectorThreadBuffered* bpc = dynamic_cast<PluginConnectorThreadBuffered*>(pc);
                if(module->getChannels()->at(i)->isEnabled())
                {
                    bufferList.append(bpc);
                    std::cout << "Appending channel " << module->getChannels()->at(i)->getName().toStdString()
                          << " of module " << module->getName().toStdString()
                          << " (bpc = " << pc->getName().toStdString()
                          << ")"
                          << std::endl;
                }
                i++;
            }
        }
    }

    std::cout << "PluginThread done adding buffers." << std::endl << std::endl;
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
    for(int level = 1; level <= pmgr->list()->size(); level++)
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
        foreach(ThreadBuffer<uint32_t>* buf, (*pmgr->inBuffers))
        {
            buf->disconnect();
        }
        pmgr->inBuffers->clear();

        bool finished = wait(5000);
        if(!finished) terminate();
    }
    std::cout << "PluginThread stopped." << std::endl;
}

void PluginThread::run()
{
//    QList<PluginConnector*>* roots = pmgr->getRootConnectors();
//    unsigned int nofInputs = roots->size();

    unsigned int nofInputs = bufferList.size();

    if(nofInputs > 0)
    {
        std::cout << "PluginThread started." << std::endl;

        for(;;)
        {
            process();
            if(abort) break;
        }
        //exec();
    }
    else
    {
        std::cout << "No plugin connected." << std::endl;
        sleep(1000);
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

        int cnt = 0;

        foreach(PluginConnectorThreadBuffered* bpc, bufferList)
        {
            if(bpc->dataAvailable() > 0)
            {
                cnt++;
            }
            else
            {
                //std::cout << "PluginThread: " << bpc->getPlugin()->getName().toStdString() << ":" << bpc->getName().toStdString() << " not ready.\n";
            }
        }
        if(cnt >= bufferList.size())
        {
            execProcessList();
        }
        else
        {
            // throw away excess data. FIXME: Is this always the right thing to do? (multi-event mode?)
            foreach (AbstractModule *m, (*mmgr->list ())) {
                foreach (PluginConnector *pc, (*m->getOutputPlugin()->getOutputs())) {
                    PluginConnectorThreadBuffered *bpc = dynamic_cast<PluginConnectorThreadBuffered*> (pc);
                    while (bpc->dataAvailable()) {
                        bpc->getDataDummy ();
                        bpc->useData ();
                    }
                }
            }
        }
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
