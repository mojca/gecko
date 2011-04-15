#include "sis3302dmx.h"
#include "eventbuffer.h"

#include <QVector>


Sis3302Demux::Sis3302Demux(const QList<EventSlot *> &evsl)
    : evslots (evsl)
{
}

void Sis3302Demux::process (Event *ev, uint32_t *_data, uint32_t _len)
{
    //printf("DemuxSis3302Plugin processing...\n");
    data = _data;
    len = _len;

    // Recover channel information
    uint8_t curCh = (len >> 29) & 0x7;
    // Revover length
    uint32_t length = (len & 0x1ffffff);

    //printf("Current channel: %d with %d data points.\n",curCh,nofSamples);

    // Publish event data
    QVector<uint32_t> outData;

    for(uint32_t i = 0; i < length; i++)
    {
        outData.push_back (data[i] & 0xffff);
        outData.push_back ((data[i] & 0xffffffff) >> 16);
    }

    //outData->assign(length,(*data));
    ev->put (evslots.at(curCh), QVariant::fromValue (outData));

    /*printf("Data dump:\n");
    for(uint32_t i=0; i < nofSamples; i++)
    {
        printf("<%d> %u  ",i,(*outData)[i]);
    }
    printf("\n");*/
}
