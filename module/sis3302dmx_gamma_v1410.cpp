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

#include "sis3302dmx_gamma_v1410.h"
#include "eventbuffer.h"
#include "abstractmodule.h"
#include "outputplugin.h"

#include <QVector>


Sis3302V1410Demux::Sis3302V1410Demux(const QList<EventSlot *> &evsl)
    : output_raw_traces_start_idx(0),
      output_energy_traces_start_idx(SIS3302_V1410_NOF_CHANNELS),
      output_energy_value_start_idx(SIS3302_V1410_NOF_CHANNELS*2),
      output_raw_data_start_idx(SIS3302_V1410_NOF_CHANNELS*3),
      enable_raw_output(false),
      enable_per_channel_output(false),
      enable_meta_output(false),
      evslots (evsl),
      pageWrap(false)
{
    enabled_raw_sample_ch.resize(SIS3302_V1410_NOF_CHANNELS);
    enabled_energy_sample_ch.resize(SIS3302_V1410_NOF_CHANNELS);
    enabled_energy_value_ch.resize(SIS3302_V1410_NOF_CHANNELS);
}

void Sis3302V1410Demux::setMetaData(uint32_t _nofTraces, EventDirEntry_t* _evDir, TimestampDir_t *_tsDir)
{
    Q_UNUSED (_evDir)
    Q_UNUSED (_tsDir)

    nofTraces = _nofTraces;
    pageWrap = true;
}

void Sis3302V1410Demux::setMultiEvent(bool _isMultiEvent) {
    isMultiEvent = _isMultiEvent;
}

void Sis3302V1410Demux::setNofEvents(uint32_t _nofEvents) {
    nofEvents = _nofEvents;
}

void Sis3302V1410Demux::runStartingEvent(AbstractModule* owner) {

    // Reset ch enabled vectors
    enabled_raw_sample_ch.fill(false);
    enabled_energy_sample_ch.fill(false);
    enabled_energy_value_ch.fill(false);

    // Check, if meta output should be used
    if(owner->getOutputPlugin()->isSlotConnected(evslots.last())) {
        enable_meta_output = true;
    } else {
        enable_meta_output = false;
    }

    // Check, if raw data output should be used
    if(owner->getOutputPlugin()->isSlotConnected(
                evslots.at(output_raw_data_start_idx))) {
        enable_raw_output = true;
    } else {
        enable_raw_output = false;
    }

    // Check, if sampling outputs should be used (raw and energy)
    int cnt = 0;
    for(int ch = output_raw_traces_start_idx;
        ch < output_raw_traces_start_idx
        + SIS3302_V1410_NOF_CHANNELS; ++ch) {
        if(owner->getOutputPlugin()->isSlotConnected(evslots.at(ch))) {
            enable_per_channel_output = true;
            enabled_raw_sample_ch[ch-output_raw_traces_start_idx] = true;
            cnt++;
        }
    }
    for(int ch = output_energy_traces_start_idx;
        ch < output_energy_traces_start_idx
        + SIS3302_V1410_NOF_CHANNELS; ++ch) {
        if(owner->getOutputPlugin()->isSlotConnected(evslots.at(ch))) {
            enable_per_channel_output = true;
            enabled_energy_sample_ch[ch-output_energy_traces_start_idx] = true;
            cnt++;
        }
    }
    // Check, if energy value outputs should be used
    for(int ch = output_energy_value_start_idx;
        ch < output_energy_value_start_idx
        + SIS3302_V1410_NOF_CHANNELS; ++ch) {
        if(owner->getOutputPlugin()->isSlotConnected(evslots.at(ch))) {
            enable_per_channel_output = true;
            enabled_energy_value_ch[ch-output_energy_value_start_idx] = true;
            cnt++;
        }
    }
    if(cnt == 0) enable_per_channel_output = false;


    printf("Sis3302V1410Demux::runStartingEvent: enable_raw_output %d\n",enable_raw_output);
    printf("Sis3302V1410Demux::runStartingEvent: enable_per_channel_output %d\n",enable_per_channel_output);
    printf("Sis3302V1410Demux::runStartingEvent: enable_meta_output %d\n",enable_meta_output);

}

void Sis3302V1410Demux::processRaw(Event *ev, uint32_t _data[][SIS3302_V1410_MAX_NOF_LWORDS], uint32_t* len){

    if(enable_raw_output) {
        DataStruct_t* data_[SIS3302_V1410_NOF_CHANNELS];

        // Recover length
        uint32_t total_length = 0;

        for(int ch = 0; ch < SIS3302_V1410_NOF_CHANNELS; ++ch) {
            data_[ch] = (DataStruct_t*)_data[ch];
            total_length += (len[ch] & 0x1ffffff); // lWords
            //printf("length %d: %d\n",ch,(len[ch] & 0x1ffffff));
        }

        rawData.resize(total_length);
        //printf("total_length: %d\n",total_length);

        int cnt = 0;
        for(int ch = 0; ch < SIS3302_V1410_NOF_CHANNELS; ++ch) {
            //printf("filling ch %d raw...\n",ch);
            for(uint32_t i = 0; i < (len[ch] & 0x1ffffff); ++i) {
                //printf("rawData[%d] = data_[%d][%d].data = %08x\n",cnt,ch,i,data_[ch][i].data);
                rawData[cnt++] = data_[ch][i].data;
            }
        }

        ev->put (evslots.at(output_raw_data_start_idx),
                 QVariant::fromValue (rawData));

    }
}

void Sis3302V1410Demux::process (Event *ev, uint32_t *_data, uint32_t _len, uint32_t raw_length)
{

    if(enable_per_channel_output) {
        //printf("DemuxSis3302V1410Plugin processing...\n");
        data = (DataStruct_t*)_data;
        len = _len;

        // Recover length
        uint32_t length = (len & 0x1ffffff); // lWords
        uint32_t length_single = 0;


        // Recover channel information
        uint8_t curCh = (len >> 29) & 0x7;

        if(isMultiEvent) length_single = length / nofEvents;
        else length_single = length;

        // Compute data lengths
        uint16_t length_raw             = raw_length; // 16-bit samples
        uint16_t length_header_trailer  = SIS3302_V1410_EVENT_LEN_MIN;
        uint16_t length_energy          = length_single
                                        - length_raw/2
                                        - length_header_trailer;

        // Compute data offsets
        uint16_t rawOffset          = 2;
        uint16_t energyOffset       = rawOffset + (length_raw/2);
        uint16_t energyValueOffset  = energyOffset + length_energy;

        //printf("sis3302dmx: Current channel: %d with %d lwords of data.\n",curCh,length);
        //printf("sis3302dmx: MultiEvent: %d, nofEvents: %d with single length: %d\n",isMultiEvent,nofEvents,length_single);
        //printf("sis3302dmx: raw: %d samples, energy: %d samples.\n",length_raw,length_energy);
        //printf("sis3302dmx: raw: %d offset, energy: %d offset.\n",rawOffset,energyOffset);

        // Event data containers
        if(enabled_raw_sample_ch[curCh]) {
            outData.resize(length_raw*nofEvents);
        }
        if(enabled_energy_sample_ch[curCh]) {
            outData2.resize(length_energy*nofEvents);
        }
        if(enabled_energy_value_ch[curCh]) {
            outData3.resize(nofEvents);
        }

        int rawcnt = 0;
        int nrgcnt = 0;

        for(uint32_t n = 0; n < nofEvents; ++n) {
            uint32_t event_offset = length_single*n;

            //  RAW trace
            if(enabled_raw_sample_ch[curCh]) {
                for(uint32_t i = 0; i < length_raw/2; i++) {
                    outData[rawcnt++] = data[event_offset + rawOffset + i].low;
                    outData[rawcnt++] = data[event_offset + rawOffset + i].high;
                }
            }

            //  Energy trace
            if(enabled_energy_sample_ch[curCh]) {
                for(uint32_t i = 0; i < length_energy; i++) {
                    outData2[nrgcnt++] = data[event_offset + energyOffset + i].data;
                }
            }

            //  Energy value
            if(enabled_energy_value_ch[curCh]) {
                double energyValue = - (int32_t)(data[event_offset + energyValueOffset + 1].data)
                                     + (int32_t)(data[event_offset + energyValueOffset].data);
                outData3[n] = energyValue;
                //printf("sis3302dmx: energy value[%d]: %d offset, %f value: \n",n,energyValueOffset,energyValue);
            }
        }

        // Publish event data
        if(enabled_raw_sample_ch[curCh]) {
            ev->put (evslots.at(curCh),
                     QVariant::fromValue (outData));
        }
        if(enabled_energy_sample_ch[curCh]) {
            ev->put (evslots.at(curCh+SIS3302_V1410_NOF_CHANNELS),
                     QVariant::fromValue (outData2));
        }
        if(enabled_energy_value_ch[curCh]) {
            ev->put (evslots.at(curCh+SIS3302_V1410_NOF_CHANNELS*2),
                     QVariant::fromValue (outData3));
        }

        /*printf("Data dump from DMX:\n");
        for(uint32_t i=0; i < length*2; i++)
        {
            printf("<%d> %u  ",i,outData[i]);
        }
        printf("\n");*/
    }
}
