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

    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"in"));
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
    QVector<double> data = inputs->first()->getData().value< QVector<double> > ();
    QVector<double> odata (data.size (), 0);
    SamDSP dsp;

    // Convert to double
    for (int i= 0; i < data.size (); ++i)
        odata [i] = data.at (i);

    //std::cout << conf.width << "  " << conf.spacing << std::endl;
    dsp.fast_pad(odata,conf.width+conf.spacing,0,odata[0]);
    dsp.fast_differentiator(odata,conf.width,conf.spacing);
    odata.resize(data.size());
    outputs->first()->setData(QVariant::fromValue (odata));
}
