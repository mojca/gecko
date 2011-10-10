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

#ifndef CAEN820SCALER_H
#define CAEN820SCALER_H

#include "basemodule.h"
#include "caen820dmx.h"

struct Caen820Config {
    uint32_t baddr;
    uint32_t channel_enable;
    uint16_t acq_mode;
    uint32_t dwell_time;
    bool short_data_format;
    bool berr_enable;
    bool hdr_enable;
    bool clear_meb;
    bool auto_reset;

    Caen820Config ()
    : baddr (0)
    , channel_enable (0xFFFFFFFFU)
    , acq_mode (1)
    , dwell_time (3)
    , short_data_format (false)
    , berr_enable (true)
    , hdr_enable (false)
    , clear_meb (false)
    , auto_reset (false)
    {}
};

class DemuxCaen820Plugin;
class Caen820UI;

class Caen820Module : public BaseModule {
    Q_OBJECT
public:
    static AbstractModule *create (int id, const QString& name);

    void setChannels ();
    int acquire (Event *ev);
    bool dataReady ();
    int reset ();
    int configure ();
    void setBaseAddress (uint32_t baddr);
    uint32_t getBaseAddress () const;
    int dataClear ();

    QVector<uint32_t> acquireMonitor ();

    void applySettings (QSettings *);
    void saveSettings (QSettings *);

    void runStartingEvent() { dmx_.runStartingEvent(); }

private:
    Caen820Module (int id, const QString &name);

    int getNofActiveChannels ();

private:
    Caen820Config conf_;
    Caen820Demux dmx_;

    QVector<EventSlot*> evslots_;

    friend class Caen820UI;
};

#endif // CAEN820SCALER_H
