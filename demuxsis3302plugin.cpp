#include "demuxsis3302plugin.h"
#include "pluginmanager.h"

#include <QGridLayout>
#include <QLabel>

static PluginRegistrar registrar ("demuxsis3302", AbstractPlugin::GroupDemux);

DemuxSis3302Plugin::DemuxSis3302Plugin(int _id, QString _name)
    : BasePlugin(_id, _name)
{
    createSettings(settingsLayout);

    for(int i = 0; i < 8; i++)
    {
        int connectorId = (getId () << 16) + i;
        addConnector(new PluginConnectorThreadBuffered(this,
                                            QString("out %1").arg(i),
                                            1,1,connectorId));
    }
    addConnector(new PluginConnectorThreadBuffered(this,"meta info",
                                            1,1,(getId () << 16)+5));

    std::cout << "Instantiated DemuxSis3302Plugin" << std::endl;
}

void DemuxSis3302Plugin::createSettings(QGridLayout * l)
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

void DemuxSis3302Plugin::setData(uint32_t* _data, uint32_t _len)
{
    len = _len;
    data = _data;
}

void DemuxSis3302Plugin::process()
{
    //printf("DemuxSis3302Plugin processing...\n");

    // Recover channel information
    uint8_t curCh = (len >> 29) & 0x7;
    // Revover length
    uint32_t length = (len & 0x1ffffff);
    uint32_t nofSamples = length*2;

    //printf("Current channel: %d with %d data points.\n",curCh,nofSamples);

    // Publish event data
    if(outputs->at(curCh)->hasOtherSide())
    {
        PluginConnectorThreadBuffered* bpc = dynamic_cast<PluginConnectorThreadBuffered*>(outputs->at(curCh));
        std::vector<uint32_t>* outData = new std::vector<uint32_t>(nofSamples,0);

        for(uint32_t i = 0; i < length; i++)
        {
            (*outData)[(i*2)]   = ( data[i] & 0xffff);
            (*outData)[(i*2)+1] = ((data[i] & 0xffffffff) >> 16);
        }

        //outData->assign(length,(*data));
        bpc->setData(outData);

        /*printf("Data dump:\n");
        for(uint32_t i=0; i < nofSamples; i++)
        {
            printf("<%d> %u  ",i,(*outData)[i]);
        }
        printf("\n");*/
    }
}
