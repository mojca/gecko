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

#include "dspampspecplugin.h"
#include "pluginmanager.h"
#include "pluginconnectorqueued.h"
#include "samqvector.h"

#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>

static PluginRegistrar registrar ("dspampspec", DspAmpSpecPlugin::create, AbstractPlugin::GroupDSP);

DspAmpSpecPlugin::DspAmpSpecPlugin(int _id, QString _name)
    : BasePlugin(_id, _name)
{
    nofLowClip = 0;
    nofHiClip = 0;

    estimateForBaseline = 0;
    outData = new QVector<double>(4096,0.);

    createSettings(settingsLayout);

    addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,"in"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::out,"spectrum"));

    std::cout << "Instantiated DspAmpSpecPlugin" << std::endl;
}

void DspAmpSpecPlugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        QLabel* label = new QLabel(tr("This plugin does a bibox based convolution of the input data."));
        QLabel* wlabel = new QLabel(tr("Width"));
        QLabel* blabel = new QLabel(tr("Points for Baseline"));
        QLabel* llabel = new QLabel(tr("Low Clip"));
        QLabel* hlabel = new QLabel(tr("High Clip"));

        widthSpinner = new QSpinBox();
        baselineSpinner = new QSpinBox();
        lowClip = new QLineEdit(tr("%1").arg(nofLowClip,1,10));
        lowClip->setReadOnly(true);
        hiClip = new QLineEdit(tr("%1").arg(nofHiClip,1,10));
        hiClip->setReadOnly(true);

        widthSpinner->setValue(3);
        widthSpinner->setSingleStep(2);
        baselineSpinner->setValue(10);

        resetButton = new QPushButton(tr("Reset spectra"));
        connect(resetButton,SIGNAL(clicked()),this,SLOT(resetSpectra()));

        connect(widthSpinner,SIGNAL(valueChanged(int)),this,SLOT(widthChanged()));
        connect(baselineSpinner,SIGNAL(valueChanged(int)),this,SLOT(baselineChanged()));

        cl->addWidget(label,0,0,1,2);
        cl->addWidget(wlabel,1,0,1,1);
        cl->addWidget(blabel,2,0,1,1);
        cl->addWidget(llabel,4,0,1,1);
        cl->addWidget(hlabel,5,0,1,1);
        cl->addWidget(widthSpinner,1,1,1,1);
        cl->addWidget(baselineSpinner,2,1,1,1);
        cl->addWidget(resetButton,3,0,1,2);
        cl->addWidget(lowClip,4,1,1,1);
        cl->addWidget(hiClip,5,1,1,1);

        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void DspAmpSpecPlugin::widthChanged()
{
    int newValue = widthSpinner->value();
    if((newValue % 2)==0)
    {
        newValue++;
        widthSpinner->setValue(newValue);
    }
    this->conf.width = newValue;
}

void DspAmpSpecPlugin::baselineChanged()
{
    int newValue = baselineSpinner->value();
    this->conf.pointsForBaseline = newValue;
}

void DspAmpSpecPlugin::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
        set = "width";   if(settings->contains(set)) conf.width = settings->value(set).toInt();
        set = "pointsForBaseline";   if(settings->contains(set)) conf.pointsForBaseline = settings->value(set).toInt();
    settings->endGroup();

    widthSpinner->setValue(conf.width);
    baselineSpinner->setValue(conf.pointsForBaseline);
}

void DspAmpSpecPlugin::saveSettings(QSettings* settings)
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
            settings->setValue("pointsForBaseline",conf.pointsForBaseline);
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void DspAmpSpecPlugin::userProcess()
{
    double tmp = 0.;
    double pol = -1.;

    //std::cout << "DspAmpSpecPlugin Processing" << std::endl;
    QVector<uint32_t> idata = inputs->first()->getData().value< QVector<uint32_t> > ();
    SamDSP dsp;

    // Convert to double
    QVector<double> data (idata.size ());
    std::copy (idata.begin (), idata.end (), data.begin ());

    // Correct baseline
    tmp = 0.;
    for(int i = 0; i<conf.pointsForBaseline && i<(int)(data.size()); i++)
    {
        tmp += data [i];
    }
    estimateForBaseline = tmp / conf.pointsForBaseline;

    // Find extends
    QVector<double> min = dsp.min(data);
    QVector<double> max = dsp.max(data);
    QVector<double> peak = min;


    // Find polarity
    if((estimateForBaseline - min[AMP]) < (max[AMP] - estimateForBaseline))
    {
        pol = 1.;
        peak = max;
    }
    else
    {
        pol = -1.;
    }

    //std::cout << peak[TIME] << "  " << peak[AMP] << std::endl;

    if(peak[TIME] > 0 && peak[TIME] < (data.size()-1))
    {
        if(peak[AMP] >= 4095)
        {

            if(    (data[peak[TIME]-1] == peak[AMP])
                || (data[peak[TIME]+1] == peak[AMP]))
            {
                nofHiClip++;
                //hiClip->setText(tr("%1").arg(nofHiClip,1,10)); // Needs to be done from gui thread
                return;
            }

        }
        else if(peak[AMP] == 0)
        {
            if(    (data[peak[TIME]-1] == peak[AMP])
                || (data[peak[TIME]+1] == peak[AMP]))
            {
                nofLowClip++;
                //lowClip->setText(tr("%1").arg(nofLowClip,1,10));
                return;
            }
        }
    }

    // Average over maximum to get stable amplitude
    tmp = 0.;
    int halfWidth = floor(conf.width / 2);
    for(int i = peak[TIME]-halfWidth; i <= peak[TIME]+halfWidth; i++)
    {
        if(i > 0 && i < (int)(data.size()))
        tmp += (data[i]-estimateForBaseline);
    }
    estimateForAmplitude = (tmp / conf.width)*pol ;

    // Sort into histogram
    if(estimateForAmplitude > 1 && estimateForAmplitude < 4095)
    {
        //std::cout << "amp: "  << estimateForAmplitude << std::endl;
        (*outData) [(int)(estimateForAmplitude)]++;
    }

    outputs->first()->setData(QVariant::fromValue (*outData));
}

void DspAmpSpecPlugin::resetSpectra()
{
    outData->fill(0., 4096);
    nofLowClip = 0;
    lowClip->setText(tr("%1").arg(nofLowClip,1,10));
    nofHiClip = 0;
    hiClip->setText(tr("%1").arg(nofHiClip,1,10));
}
