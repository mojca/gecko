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

#ifndef DEMUXCAEN820PLUGIN_H
#define DEMUXCAEN820PLUGIN_H

#include <stdint.h>
#include <QVector>

class Event;
class EventSlot;

class Caen820Demux
{
public:
    Caen820Demux (const QVector<EventSlot*> &);

    bool processData (Event *ev, const uint32_t *data, int len);
    void setChannelBitmap (uint32_t bmp) { enabledch_ = bmp; }
    void setHeaderEnabled (bool en) { hdrenabled_ = en; }
    void setShortDataFmt (bool isshort) { shortfmt_ = isshort; }

private:
    uint32_t enabledch_;
    bool shortfmt_;
    bool hdrenabled_;
    const QVector<EventSlot*> &evslots_;
};

#endif // DEMUXCAEN820PLUGIN_H
