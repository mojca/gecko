#include "caenadcdmx.h"
#include "eventbuffer.h"
#include "abstractmodule.h"
#include "outputplugin.h"
#include <iostream>

CaenADCDemux::CaenADCDemux(const QVector<EventSlot*>& _evslots, const AbstractModule* own, uint chans, uint bits)
    : inEvent (false)
    , cnt (0)
    , nofChannels (chans)
    , nofChannelsInEvent(0)
    , nofBits (bits)
    , evslots (_evslots)
    , owner (own)
{
    if (nofChannels == 0) {
        nofChannels = 32;
        std::cout << "CaenADCDemux: nofChannels invalid. Setting to 32" << std::endl;
    }

    if (nofBits == 0) {
        nofBits = 12;
        std::cout << "CaenADCDemux: nofBits invalid. Setting to 12" << std::endl;
    }

    std::cout << "Instantiated CaenADCDemux" << std::endl;
}

bool CaenADCDemux::processData (Event* ev, uint32_t *data, uint32_t len, bool singleev)
{
    //std::cout << "DemuxCaenADCPlugin Processing" << std::endl;
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

void CaenADCDemux::startNewEvent()
{
    //    std::cout << "DemuxCaenADCPlugin: Start" << std::endl;

    nofChannelsInEvent = 0x0 | (((*it) >>  8) & 0x1f);
    crateNumber = 0x0 | (((*it) >> 16) & 0xff);

    cnt = 0;
    inEvent = true;
    chData.clear ();

    //printHeader();
}

void CaenADCDemux::continueEvent()
{
    uint8_t ch     = (((*it) >> 16) & 0x1f );
    uint16_t val   = (((*it) >>  0) & 0xfff);
    bool overRange = (((*it) >> 12) & 0x1  ) != 0;
    //bool underThr  = (((*it) >> 13) & 0x1  ) != 0;

    if(ch < nofChannels)
    {
        if(val < (1 << nofBits) && !overRange)
        {
            chData.insert (std::make_pair (ch, val));
        }
    } else {
        std::cout << "DemuxCaenADC: got invalid channel number " << std::dec << (int)ch << std::endl;
    }

    cnt++;
}

bool CaenADCDemux::finishEvent(Event *ev)
{
    eventCounter = 0x0 | (((*it) >> 0)  & 0xffffff);
    //printEob();
    inEvent = false;

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

void CaenADCDemux::printHeader()
{
    printf("nof channels %u, crate %u, id %u\n",nofChannelsInEvent,crateNumber,id);fflush(stdout);
}

void CaenADCDemux::printEob()
{
    printf("event counter %u, channels %d, id %u\n",eventCounter,cnt,id);fflush(stdout);
}
