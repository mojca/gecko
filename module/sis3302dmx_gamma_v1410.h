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

#ifndef DEMUXSIS3302V1410PLUGIN_H
#define DEMUXSIS3302V1410PLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include <QList>
#include <QVector>

#include "sis3302_gamma_v1410.h"

class Event;
class EventSlot;
class AbstractModule;

#define SIS3302_V1410_NOF_CHANNELS 8
#define SIS3302_V1410_EVENT_LEN_MIN 6

class Sis3302V1410Demux
{

public:
    Sis3302V1410Demux (const QList<EventSlot*> &);

    virtual void process(Event *ev, uint32_t* _data, uint32_t _len);
    void processRaw(Event *ev, uint32_t _data[][SIS3302_V1410_MAX_NOF_LWORDS], uint32_t* _len);
    void setMetaData(uint32_t, EventDirEntry_t*, TimestampDir_t*);

    void setMultiEvent(bool _isMultiEvent);
    void setNofEvents(uint32_t _nofEvents);

    void runStartingEvent(AbstractModule* owner);

protected:
    DataStruct_t* data;
    uint32_t len;

    uint32_t* curEvent[SIS3302_V1410_NOF_CHANNELS];
    uint32_t nofTraces;
    uint32_t nofEvents;

    int output_raw_traces_start_idx;
    int output_energy_traces_start_idx;
    int output_energy_value_start_idx;
    int output_raw_data_start_idx;

    bool isMultiEvent;
    bool enable_raw_output;
    bool enable_per_channel_output;
    bool enable_meta_output;

    QVector<uint32_t> outData;
    QVector<uint32_t> outData2;
    QVector<double> outData3;

    QVector<uint32_t> rawData;
    uint32_t rawCnt;

    QVector<bool> enabled_raw_sample_ch;
    QVector<bool> enabled_energy_sample_ch;
    QVector<bool> enabled_energy_value_ch;

    const QList<EventSlot*> &evslots;

    bool pageWrap;
};

#endif // DEMUXSIS3302V1410PLUGIN_H

