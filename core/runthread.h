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

#ifndef RUNTHREAD_H
#define RUNTHREAD_H

#include <stdint.h>

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
