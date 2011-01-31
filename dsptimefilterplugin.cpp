#include "dsptimefilterplugin.h"
#include "pluginmanager.h"
#include "pluginconnectorqueued.h"

#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>

static PluginRegistrar registrar ("dsptimefilter", DspTimeFilterPlugin::create, AbstractPlugin::GroupDSP);

DspTimeFilterPlugin::DspTimeFilterPlugin(int _id, QString _name)
    : BasePlugin(_id, _name)
{
    createSettings(settingsLayout);

    addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,"in"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::out,"timing"));

    std::cout << "Instantiated DspTimeFilterPlugin" << std::endl;
}

void DspTimeFilterPlugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        QLabel* label = new QLabel(tr("This plugin does a bibox based convolution of the input data."));
        QLabel* wlabel = new QLabel(tr("Width"));
        QLabel* slabel = new QLabel(tr("Spacing"));

        widthSpinner = new QSpinBox();
        spacingSpinner = new QSpinBox();

        widthSpinner->setValue(5);
        spacingSpinner->setValue(5);

        connect(widthSpinner,SIGNAL(valueChanged(int)),this,SLOT(widthChanged()));
        connect(spacingSpinner,SIGNAL(valueChanged(int)),this,SLOT(spacingChanged()));

        cl->addWidget(label,0,0,1,2);
        cl->addWidget(wlabel,1,0,1,1);
        cl->addWidget(slabel,2,0,1,1);
        cl->addWidget(widthSpinner,1,1,1,1);
        cl->addWidget(spacingSpinner,2,1,1,1);

        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void DspTimeFilterPlugin::widthChanged()
{
    int newValue = widthSpinner->value();
    this->conf.width = newValue;
}

void DspTimeFilterPlugin::spacingChanged()
{
    int newValue = spacingSpinner->value();
    this->conf.spacing = newValue;
}

void DspTimeFilterPlugin::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
        set = "width";   if(settings->contains(set)) conf.width = settings->value(set).toInt();
        set = "spacing";   if(settings->contains(set)) conf.spacing = settings->value(set).toInt();
    settings->endGroup();

    // UI update
    widthSpinner->setValue(conf.width);
    spacingSpinner->setValue(conf.spacing);
}

void DspTimeFilterPlugin::saveSettings(QSettings* settings)
{
    if(settings == NULL)
    {
        std::cout << getName().toStdString() << ": no settings file" << std::endl;
        return;
    }
    else
    {
        std::cout << getName().toStdString() << " saving settings...";
        settings->beginGroup(getName());
            settings->setValue("width",conf.width);
            settings->setValue("spacing",conf.spacing);
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void DspTimeFilterPlugin::userProcess()
{
    //std::cout << "DspTimeFilterPlugin Processing" << std::endl;
    const vector<uint32_t>* pdata = reinterpret_cast<const std::vector<uint32_t>*>(inputs->first()->getData());
    SamDSP dsp;

    // Convert to double
    outData.assign((*pdata).begin(),(*pdata).end());

    //std::cout << conf.width << "  " << conf.spacing << std::endl;
    dsp.fast_pad(outData,conf.width+conf.spacing,0,outData[0]);
    dsp.fast_differentiator(outData,conf.width,conf.spacing);
    outData.resize(pdata->size());
    outputs->first()->setData(&outData);
}
