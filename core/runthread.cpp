#include "runthread.h"

#include "abstractmodule.h"
#include "modulemanager.h"
#include "interfacemanager.h"
#include "runmanager.h"
#include "abstractinterface.h"
#include "eventbuffer.h"

RunThread::RunThread () {

    triggered = false;
    running = false;
    abort = false;

    interruptBased = false;
    pollBased = false;

    moveToThread(this);

    nofPolls = 0;
    nofSuccessfulEvents = 0;

    std::cout << "Run thread initialized." << std::endl;
}

RunThread::~RunThread()
{
    bool finished = wait(5000);
    if(!finished) terminate();
    std::cout << "Run thread stopped." << std::endl;
}


void RunThread::run()
{
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
    Q_UNUSED(_trg)

    //std::cout << currentThreadId() << ": Run thread acquiring." << std::endl;

    QList<AbstractModule*>::iterator m(modules.begin());

    Event *ev = RunManager::ref ().getEventBuffer ()->createEvent ();
    while(m != modules.end())
    {
        AbstractModule* curM = (*m);
        if (curM->dataReady ())
            curM->acquire(ev);
        m++;
    }

    if (QSet<const EventSlot*>::fromList (mandatories).subtract(ev->getOccupiedSlots ()).empty()) {
        RunManager::ref ().getEventBuffer ()->queue (ev);
        emit acquisitionDone();
        return true;
    } else {
        delete ev;
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
    InterfaceManager *imgr = InterfaceManager::ptr ();
    while(!abort)
    {
        nofPolls++;
        foreach(AbstractModule* trg, triggers)
        {
            if(trg->dataReady())
            {
                imgr->getMainInterface()->setOutput1(true);
                if (acquire(trg))
                    nofSuccessfulEvents++;
                imgr->getMainInterface()->setOutput1(false);
            }
        }
    }
}
