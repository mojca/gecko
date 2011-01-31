#include "demuxsis3350plugin.h"
#include "pluginmanager.h"

#include <QGridLayout>
#include <QLabel>

static PluginRegistrar registrar ("demuxsis3350", AbstractPlugin::GroupDemux);

DemuxSis3350Plugin::DemuxSis3350Plugin(int _id, QString _name)
    : BasePlugin(_id, _name)
{
    inHeader = false;
    inTrace = false;
    cnt = 0;

    createSettings(settingsLayout);

    int nofBuffers = 2;

    for(int i = 0; i < 4; i++)
    {
        int connectorId = (getId () << 16) + i;
        addConnector(new PluginConnectorThreadBuffered(this,QString("out %1").arg(i),
                                                       1,nofBuffers*1,connectorId));
        curEvent[i] = new struct Sis3350Event;
        //this->outputs->at(i)->setData(NULL);
    }
    addConnector(new PluginConnectorThreadBuffered(this,"meta info",
                                                   1,nofBuffers*1,(getId () << 16)+5));

    std::cout << "Instantiated DemuxSis3350Plugin" << std::endl;
}

void DemuxSis3350Plugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        QLabel* label = new QLabel(tr("This plugin decodes the raw data to individual channels."));

        cl->addWidget(label);
        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void DemuxSis3350Plugin::setData(uint32_t* _data, uint32_t _len)
{
    len = _len;
    data = _data;
}

void DemuxSis3350Plugin::process()
{
    //std::cout << "DemuxSis3350Plugin Processing" << std::endl;
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

void DemuxSis3350Plugin::startNewHeader()
{
    //std::cout << "DemuxSis3350Plugin: Header start" << std::endl;
    cnt = 0;
    inHeader = true;
}

void DemuxSis3350Plugin::startNewTrace()
{
    uint32_t chMask = 0x0000000F;

    //std::cout << "DemuxSis3350Plugin: Trace start" << std::endl;
    cnt = 0;
    inHeader = false;
    curEvent[curChannel]->data.resize(curEvent[curChannel]->sampleLen);
    printHeader();

    // Publish meta info
    if(curChannel == 3)
    {
        if(outputs->at(4)->hasOtherSide())
        {
            int len = curEvent[curChannel]->sampleLen + 8;
            std::vector<uint32_t>* metainfo = new std::vector<uint32_t>(8,0); // Standard container for meta info
            metainfo->at(0) = 0xBBBB3000;   // base address
            metainfo->at(1) = len;
            metainfo->at(2) = curEvent[curChannel]->timeStamp >> 32;
            metainfo->at(3) = curEvent[curChannel]->timeStamp & 0xFFFFFFFF;
            metainfo->at(4) = curEvent[curChannel]->sampleLen;
            metainfo->at(5) = chMask;
            metainfo->at(6) = 0x0;          // Module count
            metainfo->at(7) = 0xFFFFFFFF;   // unused

            PluginConnectorThreadBuffered* bpc = dynamic_cast<PluginConnectorThreadBuffered*>(outputs->at(4));
            bpc->setData(metainfo);
        }
    }
}

void DemuxSis3350Plugin::continueTrace()
{
    //std::cout << "DemuxSis3350Plugin: Trace continue" << std::endl;
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

        //std::cout << "DemuxSis3350Plugin: Trace end" << std::endl;

        // Publish event data
        if(outputs->at(curChannel)->hasOtherSide())
        {
            PluginConnectorThreadBuffered* bpc = dynamic_cast<PluginConnectorThreadBuffered*>(outputs->at(curChannel));
            std::vector<uint32_t>* outData = new std::vector<uint32_t>(curEvent[curChannel]->sampleLen,0);
            outData->assign(curEvent[curChannel]->data.begin(),curEvent[curChannel]->data.end());
            bpc->setData(outData);
        }
    }
    else
    {
        //std::cout << "DemuxSis3350Plugin: Block end" << std::endl;
    }
}

void DemuxSis3350Plugin::continueHeader()
{
    //std::cout << "DemuxSis3350Plugin: Header continue" << std::endl;
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
            std::cout << "DemuxSis3350Plugin: Weird..." << std::endl;
            break;

        }
        cnt++;
        it++;
    }
    //std::cout << "DemuxSis3350Plugin: Header end" << std::endl;
}

void DemuxSis3350Plugin::printHeader()
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
