#ifndef RUNTHREAD_H
#define RUNTHREAD_H


#include <QMutex>
#include <QThread>
#include <iostream>
#include <QMetaType>
#include <QMessageBox>

class QSettings;
class AbstractModule;
class EventSlot;

/*! The RunThread waits for a AbstractPlugin::dataReady from the modules marked as triggers
 *  and acquires data for processing by the plugin thread.
 */
class RunThread : public QThread
{
    Q_OBJECT

public:
    RunThread();
    ~RunThread();

    void createConnections();

    void applySettings(QSettings*);
    void saveSettings(QSettings*);

    uint64_t getNofEvents() {return nofSuccessfulEvents;}

public slots:
    bool acquire(AbstractModule*);
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

    uint64_t nofSuccessfulEvents;
    uint64_t nofPolls;

    QList<AbstractModule*> modules;
    QList<AbstractModule*> triggers;
    QList<const EventSlot *> mandatories;


    QMutex mutex;
};

#endif // RUNTHREAD_H
