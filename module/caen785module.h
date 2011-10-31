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

#ifndef CAEN785MODULE_H
#define CAEN785MODULE_H

#include <QTime>
#include <cstdio>

#include "basemodule.h"
#include "abstractinterface.h"
#include "baseplugin.h"
#include "caen_v785.h"
#include "caen785ui.h"
#include "caenadcdmx.h"
#include "pluginmanager.h"

class Caen785config
{
public:
    uint32_t base_addr;

    uint8_t irq_level;
    uint8_t irq_vector;

    uint8_t thresholds[32];
    bool killChannel[32];

    uint8_t cratenumber;
    uint8_t nof_events;
    uint8_t slide_constant;

    // bit set 1
    bool block_end;
    bool berr_enable;
    bool program_reset;
    bool align64;

    // bit set 2
    bool memTestModeEnabled;
    bool offline;
    bool overRangeSuppressionEnabled;
    bool zeroSuppressionEnabled;
    bool slidingScaleEnabled;
    bool zeroSuppressionThr;
    bool autoIncrementEnabled;
    bool emptyEventWriteEnabled;
    bool slideSubEnabled;
    bool alwaysIncrementEventCounter;

    unsigned int pollcount;

    Caen785config ()
    : base_addr (0), irq_level (0), irq_vector (0), cratenumber (0), nof_events (0), slide_constant (0)
    , block_end (false), berr_enable (true), program_reset (false), align64 (false), memTestModeEnabled (false)
    , offline (false), overRangeSuppressionEnabled (true), zeroSuppressionEnabled (true), slidingScaleEnabled (true)
    , zeroSuppressionThr (false), autoIncrementEnabled (true), emptyEventWriteEnabled (false)
    , slideSubEnabled (false), alwaysIncrementEventCounter (true)
    , pollcount (0)
    {
        for (int i = 0; i < 32; ++i) {
            thresholds [i] = 0;
            killChannel [i] = false;
        }
    }
};

class Caen785Module : public BaseModule
{
    Q_OBJECT

private:

    Caen785Module(int _id, QString _name);

public:
    Caen785config conf;
    uint16_t bit1;
    uint16_t bit2;
    uint16_t status1;
    uint16_t status2;
    uint16_t firmware;
    uint32_t evcntr;
    uint32_t data[34];

    virtual void saveSettings(QSettings*);
    virtual void applySettings(QSettings*);

    void setChannels();

    int dataReset();
    int softReset();
    int counterReset();

    int readStatus();
    int readInfo();

    int readStatus1();
    int readStatus2();

    bool pollTrigger();

    virtual bool dataReady();
    virtual int acquire(Event *);
    virtual int reset() {
        counterReset();
        dataReset();
        return softReset(); }
    virtual int configure();

    virtual uint32_t getBaseAddress () const;
    virtual void setBaseAddress (uint32_t baddr);

    Caen785config *getConfig () { return &conf; }

    void runStartingEvent() { dmx.runStartingEvent(); }

    int singleShot();
    int acquireSingleEvent();
    int acquireSingleEventFIFO();
    int acquireSingleEventMBLT();
    int writeToBuffer(Event *ev, uint32_t nofWords);

    // Factory method
    static AbstractModule *create (int id, const QString &name) {
        return new Caen785Module (id, name);
    }
private:
    QVector<EventSlot*> evslots;
    CaenADCDemux dmx;
};

#endif // CAEN785_H
