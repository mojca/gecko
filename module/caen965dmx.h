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

/*
  This plugin will demux the data from any CAEN v965 Dual Range QDC
  The usual channel count of this module is 16.
  The output is still send to 32 channels. 16 for the low and 16 for the
  high range.
*/

#ifndef DEMUXCAEN965PLUGIN_H
#define DEMUXCAEN965PLUGIN_H

#include <algorithm>
#include <map>
#include <cstdio>
#include <stdint.h>
#include "caen_v965.h"

#include <QVector>

class Event;
class EventSlot;
class AbstractModule;
template <typename T> class QVector;

#define CAEN965_EVENT_LENGTH 34

class Caen965Demux
{
private:
    bool inEvent;
    int cnt;

    uint8_t nofChannels;
    uint8_t nofChannelsInEvent;
    uint8_t nofBits;
    uint8_t crateNumber;
    uint32_t eventCounter;
    uint8_t id;
    std::map<uint8_t, uint16_t> chData;
    QVector<uint32_t> rawData;
    uint32_t rawBuffer[CAEN965_EVENT_LENGTH];
    uint32_t* rawBufferPtr;
    uint32_t rawCnt;
    const QVector<EventSlot*>& evslots;
    const AbstractModule *owner;

    uint32_t* it;

    void startNewEvent();
    void continueEvent();
    bool finishEvent(Event *ev);
    void printHeader();
    void printEob();

public:
    Caen965Demux(const QVector<EventSlot*>& _evslots, const AbstractModule* op, uint chans = 16, uint bits = 12);

    bool processData (Event *ev, uint32_t* data, uint32_t len, bool singleev);
};

#endif // DEMUXCAEN965PLUGIN_H
