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

#ifndef DEMUXSIS3302PLUGIN_H
#define DEMUXSIS3302PLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include <QList>

#include "sis3302.h"

class Event;
class EventSlot;

class Sis3302Demux
{

public:
    Sis3302Demux (const QList<EventSlot*> &);

    virtual void process(Event *ev, uint32_t* _data, uint32_t _len);
    void setMetaData(uint32_t, EventDirEntry_t*, TimestampDir_t*);

protected:
    DataStruct_t* data;
    uint32_t len;
    EventDirEntry_t* evDir;
    TimestampDir_t* tsDir;

    uint32_t* curEvent[8];
    uint32_t nofTraces;
    const QList<EventSlot*> &evslots;

    bool pageWrap;
};

#endif // DEMUXSIS3302PLUGIN_H
