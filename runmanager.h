#ifndef RUNMANAGER_H
#define RUNMANAGER_H

#include <stdint.h>

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QBitArray>

class RunThread;
class PluginThread;
class QTimer;
class QBitArray;
class ScopeMainWindow;
class SystemInfo;

/*! Manages data acquisition runs.
 *  Each time the user starts a run a start file is written to the run directory,
 *  containing the run name, start time and additional (user-supplied) information.
 *  When the user stops a run, a stop file is written, containing the stop time
 *  and the number of events collected during the run.
 *
 *  All data written by plugins should be stored in the run directory. That way,
 *  the conditions under which the data was acquired are well-documented.
 */
class RunManager : public QObject
{
    Q_OBJECT

    enum State{StateRunning,StateRemoteControlled};

    bool singleeventmode;
    bool running;

    /*! If true, then the run is done on the local machine,
      * if false, a remote instance is controlled.
      */
    bool localRun;
    QString runName;
    QString runInfo;
    QDateTime startTime;
    QDateTime stopTime;
    unsigned evcnt;
    unsigned lastevcnt;
    unsigned evprocessedcnt;

    ScopeMainWindow *mainwnd;

    RunThread *runthread;
    PluginThread *pluginthread;
    QTimer *updateTimer;
    SystemInfo *sysinfo;

public:

    static RunManager *ptr (); /*!< Return a pointer to the singleton */
    static RunManager &ref (); /*!< Return a reference to the singleton */

    // is and get
    /*! Returns whether a run is currently active */
    bool isRunning() const {return running;}
    /*! Returns whether the run is currently local */
    bool isLocal() const {return localRun;}
    /*! Returns whether the run is currently remotely controlled */
    bool isRemoteControlled() const {return state.at(StateRemoteControlled);}
    /*! Returns the name of the current run */
    const QString getRunName() const {return runName;}
    /*! Returns the info text for the current run */
    const QString getRunInfo() const {return runInfo;}
    /*! Returns a string containing the current states */
    const QString getStateString() const;
    /*! Returns the start time of the currently active run */
    const QDateTime getStartTime() const {return startTime;}
    /*! Returns the stop time of the most recently stopped run */
    const QDateTime getStopTime() const {return stopTime;}
    /*! Returns the run time of the currently active run */
    int getRunSeconds() const {return startTime.secsTo(QDateTime::currentDateTime());}
    /*! Returns the number of events collected during this run. */
    unsigned getEventCount () const {return evcnt;}
    /*! Returns the current event rate. */
    float getEventRate () const;
    /*! Returns whether single event mode is active.
     *  In single event mode, only the first event on each module is processed in each acquisition round.
     *  The remaining events are discarded.
     */
    bool isSingleEventMode () const { return singleeventmode; }
    /*! Returns a pointer to a SystemInfo object for reading the current cpu/net load. */
    const SystemInfo *getSystemInfo () const {return sysinfo;}

    // set
    /*! sets a pointer to the main window for saving the active settings to the run directory.
     *  \note Only to be called from the ScopeMainWindow constructor.
     */
    void setMainWindow (ScopeMainWindow *mw) { mainwnd = mw; }
    void setRemoteControlled(bool val) { state.setBit(StateRemoteControlled, val); }

    ~RunManager();

public slots:
    // control
    /*! Starts a run. The info string will be appended to the start info file*/
    void start(QString info);
    /*! Stops the currently active run. */
    void stop();
    /*! Changes the name of the run. */
    void setRunName(QString newValue);
    /*! Activates single event mode, where only the first event of each acquisition cycle is kept */
    void setSingleEventMode (bool sem) { singleeventmode = sem; }
    /*! Activate local or remote mode */
    void setLocalMode (bool lm) { localRun = lm; }
    void setRemoteMode (bool lm) { localRun = !lm; }

signals:
    void runStarted (); /*!< Signalled when a run has started. */
    /*! Signalled when the run has been requested to stop.
     *  When the run and plugin threads finish, runStopped will be signalled.
     */
    void runStopping ();
    void runStopped (); /*!< Signalled when the run has stopped. */

    /*! Signalled in regular intervals, informing the slots about the current run state.
     *  \param curevspersec The average number of events per second, averaged over the time since the last update
     *  \param totalevs The total number of events collected since run start
     *  This signal is currently emitted every 500ms while a run is active.
     */
    void runUpdate (float curevspersec, unsigned totalevs);

private:
    void writeRunStartFile (QString info);
    void writeRunStopFile ();
    QString stateToString(State) const;

private slots:
    void sendUpdate ();


private:
    RunManager();

    static RunManager *inst;
    QBitArray state;

private: // no copying
    RunManager(const RunManager &);
    RunManager &operator=(const RunManager &);
};

#endif // RUNMANAGER_H
