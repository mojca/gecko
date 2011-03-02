#include "runthread.h"

#include "abstractmodule.h"
#include "modulemanager.h"
#include "interfacemanager.h"
#include "abstractinterface.h"
#include "scopechannel.h"

RunThread::RunThread () {
    qRegisterMetaType<ScopeChannel*>("ScopeChannel*");

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
    createLists();
    createConnections();

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

void RunThread::createLists()
{
    triggerList = new QList<ScopeChannel*>;
    channelList = new QList<ScopeChannel*>;
    moduleList  = new QList<AbstractModule*>;

    const QList<AbstractModule*>* mlist = ModuleManager::ptr ()->list ();
    QList<ScopeChannel*>* trgCh;
    QList<AbstractModule*>::const_iterator it(mlist->begin());


    for(;it != mlist->end(); ++it)
    {
        AbstractModule* curModule = *it;
        if (!curModule) continue;
        trgCh = curModule->getChannels();
        QList<ScopeChannel*>::iterator ch(trgCh->begin());

        curModule->reset();
        curModule->configure();

        while(ch != trgCh->end())
        {
            ScopeChannel* curCh = (*ch);

            if(curCh->isEnabled())
            {
                if(curCh->getType() != ScopeCommon::trigger)
                {
                    if(!moduleList->contains(curModule)) moduleList->append(curModule);
                    channelList->append(curCh);
                }
                else
                {
                    if(curCh->getType() == ScopeCommon::trigger)
                    {
                        pollBased = true;
                    }
                    if(curCh->getType() == ScopeCommon::interrupt)
                    {
                        interruptBased = true;
                    }
                    triggerList->append(curCh);
                }
            }
            ch++;
        }
    }

    if(interruptBased && pollBased)
    {
        interruptBased = false;
        QMessageBox::warning(0,tr("Scope"),tr("Defaulting to pollbased readout."),QMessageBox::Ok,QMessageBox::Ok);
    }
}

void RunThread::createConnections()
{
    QList<ScopeChannel*>::iterator ch(triggerList->begin());

    while(ch != triggerList->end())
    {
        ScopeChannel* curCh = (*ch);
        connect(curCh->getModule(),SIGNAL(triggered(ScopeChannel*)),this,SLOT(acquire(ScopeChannel*)));
        ch++;
    }
}

void RunThread::acquire(ScopeChannel* _ch)
{
    Q_UNUSED(_ch)

    //std::cout << currentThreadId() << ": Run thread acquiring." << std::endl;

    QList<AbstractModule*>::iterator m(moduleList->begin());

    while(m != moduleList->end())
    {
        AbstractModule* curM = (*m);
        curM->acquire();
        m++;
    }
    emit acquisitionDone();
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
        foreach(ScopeChannel* trg, (*triggerList))
        {
            if(trg->getModule()->dataReady())
            {
                imgr->getMainInterface()->setOutput1(true);
                acquire(trg);
                imgr->getMainInterface()->setOutput1(false);
                nofSuccessfulEvents++;
            }
        }
    }
}
