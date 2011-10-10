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

#include <QtConcurrentRun>
#include <QFutureSynchronizer>

#include "pluginthread.h"
#include "abstractmodule.h"
#include "outputplugin.h"
#include "runmanager.h"
#include "eventbuffer.h"

#define GECKO_PROFILE_PLUGIN

#ifdef GECKO_PROFILE_PLUGIN
#include <time.h>
static struct timespec starttime;
static uint64_t timeinwait;
static uint64_t timeForPlugin[40];
#endif

PluginThread::PluginThread(PluginManager* _pmgr, ModuleManager* _mmgr)
        : pmgr(_pmgr), mmgr(_mmgr), nofAcqsWaiting (0)
{
    abort = false;
    moveToThread(this);

    createProcessList();

    std::cout << "PluginThread initialized." << std::endl;
}

void PluginThread::createProcessList()
{
    QMap<AbstractPlugin*, int> processList;
    int maxDepth;

    unconnectedList.clear ();

    // Add output plugins to the list
    foreach(AbstractModule* module, (*mmgr->list ()))
    {
        processList.insert(module->getOutputPlugin(),0);
    }
    addChildrenToProcessList(processList, maxDepth);

    // Output process list
    std::cout << "ProcessList: " << std::endl;
    QMap<AbstractPlugin*, int>::const_iterator i = processList.constBegin();
    while (i != processList.constEnd())
    {
        AbstractPlugin* p = i.key();
        std::cout << p->getName().toStdString() << ": " << i.value() << std::endl;
        ++i;
    }
    std::cout << std::endl;

    // construct lists of AbstractPlugins better suited for use in the process methods
    levelList.clear();
    for (int i = 1; i <= maxDepth; ++i)
    {
        QList<AbstractPlugin*> l = processList.keys (i);
        if (!l.empty())
            levelList.push_back (l);
    }
}

void PluginThread::addChildrenToProcessList(QMap<AbstractPlugin *, int> &processList, int &maxDepth)
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

#ifdef GECKO_PROFILE_PLUGIN
    struct timespec et;
    clock_gettime(CLOCK_MONOTONIC, &et);
    uint64_t rt = (et.tv_sec - starttime.tv_sec) * 1000000000 + (et.tv_nsec - starttime.tv_nsec);
    std::cout << "Runtime: " << (rt * 1e-9) <<" s, Waiting: " << (100.* timeinwait / rt) << "%" << std::endl;
    for(int i = 0; i < 10; ++i) {
        std::cout << "Time for plugin " << i << ": " << (100. * timeForPlugin[i] / rt) << "%" << std::endl;
    }
#endif

    std::cout << "PluginThread stopped." << std::endl;
}

void PluginThread::run()
{
    std::cout << "PluginThread started." << std::endl;
    if(levelList.empty())
        std::cout << "No plugins connected." << std::endl;

    foreach(AbstractModule* module, (*mmgr->list ())) {
        std::cout << "runStartingEvent: " << module->getName().toStdString() << std::endl;
        module->getOutputPlugin()->runStartingEvent();
    }

    foreach (AbstractPlugin *p, *PluginManager::ref().list()) {
        p->runStartingEvent ();
    }

#ifdef GECKO_PROFILE_PLUGIN
    clock_gettime(CLOCK_MONOTONIC, &starttime);
    timeinwait = 0;
    for (int i = 0; i < 10; ++i) {
        timeForPlugin[i] = 0;
    }
#endif

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
    if(!!nofAcqsWaiting)
    {
        //std::cout << ".... " << q.size() << " ";
        nofAcqsWaiting.deref();

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
        QMutexLocker l (&mutex);
#ifdef GECKO_PROFILE_PLUGIN
        struct timespec st, et;
        clock_gettime (CLOCK_MONOTONIC, &st);
#endif
        cond.wait(&mutex);
#ifdef GECKO_PROFILE_PLUGIN
        clock_gettime (CLOCK_MONOTONIC, &et);
        timeinwait += (et.tv_sec - st.tv_sec) * 1000000000 + (et.tv_nsec - st.tv_nsec);
#endif
    }
}

void PluginThread::execProcessList()
{
    //std::cout << "PluginThread::execProcessList" << std::endl;
#ifdef GECKO_PROFILE_PLUGIN
    int i_prof = 0;
#endif
    for (QList< QList<AbstractPlugin*> >::const_iterator i = levelList.begin ();
         i != levelList.end ();
         ++i)
    {

        if(false && i->size() > 1)
        {
            QFutureSynchronizer<void> fsync;
            foreach(AbstractPlugin* p, *i)
            {
                fsync.addFuture(QtConcurrent::run(p, &AbstractPlugin::process));
            }
        }
        else
        {
            foreach(AbstractPlugin* p, *i)
            {
#ifdef GECKO_PROFILE_PLUGIN
                struct timespec st, et;
                clock_gettime (CLOCK_MONOTONIC, &st);
#endif
                p->process();
#ifdef GECKO_PROFILE_PLUGIN
                clock_gettime (CLOCK_MONOTONIC, &et);
                timeForPlugin[i_prof] += (et.tv_sec - st.tv_sec) * 1000000000 + (et.tv_nsec - st.tv_nsec);
                ++i_prof;
#endif
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
    nofAcqsWaiting.ref();
    QMutexLocker locker (&mutex);
    cond.wakeAll ();
}
