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

#include "sis3302dmx.h"
#include "eventbuffer.h"

#include <QVector>


Sis3302Demux::Sis3302Demux(const QList<EventSlot *> &evsl)
    : evslots (evsl), pageWrap(false)
{
}

void Sis3302Demux::setMetaData(uint32_t _nofTraces, EventDirEntry_t* _evDir, TimestampDir_t *_tsDir)
{
    nofTraces = _nofTraces;
    evDir = _evDir;
    tsDir = _tsDir;
    pageWrap = true;
}

void Sis3302Demux::process (Event *ev, uint32_t *_data, uint32_t _len)
{
    //printf("DemuxSis3302Plugin processing...\n");
    data = (DataStruct_t*)_data;
    len = _len;

    // Recover channel information
    uint8_t curCh = (len >> 29) & 0x7;
    // Revover length
    uint32_t length = (len & 0x1ffffff); // lWords

    //printf("sis3302dmx: Current channel: %d with %d data points.\n",curCh,length*2);

    // In case of page wrap mode, untangle data
    if(pageWrap == true)
    {
        uint32_t traceLength = length/nofTraces; // lwords
        for(unsigned int tr = 0; tr < nofTraces; tr++)
        {
            DataStruct_t tmp[traceLength];
            uint32_t off = evDir[tr].addr/2 - tr*traceLength; // Offset wrt to page border in Lwords

            // Fill temporary vector
            for(unsigned int s = off; s < traceLength; s++)
            {
                tmp[s-off] = data[tr*traceLength+s];
            }
            for(unsigned int s = 0; s < off; s++)
            {
                tmp[traceLength-off+s] = data[tr*traceLength+s];
            }

            // Fill rearranged data vector
            for(unsigned int s = 0; s < traceLength; s++)
            {
                data[tr*traceLength+s] = tmp[s];
            }
        }
    }

    // Publish event data
    QVector<uint32_t> outData(length*2,0);
    int cnt = 0;
    for(uint32_t i = 0; i < length; i++)
    {
        outData[cnt++] = data[i].low;
        outData[cnt++] = data[i].high;
    }
    ev->put (evslots.at(curCh), QVariant::fromValue (outData));

    /*printf("Data dump from DMX:\n");
    for(uint32_t i=0; i < length*2; i++)
    {
        printf("<%d> %u  ",i,outData[i]);
    }
    printf("\n");*/
}
