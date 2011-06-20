/*
Copyright 2011 Bastian Loeher, Roland Wirth

This file is part of GECKO.

GECKO is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

GECKO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dsppileupcorrectionplugin.h"
#include "pluginmanager.h"
#include "pluginconnectorqueued.h"

#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>

static PluginRegistrar registrar ("dsppileupcorrect", DspPileUpCorrectionPlugin::create, AbstractPlugin::GroupDSP);

DspPileUpCorrectionPlugin::DspPileUpCorrectionPlugin(int _id, QString _name)
    : BasePlugin(_id, _name)
{
    createSettings(settingsLayout);

    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"timestamps"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"calorimetry"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"signal shape"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"baseline"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::out,"unpiled amplitudes"));

    std::cout << "Instantiated DspExtractSignalPlugin" << std::endl;
}

void DspPileUpCorrectionPlugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        QLabel* label = new QLabel(tr("This plugin corrects piled-up amplitudes using a matrix inversion algorithm."));
        QLabel* sillabel = new QLabel(tr("Signals left"));
        QLabel* sirlabel = new QLabel(tr("Signals right"));
        QLabel* sallabel = new QLabel(tr("Samples left"));
        QLabel* sarlabel = new QLabel(tr("Samples right"));

        signalsLeftSpinner = new QSpinBox();
        signalsRightSpinner = new QSpinBox();
        samplesLeftSpinner = new QSpinBox();
        samplesRightSpinner = new QSpinBox();

        signalsLeftSpinner->setMaximum(10);
        signalsRightSpinner->setMaximum(10);
        samplesLeftSpinner->setMaximum(10000);
        samplesRightSpinner->setMaximum(10000);

        signalsLeftSpinner->setValue(3);
        signalsRightSpinner->setValue(3);
        samplesLeftSpinner->setValue(100);
        samplesRightSpinner->setValue(100);

        connect(signalsLeftSpinner,SIGNAL(valueChanged(int)),this,SLOT(signalsLeftChanged(int)));
        connect(signalsRightSpinner,SIGNAL(valueChanged(int)),this,SLOT(signalsRightChanged(int)));
        connect(samplesLeftSpinner,SIGNAL(valueChanged(int)),this,SLOT(samplesLeftChanged(int)));
        connect(samplesRightSpinner,SIGNAL(valueChanged(int)),this,SLOT(samplesRightChanged(int)));

        cl->addWidget(label,0,0,1,2);
        cl->addWidget(sillabel,1,0,1,1);
        cl->addWidget(signalsLeftSpinner,1,1,1,1);
        cl->addWidget(sirlabel,2,0,1,1);
        cl->addWidget(signalsRightSpinner,2,1,1,1);
        cl->addWidget(sallabel,3,0,1,1);
        cl->addWidget(samplesLeftSpinner,3,1,1,1);
        cl->addWidget(sarlabel,4,0,1,1);
        cl->addWidget(samplesRightSpinner,4,1,1,1);

        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void DspPileUpCorrectionPlugin::signalsLeftChanged(int newValue)
{
    this->conf.signalsLeft = newValue;
}

void DspPileUpCorrectionPlugin::signalsRightChanged(int newValue)
{
    this->conf.signalsRight = newValue;
}

void DspPileUpCorrectionPlugin::samplesLeftChanged(int newValue)
{
    this->conf.samplesLeft = newValue;
}

void DspPileUpCorrectionPlugin::samplesRightChanged(int newValue)
{
    this->conf.samplesRight = newValue;
}

void DspPileUpCorrectionPlugin::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
        set = "signalsLeft";   if(settings->contains(set)) conf.signalsLeft = settings->value(set).toInt();
        set = "signalsRight";   if(settings->contains(set)) conf.signalsRight = settings->value(set).toInt();
        set = "samplesLeft";   if(settings->contains(set)) conf.samplesLeft = settings->value(set).toInt();
        set = "samplesRight";   if(settings->contains(set)) conf.samplesRight = settings->value(set).toInt();
    settings->endGroup();

    // UI update
    signalsLeftSpinner->setValue(conf.signalsLeft);
    signalsRightSpinner->setValue(conf.signalsRight);
    samplesLeftSpinner->setValue(conf.samplesLeft);
    samplesRightSpinner->setValue(conf.samplesRight);

}

void DspPileUpCorrectionPlugin::saveSettings(QSettings* settings)
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
            settings->setValue("signalsLeft",conf.signalsLeft);
            settings->setValue("signalsRight",conf.signalsRight);
            settings->setValue("samplesLeft",conf.samplesLeft);
            settings->setValue("samplesRight",conf.samplesRight);
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void DspPileUpCorrectionPlugin::userProcess()
{
    //std::cout << "DspPileUpCorrectionPlugin Processing" << std::endl;
    std::vector<double> itrigger = inputs->at(0)->getData().value< QVector<double> > ().toStdVector ();
    std::vector<double> icalorimetry = inputs->at(1)->getData().value< QVector<double> > ().toStdVector ();
    std::vector<double> ishape = inputs->at(2)->getData().value< QVector<double> > ().toStdVector ();
    std::vector<double> ibase = inputs->at(3)->getData().value< QVector<double> > ().toStdVector ();

    double baseline = 0;
    double pointsForBaseline = 0;

    if(ishape.empty ()) // no shape input present
    {
        ishape = std::vector<double>(100,1.0);
    }

    if(!ibase.empty ())
    {
        baseline = ibase.at(0);
        pointsForBaseline = ibase.at(1);
//        std::cout << "Using baseline value: " << baseline << " from " << pointsForBaseline << " points" << std::endl;
    }
//    else
//    {
//        std::cout << "Using standard baseline value: " << baseline << std::endl;
//    }

    SamDSP dsp;

    // Compact input data
    std::vector<std::vector<double> > amplitudes = dsp.select(icalorimetry,itrigger);
    dsp.fast_addC(amplitudes[AMP],-baseline);

    // Find maximum in pshape
    int tz = dsp.max(ishape)[TIME];

    // Create dimension vector
    std::vector<int> dim(4,0);
    dim[0] = conf.signalsLeft;
    dim[1] = conf.signalsRight;
    dim[2] = conf.samplesLeft;
    dim[3] = conf.samplesRight;

//    for(int i = 0; i < 4; i++)
//    {
//        std::cout << "dim[" << i << "] : " << dim[i] << endl;
//    }

    // Unpile signals
    QVector<double> outData = QVector<double>::fromStdVector (dsp.unpile(amplitudes[TIME],amplitudes[AMP],dim,ishape,tz));

    outputs->first()->setData(QVariant::fromValue (outData));
}
