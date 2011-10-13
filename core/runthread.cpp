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

#include "runthread.h"

#include "abstractmodule.h"
#include "modulemanager.h"
#include "interfacemanager.h"
#include "runmanager.h"
#include "abstractinterface.h"
#include "eventbuffer.h"

#include <QCoreApplication>
#include <sched.h>
#include <cstdio>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/resource.h>

#define GECKO_PROFILE_RUN

#ifdef GECKO_PROFILE_RUN
#include <time.h>
static struct timespec starttime;
static uint64_t timeinacq;
static uint64_t timeinpoll;
static uint64_t timeForModule[20];
#endif

RunThread::RunThread () {

    triggered = false;
    running = false;
    abort = false;

    interruptBased = false;
    pollBased = false;

    setObjectName("RunThread");

    moveToThread(this);

    nofPolls = 0;
    nofSuccessfulEvents = 0;

    std::cout << "Run thread initialized." << std::endl;
}

RunThread::~RunThread()
{
    bool finished = wait(5000);
    if(!finished) terminate();

#ifdef GECKO_PROFILE_RUN
    struct timespec et;
    clock_gettime(CLOCK_MONOTONIC, &et);
    uint64_t rt = (et.tv_sec - starttime.tv_sec) * 1000000000 + (et.tv_nsec - starttime.tv_nsec);
    std::cout << "Runtime: " << (rt * 1e-9) <<" s, Acq%: " << (100.* timeinacq / rt) << ", Unsuccessful%: "
              << (100. * (nofPolls - nofSuccessfulEvents) / nofPolls)
              << " Polls per event: " << (1.*nofPolls/nofSuccessfulEvents)
              << std::endl;
    for(int i = 0; i < 10; ++i) {
        std::cout << "Time for module " << i << ": " << (100. * timeForModule[i] / rt) << "%" << std::endl;
    }
#endif

    std::cout << "Run thread stopped." << std::endl;
}


void RunThread::run()
{
    // Scheduling magic
    int cpu = sched_getcpu();
    printf("cpu: %d\n",cpu);

    __pid_t pid = getpid();
    printf("pid: %d\n",(uint32_t)pid);

    int scheduler = sched_getscheduler(pid);
    printf("scheduler: %d\n",scheduler);

    Qt::HANDLE threadId = this->thread()->currentThreadId();
    printf("threadId: 0x%08x\n",(uint)threadId);

    QThread* thisThread = this->thread()->currentThread();
    printf("thisThread: 0x%08x\n",(uint*)thisThread);

    pid_t tid;
    tid = syscall(SYS_gettid);
    printf("tid: %d\n",(uint32_t)tid);

    int threadScheduler = sched_getscheduler(tid);
    printf("thread scheduler: %d\n",threadScheduler);

    rlimit rl_nice;
    getrlimit(RLIMIT_NICE,&rl_nice);
    printf("rlimit nice: %d, %d\n",rl_nice.rlim_cur,rl_nice.rlim_max);
    rlimit rl_prio;
    getrlimit(RLIMIT_RTPRIO,&rl_prio);
    printf("rlimit rtprio: %d, %d\n",rl_prio.rlim_cur,rl_prio.rlim_max);

    modules = *ModuleManager::ref ().list ();
    triggers = ModuleManager::ref ().getTriggers ().toList ();
    mandatories = ModuleManager::ref ().getMandatorySlots ().toList ();
    createConnections();

    foreach (AbstractModule *m, modules) {
        m->reset ();
        if (m->configure ())
            std::cout << "Run Thread: " << m->getName ().toStdString () <<": Configure failed!" << std::endl;
    }

    std::cout << "Run thread started." << std::endl;

#ifdef GECKO_PROFILE_RUN
    clock_gettime (CLOCK_MONOTONIC, &starttime);
    timeinacq = 0;
    timeinpoll = 0;
    for (int i = 0; i < 10; ++i) {
        timeForModule[i] = 0;
    }
#endif
    if(interruptBased)
    {
        exec();
    }
    else
    {
        pollLoop();
    }

    exit(0);
}

void RunThread::createConnections()
{
    QList<AbstractModule*>::iterator ch(triggers.begin());

    while(ch != triggers.end())
    {
        connect(*ch, SIGNAL(triggered(AbstractModule*)),this,SLOT(acquire(AbstractModule*)));
        ch++;
    }
}

bool RunThread::acquire(AbstractModule* _trg)
{
    //std::cout << currentThreadId() << ": Run thread acquiring." << std::endl;
    InterfaceManager *imgr = InterfaceManager::ptr ();
    Event *ev = RunManager::ref ().getEventBuffer ()->createEvent ();

    int modulesz = modules.size ();

    imgr->getMainInterface()->setOutput1(true); // VETO signal for DAQ readout

    for (int i = 0; i < modulesz; ++i)
    {
        AbstractModule* curM = modules [i];
        if (curM == _trg || curM->dataReady ()) {
#ifdef GECKO_PROFILE_RUN
            struct timespec st, et;
            clock_gettime (CLOCK_MONOTONIC, &st);
#endif

            curM->acquire(ev);

#ifdef GECKO_PROFILE_RUN
            clock_gettime (CLOCK_MONOTONIC, &et);
            timeForModule[i] += (et.tv_sec - st.tv_sec) * 1000000000 + (et.tv_nsec - st.tv_nsec);
#endif
        }
    }

    imgr->getMainInterface()->setOutput1(false); // Remove VETO signal for DAQ readout

    if (QSet<const EventSlot*>::fromList (mandatories).subtract(ev->getOccupiedSlots ()).empty()) {
        RunManager::ref ().getEventBuffer ()->queue (ev);
        emit acquisitionDone();
        return true;
    } else {
        RunManager::ref ().getEventBuffer ()->releaseEvent (ev);
        return false;
    }
}

void RunThread::stop()
{
    mutex.lock();
    abort = true;
    mutex.unlock();

    this->exit(0);
    std::cout << "Run thread stopping." << std::endl;
}

void RunThread::pollLoop()
{
    while(!abort)
    {
        nofPolls++;
        foreach(AbstractModule* trg, triggers)
        {
            if(trg->dataReady())
            {

#ifdef GECKO_PROFILE_RUN
                struct timespec st, et;
                clock_gettime (CLOCK_MONOTONIC, &st);
#endif
                if (acquire(trg))
                    nofSuccessfulEvents++;
#ifdef GECKO_PROFILE_RUN
                clock_gettime (CLOCK_MONOTONIC, &et);
                timeinacq += (et.tv_sec - st.tv_sec) * 1000000000 + (et.tv_nsec - st.tv_nsec);
#endif
            }
        }
    }
}
