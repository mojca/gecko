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

        QLabel* label = new QLabel(tr("This plugin detects clipping in the input signal."));
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
    QVector<uint32_t> idata = inputs->first()->getData().value< QVector<uint32_t> > ();

    clip.fill(0., idata.size());

    // Check for clipping
    QVector<double>::iterator it(clip.begin());
    foreach(int sample, idata)
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

    outputs->first()->setData(QVariant::fromValue (clip));
}

/*!
\page dspclippingdetectorplg Clipping Detector Plugin
\li <b>Plugin names:</b> \c dspclippingdetectorplugin
\li <b>Group:</b> DSP

\section dspclippingdetectorplg_pdesc Plugin Description
The clipping detector plugin detects clipping in its input signal.
Whenever the input signal exceeds the given \c high or \c low thresholds the clipping detector sets its output for that sample to +/-1 respectively.
If it stays between the thresholds 0 is put into the output vector.

\section attrs Attributes
None

\section conf Configuration
\li <b>High Thr</b>: The high threshold
\li <b>Low Thr</b>: The low threshold

\section inputs Input Connectors
\li \c in \c &lt;uint32_t>: Input signal

\section outputs Output Connectors
\li \c clipping \c veto \c &lt;double>: A signal containing information about the signal clipping (see \ref dspclippingdetectorplg_pdesc for details)
*/
