#ifndef RUNTHREAD_H
#define RUNTHREAD_H


#include <QMutex>
#include <QThread>
#include <iostream>
#include <QMetaType>
#include <QMessageBox>

#include "modulemanager.h"
#include "baseinterfacemodule.h"

class QSettings;

class RunThread : public QThread
{
    Q_OBJECT

public:
    RunThread(ModuleManager*);
    ~RunThread();

    void createLists();
    void createConnections();

    void applySettings(QSettings*);
    void saveSettings(QSettings*);

    uint getNofEvents() {return nofSuccessfulEvents;}

public slots:
    void acquire(ScopeChannel*);
    void stop();

signals:
    void acquisitionDone();

protected:
    void run();
    void pollLoop();

private:

    bool triggered;
    bool running;
    bool abort;

    bool interruptBased;
    bool pollBased;

    ModuleManager* mmgr;

    uint nofSuccessfulEvents;
    uint nofPolls;

    QList<ScopeChannel*>* triggerList;
    QList<ScopeChannel*>* channelList;
    QList<BaseDAqModule*>* moduleList;

    QMutex mutex;
};

#endif // RUNTHREAD_H
