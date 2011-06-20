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

#include "eventbuilderplugin.h"
#include "pluginmanager.h"
#include "runmanager.h"
#include "pluginconnectorqueued.h"

static PluginRegistrar registrar ("eventbuilder", EventBuilderPlugin::create, AbstractPlugin::GroupPack, EventBuilderPlugin::getEventBuilderAttributeMap());

EventBuilderPlugin::EventBuilderPlugin(int _id, QString _name, const Attributes &_attrs)
            : BasePlugin(_id, _name)
            , attribs_ (_attrs)
{
    createSettings(settingsLayout);

    bool ok;
    int _nofInputs = _attrs.value ("nofInputs", QVariant (4)).toInt (&ok);
    if (!ok || _nofInputs <= 0) {
        _nofInputs = 1;
        std::cout << _name.toStdString () << ": nofInputs invalid. Setting to 1" << std::endl;
    }

    addConnector(new PluginConnectorQVUint (this,ScopeCommon::out,"out"));
    //outData.push_back(NULL);

    for(int n = 0; n < _nofInputs; n++)
    {
        addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,QString("in %1").arg(n)));
    }

    std::cout << "Instantiated EventBuilderPlugin" << std::endl;
}

AbstractPlugin::AttributeMap EventBuilderPlugin::getEventBuilderAttributeMap() {
    AbstractPlugin::AttributeMap attrs;
    attrs.insert ("nofInputs", QVariant::Int);
    return attrs;
}
AbstractPlugin::AttributeMap EventBuilderPlugin::getAttributeMap () const { return getEventBuilderAttributeMap();}
AbstractPlugin::Attributes EventBuilderPlugin::getAttributes () const { return attribs_;}

void EventBuilderPlugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        QLabel* label = new QLabel(tr("Number of inputs:"));
        QLineEdit* lineEdit = new QLineEdit(tr("%1").arg(outputs->size(),1,10));
        lineEdit->setReadOnly(true);

        cl->addWidget(label,   0,0,1,1);
        cl->addWidget(lineEdit,0,1,1,1);

        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void EventBuilderPlugin::applySettings(QSettings* settings)
{
    settings->beginGroup(getName());
    settings->endGroup();
}

void EventBuilderPlugin::saveSettings(QSettings* settings)
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

void EventBuilderPlugin::userProcess()
{
    //std::cout << "EventBuilderPlugin Processing" << std::endl;

    outputs->first()->setData(QVariant::fromValue (outData));
}
