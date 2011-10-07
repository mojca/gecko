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

#include "caen965dmx.h"
#include "eventbuffer.h"
#include "abstractmodule.h"
#include "outputplugin.h"
#include <iostream>

Caen965Demux::Caen965Demux(const QVector<EventSlot*>& _evslots, const AbstractModule* own, uint chans, uint bits)
    : inEvent (false)
    , cnt (0)
    , nofChannels (chans)
    , nofChannelsInEvent(0)
    , nofBits (bits)
    , evslots (_evslots)
    , owner (own)
{
    if (nofChannels == 0) {
        nofChannels = 16;
        std::cout << "Caen965Demux: nofChannels invalid. Setting to 16" << std::endl;
    }

    if (nofBits == 0) {
        nofBits = 12;
        std::cout << "Caen965Demux: nofBits invalid. Setting to 12" << std::endl;
    }

	nofChannels *= 2; // Need twice the amount of channels to store the data (high and low range).

    std::cout << "Instantiated Caen965Demux" << std::endl;
}

bool Caen965Demux::processData (Event* ev, uint32_t *data, uint32_t len, bool singleev)
{
    //std::cout << "DemuxCaen965Plugin Processing" << std::endl;
    it = data;

    while(it != (data+len))
    {
        id = 0x0 | (((*it) >> 24) & 0x7 );

        if(id == 0x2)
        {
            if(!inEvent) startNewEvent();
            else std::cout << "Already in event!" << std::endl;
        }
        else if(id == 0x0)
        {
            if(inEvent) continueEvent();
            else std::cout << "Not in event!" << std::endl;
        }
        else if(id == 0x4)
        {
            if(inEvent) {
                bool go_on = finishEvent(ev);

                if (singleev || ! go_on)
                    return false;
            }
            else std::cout << "Not in event!" << std::endl;
        }
        else if(id == 0x6)
        {
            // invalid data
            std::cout << "Invalid data word " << std::hex << (*it) << std::endl;
        }
        else
        {
            std::cout << "Unknown data word " << std::hex << (*it) << std::endl;
        }

        it++;
    }
    return true;
}

void Caen965Demux::startNewEvent()
{
    //    std::cout << "DemuxCaen965Plugin: Start" << std::endl;

    nofChannelsInEvent = 0x0 | (((*it) >>  8) & 0x1f);
    crateNumber = 0x0 | (((*it) >> 16) & 0xff);

    cnt = 0;
    inEvent = true;
    chData.clear ();

    //printHeader();
}

void Caen965Demux::continueEvent()
{
    uint16_t val    = (((*it) >>  0) & 0xfff);
    bool isLowRange = (((*it) >> 16) & 0x1  );
    bool overRange  = (((*it) >> 12) & 0x1  ) != 0;
    uint8_t ch      = (((*it) >> 17) & 0xf  );
    bool underThr   = (((*it) >> 13) & 0x1  ) != 0;

    if(ch < 16) {
	// store high range values in the upper half of channels
        if(isLowRange == 0) {
            ch += 16;
        }
        if(val < (1 << nofBits)) {
            chData.insert (std::make_pair (ch, val));
            //printf("ch <%d> : low range: %d, value = %d, over: %d, under: %d (0x%08x)\n",ch,(int)isLowRange,val,(int)overRange,(int)underThr);
        }
    } else {
        std::cout << "DemuxCaen965: got invalid channel number " << std::dec << (int)ch << std::endl;
    }

    cnt++;
}

bool Caen965Demux::finishEvent(Event *ev)
{

    eventCounter = 0x0 | (((*it) >> 0)  & 0xffffff);
    //printEob();
    inEvent = false;

    /*for(int i=0; i<nofChannels; ++i) {
	// Publish event data
	uint8_t ch = chData.at(i)->first;
	uint16_t val = chData.at(i)->second;
	if (owner->getOutputPlugin()->isSlotConnected(evslots.at(ch))) {
		const EventSlot* sl = evslots.at(ch);
		QVector<uint32_t> v = ev->get(sl).value<QVector_uint32_t> >();
		v << val;
		ev->put(evslots.at(ch), QVariant::fromValue(v));
	}
    }*/

    for (std::map<uint8_t,uint16_t>::const_iterator i = chData.begin (); i != chData.end (); ++i) {
        // Publish event data
        if (owner->getOutputPlugin ()->isSlotConnected (evslots.at (i->first))) {
            const EventSlot* sl = evslots.at (i->first);
            QVector<uint32_t> v = ev->get (sl).value< QVector<uint32_t> > ();
            v << i->second;
            ev->put (evslots.at (i->first), QVariant::fromValue (v));
        }
    }

    return true;
}

void Caen965Demux::printHeader()
{
    printf("nof channels %u, crate %u, id %u\n",nofChannelsInEvent,crateNumber,id);fflush(stdout);
}

void Caen965Demux::printEob()
{
    printf("event counter %u, channels %d, id %u\n",eventCounter,cnt,id);fflush(stdout);
}
