#ifndef RUNTHREAD_H
#define RUNTHREAD_H


#include <QMutex>
#include <QThread>
#include <iostream>
#include <QMetaType>
#include <QMessageBox>

class ScopeChannel;
class QSettings;
class AbstractModule;

class RunThread : public QThread
{
    Q_OBJECT

public:
    RunThread();
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

    uint nofSuccessfulEvents;
    uint nofPolls;

    QList<ScopeChannel*>* triggerList;
    QList<ScopeChannel*>* channelList;
    QList<AbstractModule*>* moduleList;

    QMutex mutex;
};

#endif // RUNTHREAD_H
