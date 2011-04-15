#include "dspadcplugin.h"
#include "pluginmanager.h"
#include "pluginconnectorqueued.h"
#include "samqvector.h"

#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>

static PluginRegistrar registrar ("dspadc", DspAdcPlugin::create, AbstractPlugin::GroupDSP);

DspAdcPlugin::DspAdcPlugin(int _id, QString _name)
    : BasePlugin(_id, _name)
{
    createSettings(settingsLayout);

    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"timestamps"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"calorimetry"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"baseline"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::out,"amplitudes"));

    std::cout << "Instantiated DspAdcPlugin" << std::endl;
}

void DspAdcPlugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        QLabel* label = new QLabel(tr("This plugin reads amplitudes at the timestamped positions."));

        cl->addWidget(label,0,0,1,2);

        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void DspAdcPlugin::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
    settings->endGroup();

    // UI update

}

void DspAdcPlugin::saveSettings(QSettings* settings)
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
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void DspAdcPlugin::userProcess()
{
    //std::cout << "DspPileUpCorrectionPlugin Processing" << std::endl;
    QVector<double> itrigger = inputs->at(0)->getData().value< QVector<double> > ();
    QVector<double> icalorimetry = inputs->at(1)->getData().value< QVector<double> > ();
    QVector<double> ibase = inputs->at(2)->getData().value< QVector<double> > ();
	
    double baseline = 0;
    double pointsForBaseline = 0;

    SamDSP dsp;

    if(!ibase.empty())
    {
        baseline = ibase.at(0);
        pointsForBaseline = ibase.at(1);
//      std::cout << "Using baseline value: " << baseline << " from " << pointsForBaseline << " points" << std::endl;
    }
    else
    {
//      std::cout << "Using standard baseline value: " << baseline << std::endl;
    }


    // Compact input data
    dsp.fast_addC(icalorimetry,-baseline);
    QVector< QVector<double> > amplitudes = dsp.select(icalorimetry, itrigger);
    outputs->first()->setData(QVariant::fromValue (amplitudes [AMP]));
}
