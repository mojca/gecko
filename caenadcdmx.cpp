#include "demuxcaenadcplugin.h"
#include "pluginmanager.h"
#include "pluginconnectorthreadbuffered.h"

#include <QGridLayout>
#include <QLabel>

static PluginRegistrar registrar ("demuxcaenadc", AbstractPlugin::GroupDemux);

DemuxCaenADCPlugin::DemuxCaenADCPlugin(int _id, QString _name, const Attributes &attrs)
    : BasePlugin(_id, _name)
    , scheduleReset(false)
    , nofChannelsInEvent(0)
{
    inEvent = false;
    cnt = 0;

    bool ok;
    nofChannels = attrs.value ("nofChannels", QVariant (32)).toInt (&ok);
    if (!ok || nofChannels <= 0) {
        nofChannels = 32;
        std::cout << _name.toStdString () << ": nofChannels invalid. Setting to 32" << std::endl;
    }

    nofBits = attrs.value ("nofBits", QVariant (12)).toInt (&ok);
    if (!ok || nofBits <= 0) {
        nofBits = 12;
        std::cout << _name.toStdString () << ": nofBits invalid. Setting to 12" << std::endl;
    }

    createSettings(settingsLayout);

    for(int i = 0; i < nofChannels; i++)
    {
        addConnector(new PluginConnectorThreadBuffered(this,QString("out %1").arg(i),
                                                       1,5,(getId () << 16) | i));
    }

    std::cout << "Instantiated DemuxCaenADCPlugin" << std::endl;
}

AbstractPlugin::AttributeMap DemuxCaenADCPlugin::getAttributeList () const {
    AbstractPlugin::AttributeMap attrs;
    attrs.insert ("nofChannels", QVariant::Int);
    attrs.insert ("nofBits", QVariant::Int);
    return attrs;
}

void DemuxCaenADCPlugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        QLabel* label = new QLabel(tr("This plugin decodes the raw data to individual spectra."));
        resetButton = new QPushButton(tr("Reset spectra"));
        connect(resetButton,SIGNAL(clicked()),this,SLOT(resetSpectra()));

        cl->addWidget(label);
        cl->addWidget(resetButton);
        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

bool DemuxCaenADCPlugin::processData (uint32_t *data, uint32_t len, bool singleev)
{
    if (scheduleReset) {
        scheduleReset = false;
    }

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
                bool go_on = finishEvent();

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

void DemuxCaenADCPlugin::startNewEvent()
{
    //    std::cout << "DemuxCaenADCPlugin: Start" << std::endl;

    nofChannelsInEvent = 0x0 | (((*it) >>  8) & 0x1f);
    crateNumber = 0x0 | (((*it) >> 16) & 0xff);

    cnt = 0;
    inEvent = true;
    chData.clear ();

    //printHeader();
}

void DemuxCaenADCPlugin::continueEvent()
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

bool DemuxCaenADCPlugin::finishEvent()
{
    eventCounter = 0x0 | (((*it) >> 0)  & 0xffffff);
    //printEob();
    inEvent = false;

    bool isfirst = true;

    for (std::map<uint8_t,uint16_t>::const_iterator i = chData.begin (); i != chData.end (); ++i) {
        // Publish event data
        if(outputs->at(i->first)->hasOtherSide())
        {
            PluginConnectorThreadBuffered* bpc = dynamic_cast<PluginConnectorThreadBuffered*>(outputs->at(i->first));
            if (isfirst) {
                isfirst = false;
                if (!bpc->elementsFree ())
                    return false;
            }
            bpc->setData(new std::vector<uint32_t>(1,i->second));
            //std::cout << std::dec << "Adding to bin " << adcValue[ch] << " in spectrum " << (int)ch << std::endl;
        }
    }
    return true;
}

void DemuxCaenADCPlugin::printHeader()
{
    printf("nof channels %u, crate %u, id %u\n",nofChannelsInEvent,crateNumber,id);fflush(stdout);
}

void DemuxCaenADCPlugin::printEob()
{
    printf("event counter %u, channels %d, id %u\n",eventCounter,cnt,id);fflush(stdout);
}

void DemuxCaenADCPlugin::resetSpectra()
{
    scheduleReset = true;
}
