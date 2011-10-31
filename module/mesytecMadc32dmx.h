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

#ifndef DEMUXMESYTECMADC32PLUGIN_H
#define DEMUXMESYTECMADC32PLUGIN_H

#include <algorithm>
#include <map>
#include <cstdio>
#include <stdint.h>
#include "mesytec_madc_32_v2.h"

#include <QVector>

class Event;
class EventSlot;
class AbstractModule;
template <typename T> class QVector;

class MesytecMadc32Demux
{
private:
    bool inEvent;
    int cnt;

    bool enable_raw_output;
    bool enable_per_channel_output;

    uint8_t nofChannels;
    uint8_t nofChannelsInEvent;
    uint8_t nofBits;
    uint8_t crateNumber;
    uint32_t eventCounter;
    uint8_t id;

    std::map<uint8_t, uint16_t> chData;
    QVector<uint32_t> rawData;
    uint32_t rawCnt;
    QVector<bool> enable_ch;

    const QVector<EventSlot*>& evslots;
    const AbstractModule *owner;

    uint32_t* it;

    void startNewEvent();
    void continueEvent();
    bool finishEvent(Event *ev);
    void printHeader();
    void printEob();

    madc32_header_t header;
    madc32_end_of_event_t trailer;

public:
    MesytecMadc32Demux(const QVector<EventSlot*>& _evslots, const AbstractModule* op,
                 uint chans = MADC32V2_NUM_CHANNELS,
                 uint bits = MADC32V2_NUM_BITS);

    bool processData (Event *ev, uint32_t* data, uint32_t len, bool singleev);
    void runStartingEvent();
};

#endif // DEMUXMESYTECMADC32PLUGIN_H
