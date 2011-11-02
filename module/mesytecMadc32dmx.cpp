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

#include "mesytecMadc32dmx.h"
#include "eventbuffer.h"
#include "abstractmodule.h"
#include "outputplugin.h"
#include <iostream>

MesytecMadc32Demux::MesytecMadc32Demux(const QVector<EventSlot*>& _evslots,
                           const AbstractModule* own,
                           uint chans, uint bits)
    : inEvent (false)
    , cnt (0)
    , enable_raw_output(false)
    , enable_per_channel_output(false)
    , nofChannels (chans)
    , nofChannelsInEvent(0)
    , nofBits (bits)
    , evslots (_evslots)
    , owner (own)
{
    if (nofChannels == 0) {
        nofChannels = MADC32V2_NUM_CHANNELS;
        std::cout << "MesytecMadc32Demux: nofChannels invalid. Setting to 32" << std::endl;
    }

    if (nofBits == 0) {
        nofBits = MADC32V2_NUM_BITS;
        std::cout << "MesytecMadc32Demux: nofBits invalid. Setting to 12" << std::endl;
    }

    rawData.resize(MADC32V2_LEN_EVENT_MAX);
    enable_ch.resize(MADC32V2_NUM_CHANNELS);

    std::cout << "Instantiated MesytecMadc32Demux" << std::endl;
}

void MesytecMadc32Demux::runStartingEvent() {
    if (owner->getOutputPlugin ()->isSlotConnected (evslots.last())) {
        enable_raw_output = true;
    } else {
        enable_raw_output = false;
    }

    int cnt = 0;
    for(int ch = 0; ch < MADC32V2_NUM_CHANNELS; ++ch) {
        if (owner->getOutputPlugin ()->isSlotConnected (evslots.at(ch))) {
            enable_per_channel_output = true;
            enable_ch[ch] = true;
            cnt++;
        } else {
            enable_ch[ch] = false;
        }
    }
    if(cnt == 0) enable_per_channel_output = false;

    printf("MesytecMadc32Demux::runStartingEvent: enable_raw_output %d\n",enable_raw_output);
    printf("MesytecMadc32Demux::runStartingEvent: enable_per_channel_output %d\n",enable_per_channel_output);
}

bool MesytecMadc32Demux::processData (Event* ev, uint32_t *data, uint32_t len, bool singleev)
{
    //std::cout << "DemuxMesytecMadc32Plugin Processing" << std::endl;
    it = data;

    while(it != (data+len))
    {
        //printf("0x%08x\n",*it);

        id = 0x0 | (((*it) >> MADC32V2_OFF_DATA_SIG) & MADC32V2_MSK_DATA_SIG);

        if(id == MADC32V2_SIG_HEADER)
        {
            if(!inEvent) startNewEvent();
            else {
                std::cout << "Already in event!" << std::endl;
            }
        }
        else if(id == MADC32V2_SIG_DATA)
        {
            if(inEvent) continueEvent();
            else std::cout << "Not in event!" << std::endl;
        }
        else if(id == MADC32V2_SIG_END || id == MADC32V2_SIG_END_BERR)
        {
            if(inEvent) {
                bool go_on = finishEvent(ev);

                if (singleev || ! go_on)
                    return false;
            }
            else std::cout << "Not in event!" << std::endl;
        }
        else
        {
            std::cout << "Unknown data word " << std::hex << (*it) << std::endl;
        }

        it++;
    }
    return true;
}

void MesytecMadc32Demux::startNewEvent()
{
    //std::cout << "DemuxMesytecMadc32Plugin: Start" << std::endl;
    inEvent = true;

    header.data = (*it);

    if(enable_per_channel_output) {
        cnt = 0;
        chData.clear ();
    }

    if(enable_raw_output) {
        rawData.fill(0);
        rawCnt = 0;
        rawData[rawCnt++] = (*it);
    }

    //printHeader();
}

void MesytecMadc32Demux::continueEvent()
{
    if(enable_raw_output) {
        rawData[rawCnt++] = (*it);
    }

    if(enable_per_channel_output) {
        madc32_data_t data;
        data.data = (*it);
        if(data.bits.sub_signature == MADC32V2_SIG_DATA_EVENT) {
            if(data.bits.channel < nofChannels) {
                if(data.bits.value < (1 << nofBits) && !data.bits.out_of_range) {
                    chData.insert(std::make_pair(data.bits.channel,data.bits.value));
                }
            } else {
                std::cout << "DemuxMesytecMadc32: got invalid channel number " << std::dec << (int)data.bits.channel << std::endl;
            }
        } else if (data.bits.sub_signature == MADC32V2_SIG_DATA_DUMMY) {
            // Do nothing
        } else if (data.bits.sub_signature == MADC32V2_SIG_DATA_TIME) {
            madc32_extended_timestamp_t time;
            time.data = (*it);
            std::cout << "DemuxMesytecMadc32: Found extended timestamp: "
                      << std::dec << (uint32_t) time.bits.timestamp << std::endl;
        } else {
            std::cout << "DemuxMesytecMadc32: No valid sub signature in word: "
                      << std::hex << (uint32_t) data.data << std::endl;
        }
        cnt++;
    }
}

bool MesytecMadc32Demux::finishEvent(Event *ev)
{
    inEvent = false;

    if(enable_per_channel_output) {
        trailer.data = (*it);
        eventCounter = trailer.bits.trigger_counter;
        //printEob();

        for (std::map<uint8_t,uint16_t>::const_iterator i = chData.begin (); i != chData.end (); ++i) {
            // Publish event data
            if (owner->getOutputPlugin ()->isSlotConnected (evslots.at (i->first))) {
                const EventSlot* sl = evslots.at (i->first);
                QVector<uint32_t> v = ev->get (sl).value< QVector<uint32_t> > ();
                v << i->second;
                ev->put (evslots.at (i->first), QVariant::fromValue (v));
            }
        }
    }

    if(enable_raw_output){
        rawData[rawCnt++] = (*it);
        ev->put(evslots.last(), QVariant::fromValue(rawData));
    }

    return true;
}

void MesytecMadc32Demux::printHeader()
{
    printf("nof words %u, resolution %u, module id %u, output format %u\n",
           header.bits.data_length,header.bits.adc_resolution,
           header.bits.module_id,header.bits.output_format);fflush(stdout);
}

void MesytecMadc32Demux::printEob()
{
    printf("event counter %u, channels %d, module id %u\n",eventCounter,cnt,header.bits.module_id);fflush(stdout);
}
