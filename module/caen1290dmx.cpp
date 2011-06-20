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

#include "caen1290dmx.h"
#include "eventbuffer.h"

#include <cstdio>
#include <iostream>

Caen1290Demux::Caen1290Demux (const QVector<EventSlot*> &evslots, int channels, bool hires)
: status_ (Start)
, chans_ (channels)
, measurementBits_ (hires ? 21 : 19)
, channelBits_ (hires ? 5 : 7)
, evslots_ (evslots)
{
    evbuf_.resize (chans_);
}

bool Caen1290Demux::processData (Event *ev, const std::vector<uint32_t> &data, bool singleev) {
    for (std::vector<uint32_t>::const_iterator i = data.begin (); i != data.end (); ++i) {
        uint32_t tag = (*i >> 27) & 0x1F;

        switch (tag) {
        case GlobalHeader:
            if (status_ != Start) {
                printf ("DemuxCaen1290: Header while event processing\n");
                endEvent (ev);
            }
            startEvent (*i);
            break;
        case GlobalTrailer:
            if (status_ != Measurements) {
                std::cout << "DemuxCaen1290: Trailer while not processing" << std::endl;
            } else {
                bool go_on = endEvent (ev);
                if (*i & (1 << 25))
                    std::cout << "MHTDC Overflow!" << std::endl;

                if (singleev || ! go_on)
                    return false;
            }
            break;
        case TDCMeasurement:
            if (status_ != Measurements)
                std::cout << "DemuxCaen1290: Data outside of event. Ignoring" << std::endl;
            else
                processEvent (*i);
            break;
        case TDCHeader:
        case TDCTrailer:
        case Filler:
        case TrigTimeTag:
            break;
        case TDCError:
            printf ("DemuxCaen1290: Got TDC error %#04x from TDC %d\n", (*i & 0x3FFF), (*i >> 24) & 0x3);
            break;
        default:
            printf ("DemuxCaen1290: Unknown tag: %#02x\n", tag);
        }
    }
    return true;
}

void Caen1290Demux::startEvent (uint32_t info) {
    Q_UNUSED (info);
    status_ = Measurements;
    for (int i = 0; i < chans_; ++i)
        evbuf_ [i].clear ();
}

void Caen1290Demux::processEvent (uint32_t ev) {
    uint16_t channel = (ev >> measurementBits_) & ((1 << channelBits_) - 1);
    uint32_t value = ev & ((1 << measurementBits_) - 1);

    evbuf_ [channel].push_back (value);
}

bool Caen1290Demux::endEvent (Event *ev) {
    for (int i = 0; i < chans_; ++i) {
        ev->put (evslots_.at (i), QVariant::fromValue (evbuf_ [i]));
    }
    status_ = Start;

    return true;
}
