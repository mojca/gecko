#include "dspadcplugin.h"
#include "pluginmanager.h"
#include "pluginconnectorqueued.h"

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
    const vector<double>* ptrigger = reinterpret_cast<const std::vector<double>*>(inputs->at(0)->getData());
    const vector<double>* pcalorimetry = reinterpret_cast<const std::vector<double>*>(inputs->at(1)->getData());
    const vector<double>* pbase = reinterpret_cast<const std::vector<double>*>(inputs->at(2)->getData());
	
    double baseline = 0;
    double pointsForBaseline = 0;

    SamDSP dsp;

    if(pbase && pbase->size() > 0)
    {
        baseline = pbase->at(0);
        pointsForBaseline = pbase->at(1);
//      std::cout << "Using baseline value: " << baseline << " from " << pointsForBaseline << " points" << std::endl;
    }
    else
    {
//      std::cout << "Using standard baseline value: " << baseline << std::endl;
    }


    // Compact input data
    vector<double> calorimetry = dsp.addC((*pcalorimetry),-baseline);
    vector<vector<double> > amplitudes = dsp.select(calorimetry,(*ptrigger));

    outData.resize(amplitudes[0].size(),0);
    outData = amplitudes[AMP];

    outputs->first()->setData(&outData);
}
