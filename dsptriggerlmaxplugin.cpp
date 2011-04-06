#include "dsptriggerlmaxplugin.h"
#include "pluginmanager.h"
#include "pluginconnectorqueued.h"

#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>

static PluginRegistrar registrar ("dsptriggerlmax", DspTriggerLMAXPlugin::create, AbstractPlugin::GroupDSP);

DspTriggerLMAXPlugin::DspTriggerLMAXPlugin(int _id, QString _name)
    : BasePlugin(_id, _name)
{
    createSettings(settingsLayout);

    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"in"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"veto in"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::out,"trigger"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::out,"time"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::out,"veto"));

    std::cout << "Instantiated DspTriggerLMAXPlugin" << std::endl;
}

void DspTriggerLMAXPlugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        QLabel* label = new QLabel(tr("This plugin generates a trigger signal based on local maxima in the input."));
        QLabel* tlabel = new QLabel(tr("Threshold"));
        QLabel* hlabel = new QLabel(tr("Holdoff"));
        QLabel* plabel = new QLabel(tr("Positive"));

        thresholdSpinner = new QSpinBox();
        holdoffSpinner = new QSpinBox();

        thresholdSpinner->setValue(5);
        thresholdSpinner->setMaximum(100000);

        holdoffSpinner->setValue(3);
        holdoffSpinner->setMaximum(10);

        polarityBox = new QCheckBox();
        polarityBox->setChecked(true);

        connect(thresholdSpinner,SIGNAL(valueChanged(int)),this,SLOT(thresholdChanged()));
        connect(holdoffSpinner,SIGNAL(valueChanged(int)),this,SLOT(holdoffChanged()));
        connect(polarityBox,SIGNAL(toggled(bool)),this,SLOT(polarityChanged(bool)));

        cl->addWidget(label,0,0,1,2);
        cl->addWidget(tlabel,1,0,1,1);
        cl->addWidget(thresholdSpinner,1,1,1,1);
        cl->addWidget(hlabel,2,0,1,1);
        cl->addWidget(holdoffSpinner,2,1,1,1);
        cl->addWidget(plabel,3,0,1,1);
        cl->addWidget(polarityBox,3,1,1,1);

        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void DspTriggerLMAXPlugin::thresholdChanged()
{
    int newValue = thresholdSpinner->value();
    this->conf.threshold = newValue;
}

void DspTriggerLMAXPlugin::holdoffChanged()
{
    int newValue = holdoffSpinner->value();
    this->conf.holdoff = newValue;
}

void DspTriggerLMAXPlugin::polarityChanged(bool)
{
    bool newValue = polarityBox->isChecked();
    this->conf.positive = newValue;
}

void DspTriggerLMAXPlugin::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
        set = "threshold";   if(settings->contains(set)) conf.threshold = settings->value(set).toInt();
        set = "holdoff";     if(settings->contains(set)) conf.holdoff = settings->value(set).toInt();
        set = "positive";    if(settings->contains(set)) conf.positive = settings->value(set).toBool();
    settings->endGroup();

    thresholdSpinner->setValue(conf.threshold);
    holdoffSpinner->setValue(conf.holdoff);
    polarityBox->setChecked(conf.positive);
}

void DspTriggerLMAXPlugin::saveSettings(QSettings* settings)
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
            settings->setValue("threshold",conf.threshold);
            settings->setValue("holdoff"  ,conf.holdoff);
            settings->setValue("positive" ,conf.positive);
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void DspTriggerLMAXPlugin::userProcess()
{
    //std::cout << "DspTriggerLMAXPlugin Processing" << std::endl;
    QVector<double> idata = inputs->at(0)->getData().value< QVector<double> > ();
    QVector<double> iveto = inputs->at(1)->getData().value< QVector<double> > ();

    if(inputs->at(1)->getData().isNull ())
    {
        iveto.fill (0., idata.size ());
    }
    if(idata.size() != iveto.size())
    {
        std::cout << getName().toStdString() << ": data and veto have different lengths";
        return;
    }

    trigger.fill (0., idata.size());

    for(int i = 1; i<idata.size()-1; i++)
    {
        if(conf.positive)
        {
            if(idata.at(i) > conf.threshold && iveto.at(i) == 0)
            {
                if(idata.at(i-1) < idata.at(i) && idata.at(i+1) < idata.at(i))
                {
                    //std::cout << std::dec << "Trigger at " << i << std::endl;
                    //std::cout << pdata->at(i-1) << "  " << pdata->at(i) << "  " << pdata->at(i+1) << std::endl;
                    trigger[i] = 1;
                    i += conf.holdoff;
                }
            }
//            else
//            {
//                trigger[i] = 0;
//            }
        }
        else
        {
            if(idata.at(i) < -conf.threshold && iveto.at(i) == 0)
            {
                if(idata.at(i-1) > idata.at(i) && idata.at(i+1) > idata.at(i))
                {
                    trigger[i] = 1;
                    i += conf.holdoff;
                }
            }
//            else
//            {
//                trigger[i] = 0;
//            }
        }
    }

    outputs->at(0)->setData(QVariant::fromValue (trigger));
    outputs->at(1)->setData(QVariant::fromValue (idata));
    outputs->at(2)->setData(QVariant::fromValue (iveto));
}
