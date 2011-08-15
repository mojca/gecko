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

#ifndef DEMUXCAENADCPLUGIN_H
#define DEMUXCAENADCPLUGIN_H

#include <algorithm>
#include <map>
#include <cstdio>
#include <stdint.h>
#include "caen_v785.h"

class Event;
class EventSlot;
class AbstractModule;
template <typename T> class QVector;

class CaenADCDemux
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
    const QVector<EventSlot*>& evslots;
    const AbstractModule *owner;

    uint32_t* it;

    void startNewEvent();
    void continueEvent();
    bool finishEvent(Event *ev);
    void printHeader();
    void printEob();

public:
    CaenADCDemux(const QVector<EventSlot*>& _evslots, const AbstractModule* op, uint chans = 32, uint bits = 12);

    bool processData (Event *ev, uint32_t* data, uint32_t len, bool singleev);
};

#endif // DEMUXCAENADCPLUGIN_H
