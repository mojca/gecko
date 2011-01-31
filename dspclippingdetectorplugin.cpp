#include "dspclippingdetectorplugin.h"
#include "pluginmanager.h"
#include "pluginconnectorqueued.h"

#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>

static PluginRegistrar registrar ("dspclippingdetector", DspClippingDetectorPlugin::create, AbstractPlugin::GroupDSP);

DspClippingDetectorPlugin::DspClippingDetectorPlugin(int _id, QString _name)
    : BasePlugin(_id, _name)
{
    createSettings(settingsLayout);

    addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,"in"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::out,"clipping veto"));

    std::cout << "Instantiated DspClippingDetectorPlugin" << std::endl;
}

void DspClippingDetectorPlugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        QLabel* label = new QLabel(tr("This plugin does a box filter convolution of the input data."));
        QLabel* hlabel = new QLabel(tr("High Thr"));
        QLabel* llabel = new QLabel(tr("Low Thr"));

        lowSpinner = new QSpinBox();
        highSpinner = new QSpinBox();

        lowSpinner->setValue(5);
        highSpinner->setValue(5);

        lowSpinner->setMaximum(1000000);
        highSpinner->setMaximum(1000000);

        connect(lowSpinner,SIGNAL(valueChanged(int)),this,SLOT(lowChanged()));
        connect(highSpinner,SIGNAL(valueChanged(int)),this,SLOT(highChanged()));

        cl->addWidget(label,0,0,1,2);
        cl->addWidget(hlabel,1,0,1,1);
        cl->addWidget(llabel,2,0,1,1);
        cl->addWidget(highSpinner,1,1,1,1);
        cl->addWidget(lowSpinner,2,1,1,1);

        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void DspClippingDetectorPlugin::lowChanged()
{
    int newValue = lowSpinner->value();
    this->conf.low = newValue;
}

void DspClippingDetectorPlugin::highChanged()
{
    int newValue = highSpinner->value();
    this->conf.high = newValue;
}

void DspClippingDetectorPlugin::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
        set = "low";   if(settings->contains(set)) conf.low = settings->value(set).toInt();
        set = "high";   if(settings->contains(set)) conf.high = settings->value(set).toInt();
    settings->endGroup();

    // UI update
    lowSpinner->setValue(conf.low);
    highSpinner->setValue(conf.high);
}

void DspClippingDetectorPlugin::saveSettings(QSettings* settings)
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
            settings->setValue("low",conf.low);
            settings->setValue("high",conf.high);
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void DspClippingDetectorPlugin::userProcess()
{
    //std::cout << "DspClippingDetectorPlugin Processing" << std::endl;
    const vector<uint32_t>* pdata = reinterpret_cast<const std::vector<uint32_t>*>(inputs->first()->getData());

    clip.resize((*pdata).size(),0);

    // Check for clipping
    vector<double>::iterator it(clip.begin());
    foreach(int sample, (*pdata))
    {
        if(sample >= conf.high)
        {
            (*it) = +1.0;
        }
        else if(sample <= conf.low )
        {
            (*it) = -1.0;
        }
        else
        {
            (*it) = 0.0;
        }

        it++;
    }

    outputs->first()->setData(&clip);
}
