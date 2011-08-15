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

#ifndef DEMUXSIS3350PLUGIN_H
#define DEMUXSIS3350PLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include <QVector>

class EventSlot;
class Event;
class AbstractModule;

struct Sis3350Event
{
    uint8_t channel;
    uint64_t timeStamp;
    uint32_t sampleLen;
    uint8_t triggerCount;
    uint8_t stopDelay;
    uint8_t extraHeader;
    bool wrap;
    std::vector<uint32_t> data;
};

class Sis3350Demux
{
private:
    bool inHeader, inTrace;
    int cnt;

    struct Sis3350Event* curEvent[4];
    struct Sis3350Event* outEvent[4];
    int curChannel;

    uint32_t* it;
    uint32_t* data;
    uint32_t len;

    const QVector<EventSlot*> &evslots;
    const AbstractModule *owner_;
    Event *ev;

    void startNewHeader();
    void startNewTrace();
    void continueHeader();
    void continueTrace();
    void printHeader();

public:
    Sis3350Demux (const QVector<EventSlot*> &_evslots, const AbstractModule* own);

    void process(Event *_ev, uint32_t *_data, uint32_t _len);
};

#endif // DEMUXSIS3350PLUGIN_H
