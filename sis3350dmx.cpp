#include "sis3350dmx.h"
#include "runmanager.h"
#include "eventbuffer.h"
#include <iostream>

Sis3350Demux::Sis3350Demux (const QVector<EventSlot *> &_evslots)
    : evslots (_evslots)
{
    inHeader = false;
    inTrace = false;
    cnt = 0;

    for (int i = 0; i < 4; ++i)
        curEvent [i] = new Sis3350Event;
    //std::cout << "Instantiated Sis3350Demux" << std::endl;
}

void Sis3350Demux::process (Event *_ev, uint32_t *_data, uint32_t _len)
{
    //std::cout << "Sis3350Demux Processing" << std::endl;
    data = _data;
    len = _len;
    ev = _ev;
    it = data;

    while(it != (data+len))
    {
        if(!inHeader)
        {
            if(!inTrace)
            {
                startNewHeader();
            }
            else
            {
                continueTrace();
            }
        }
        else if(!inTrace)
        {
            continueHeader();
        }
        else
        {
            startNewTrace();
        }
    }
    //usleep(10000); // FIXME
}

void Sis3350Demux::startNewHeader()
{
    //std::cout << "Sis3350Demux: Header start" << std::endl;
    cnt = 0;
    inHeader = true;
}

void Sis3350Demux::startNewTrace()
{
    uint32_t chMask = 0x0000000F;

    //std::cout << "Sis3350Demux: Trace start" << std::endl;
    cnt = 0;
    inHeader = false;
    curEvent[curChannel]->data.resize(curEvent[curChannel]->sampleLen);
    printHeader();

    // Publish meta info
    if(curChannel == 3)
    {
        int len = curEvent[curChannel]->sampleLen + 8;
        QVector<uint32_t> metainfo (8,0); // Standard container for meta info
        metainfo [0] = 0xBBBB3000;   // base address
        metainfo [1] = len;
        metainfo [2] = curEvent[curChannel]->timeStamp >> 32;
        metainfo [3] = curEvent[curChannel]->timeStamp & 0xFFFFFFFF;
        metainfo [4] = curEvent[curChannel]->sampleLen;
        metainfo [5] = chMask;
        metainfo [6] = 0x0;          // Module count
        metainfo [7] = 0xFFFFFFFF;   // unused

        ev->put (evslots.at (4), QVariant::fromValue (metainfo));
    }
}

void Sis3350Demux::continueTrace()
{
    //std::cout << "Sis3350Demux: Trace continue" << std::endl;
    while(cnt < (int)curEvent[curChannel]->sampleLen && it != (data+len))
    {
        curEvent[curChannel]->data[cnt] = ((0x00000fff & (*it)));
        cnt++;
        curEvent[curChannel]->data[cnt] = ((0x0fff0000 & (*it))) >> 16;
        cnt++;
        it++;
    }
    if(cnt == (int)curEvent[curChannel]->sampleLen)
    {
        //outEvent[curChannel] = curEvent[curChannel];

        inTrace = false;

        //std::cout << "Sis3350Demux: Trace end" << std::endl;

        // Publish event data
        QVector<uint32_t> outData (QVector<uint32_t>::fromStdVector (curEvent[curChannel]->data));
        ev->put (evslots.at (curChannel), QVariant::fromValue (outData));
    }
    else
    {
        //std::cout << "Sis3350Demux: Block end" << std::endl;
    }
}

void Sis3350Demux::continueHeader()
{
    //std::cout << "Sis3350Demux: Header continue" << std::endl;
    while(cnt < 4 && it != (data+len))
    {
        switch(cnt)
        {
        case 0:
            curChannel = ((0x30000000 & (*it)) >> 28);
            curEvent[curChannel]->channel    = curChannel;
            curEvent[curChannel]->timeStamp  = 0x0ULL | ((0x0fff0000ULL & (*it)) << 20);
            curEvent[curChannel]->timeStamp |= ((0x00000fffULL & (*it)) << 24);
            break;
        case 1:
            curEvent[curChannel]->timeStamp |= ((0x0fff0000ULL & (*it)) >> 4);
            curEvent[curChannel]->timeStamp |= ((0x00000fffULL & (*it)));
            break;
        case 2:
            curEvent[curChannel]->wrap         = ((0x08000000 & (*it)) >> 27);  // 47-36+16
            curEvent[curChannel]->stopDelay    = ((0x03000000 & (*it)) >> 24);  // 44-36+16
            curEvent[curChannel]->triggerCount = ((0x00f00000 & (*it)) >> 20);  // 40-36+16
            curEvent[curChannel]->extraHeader  = ((0x000f0000 & (*it)) >> 16);  // 36-36+16
            curEvent[curChannel]->sampleLen    = 0x0 | ((0x00000fff & (*it)) << 24);
            break;
        case 3:
            curEvent[curChannel]->sampleLen |= ((0x0fff0000 & (*it)) >> 4);
            curEvent[curChannel]->sampleLen |= ((0x00000fff & (*it)));
            inTrace = true;
            break;
        default:
            std::cout << "Sis3350Demux: Weird..." << std::endl;
            break;

        }
        cnt++;
        it++;
    }
    //std::cout << "Sis3350Demux: Header end" << std::endl;
}

void Sis3350Demux::printHeader()
{
//    printf("Time: %012llx\n",(unsigned long long)curEvent[curChannel]->timeStamp);
//    std::cout << "channel "      << std::dec << (int)curEvent[curChannel]->channel << std::endl;
//    std::cout << "timeStamp "    << std::dec << (uint64_t)curEvent[curChannel]->timeStamp << std::endl;
//    std::cout << "triggerCount " << std::dec << (int)curEvent[curChannel]->triggerCount << std::endl;
//    std::cout << "sampleLen "    << std::dec << curEvent[curChannel]->sampleLen << std::endl;
//    std::cout << "stopDelay "    << std::dec << (int)curEvent[curChannel]->stopDelay << std::endl;
//    std::cout << "extraHeader "  << std::dec << (int)curEvent[curChannel]->extraHeader << std::endl;
//    std::cout << "wrap "         << std::dec << (int)curEvent[curChannel]->wrap << std::endl;
}
