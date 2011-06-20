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

#ifndef DEMUXCAEN1290PLUGIN_H
#define DEMUXCAEN1290PLUGIN_H

#include <stdint.h>
#include <vector>

template <typename T> class QVector;
class EventSlot;
class Event;

class Caen1290Demux
{
public:
    Caen1290Demux (const QVector<EventSlot*> &evslots, int channels, bool hires);

    bool processData (Event *ev, const std::vector<uint32_t> &data, bool singleev);

private:
    void startEvent (uint32_t info);
    bool endEvent (Event *ev);
    void processEvent (uint32_t ev);

private:
    enum Status {Start, Measurements};
    enum Tag {GlobalHeader = 0x08, GlobalTrailer = 0x10, Filler = 0x18, TrigTimeTag = 0x11,
              TDCHeader = 0x01, TDCMeasurement = 0x00, TDCError = 0x04, TDCTrailer = 0x03};

    Status status_;
    std::vector< QVector<uint32_t> > evbuf_;
    int chans_;
    int measurementBits_;
    int channelBits_;
    const QVector<EventSlot*> &evslots_;
};

#endif // DEMUXCAEN1290PLUGIN_H
