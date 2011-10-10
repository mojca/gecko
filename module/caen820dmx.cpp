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

#include "caen820dmx.h"
#include "eventbuffer.h"
#include "abstractmodule.h"
#include "outputplugin.h"

#include <cstdio>
#include <iostream>

Caen820Demux::Caen820Demux (const QVector<EventSlot*> &evslots, const AbstractModule* own)
: enabledch_ (0xFFFFFFFFU)
, shortfmt_ (false)
, hdrenabled_ (false)
, evslots_ (evslots)
, owner(own)
{
    rawData.resize(34);
    enable_ch.resize(32);

    std::cout << "Instantiated Caen820Demux" << std::endl;
}

void Caen820Demux::runStartingEvent() {
    if (owner->getOutputPlugin ()->isSlotConnected (evslots_.last())) {
        enable_raw_output = true;
    } else {
        enable_raw_output = false;
    }
    int cnt = 0;
    for(int ch = 0; ch < 32; ++ch) {
        if (owner->getOutputPlugin ()->isSlotConnected (evslots_.at(ch))) {
            enable_per_channel_output = true;
            enable_ch[ch] = true;
            cnt++;
        } else {
            enable_ch[ch] = false;
        }
    }
    if(cnt == 0) enable_per_channel_output = false;

    printf("Caen820Demux::runStartingEvent: enable_raw_output %d\n",enable_raw_output);
    printf("Caen820Demux::runStartingEvent: enable_per_channel_output %d\n",enable_per_channel_output);
}

bool Caen820Demux::processData (Event *ev, const uint32_t *data, int len) {
    if (hdrenabled_) { // no use for the header yet
        ++data; --len;
    }

    if(enable_raw_output) {
        rawCnt = 0;
        rawData.fill(0);
    }

    for (int i = 0; i < 32; ++i) {
        if (!((enabledch_ & (1 << i)) && len)) {
            if (!len && (enabledch_ & (1 << i))) {
                std::cout << "DemuxCaen820: Warning! not enough data to serve channel " << i << std::endl;
            }
            continue;
        }

        if(enable_per_channel_output) {
            ev->put (evslots_.at (i), QVariant::fromValue (QVector<uint32_t> () << *data));
        }
        if(enable_raw_output) {
            rawData[rawCnt++] = (*data);
        }

        ++data; --len;
    }

    if(enable_raw_output) {
        ev->put(evslots_.last(), QVariant::fromValue(rawData));
    }

    return true;
}
