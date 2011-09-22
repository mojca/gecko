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

#ifndef CAEN965MODULE_H
#define CAEN965MODULE_H

#include "basemodule.h"
#include "baseplugin.h"
#include "caen965dmx.h"
#include "pluginmanager.h"

#define CAEN_V965_NOF_CHANNELS 16
#define CAEN_V965_MAX_NOF_WORDS 34 // per event

struct Caen965ModuleConfig {
    uint32_t base_addr;

    uint8_t irq_level;
    uint8_t irq_vector;
    uint8_t ev_trg;

    uint8_t thresholds[CAEN_V965_NOF_CHANNELS];
    bool killChannel[CAEN_V965_NOF_CHANNELS];

    uint8_t cratenumber;
    uint16_t fastclear;
    uint8_t i_ped;
    uint8_t slideconst;

    // tdc registers
    uint8_t fsr;
    bool stop_mode;

    // control register 1
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

    Caen965ModuleConfig ()
    : irq_level (0), irq_vector (0), ev_trg (0)
    , cratenumber (0), fastclear (0), i_ped (180), slideconst (0)
    , fsr (0x18), stop_mode (false)
    , block_end (false), berr_enable (true), program_reset (false), align64 (false)
    , memTestModeEnabled (false), offline (false), overRangeSuppressionEnabled (true)
    , zeroSuppressionEnabled (true), slidingScaleEnabled (false), zeroSuppressionThr (false)
    , autoIncrementEnabled (true), emptyEventWriteEnabled (false), slideSubEnabled (false)
    , alwaysIncrementEventCounter (false)
    , pollcount (10000)
    {
        for (int i = 0; i < CAEN_V965_NOF_CHANNELS; ++i) {
            killChannel [i] = false;
            thresholds [i] = 0;
        }
    }
};

class Caen965Module : public BaseModule {
	Q_OBJECT
public:
    // Factory method
    static AbstractModule *create (int id, const QString &name) {
        return new Caen965Module (id, name);
    }

    virtual void saveSettings (QSettings*);
    virtual void applySettings (QSettings*);


    int counterReset ();
    int dataReset ();
    int softReset ();

    virtual void setChannels ();
    virtual int acquire (Event* ev);
    virtual bool dataReady ();
    virtual int reset ();
    virtual int configure ();

    virtual uint32_t getBaseAddress () const;
    virtual void setBaseAddress (uint32_t baddr);

    uint16_t getInfo () const;
    int readStatus ();

    uint16_t getStatus1 () const { return status1; }
    uint16_t getStatus2 () const { return status2; }
    uint16_t getBitset1 () const { return bitset1; }
    uint16_t getBitset2 () const { return bitset2; }
    uint32_t getEventCount () const { return evcnt; }

    Caen965ModuleConfig *getConfig () { return &conf_; }

    int acquireSingle (uint32_t *data, uint32_t *rd);

private:
    Caen965Module (int _id, const QString &);
    void singleShot (uint32_t *data, uint32_t *rd);
    void writeToBuffer(Event *ev);

public slots:
    virtual void prepareForNextAcquisition () {}

private:
    Caen965ModuleConfig conf_;

    mutable uint16_t info_;
    uint16_t bitset1;
    uint16_t bitset2;
    uint16_t status1;
    uint16_t status2;
    uint32_t evcnt;
    uint32_t data [34];
    uint32_t rd;

    Caen965Demux dmx_;
    QVector<EventSlot*> evslots_;
};

#endif // CAEN965MODULE_H
