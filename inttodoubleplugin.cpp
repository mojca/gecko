#include "inttodoubleplugin.h"
#include "pluginmanager.h"
#include "pluginconnectorqueued.h"

#include <iostream>
#include <string>

#include <QGridLayout>
#include <QLabel>

static PluginRegistrar reg ("int->double", &IntToDoublePlugin::create, AbstractPlugin::GroupAux, IntToDoublePlugin::getIntToDoubleAttributeMap ());

IntToDoublePlugin::IntToDoublePlugin(int id, QString name, const Attributes &attrs)
    : BasePlugin(id, name)
    , attrs_ (attrs)
{
    nofChannels_ = attrs_.value ("nofChannels", 1).toInt ();
    if (nofChannels_ <= 0) {
        std::cout << "Invalid number of channels: " << nofChannels_ << "! Setting to 1" << std::endl;
        nofChannels_ = 1;
    }

    attrs_.insert ("nofChannels", nofChannels_);

    for (int i = 0; i < nofChannels_; ++i) {
        addConnector (new PluginConnectorQVUint (this, ScopeCommon::in, QString ("in %1").arg (i)));
        addConnector (new PluginConnectorQVDouble (this, ScopeCommon::out, QString ("out %1").arg (i)));
    }

    outData_.resize (nofChannels_);
}

void IntToDoublePlugin::createSettings (QGridLayout *l) {
    l->addWidget (new QLabel (tr ("%1 channel uint32 to double convertor").arg (nofChannels_)), 0, 0, 1, 1);
    l->setRowStretch (1, 1);
}

AbstractPlugin::AttributeMap IntToDoublePlugin::getIntToDoubleAttributeMap () {
    AbstractPlugin::AttributeMap attrs;
    attrs.insert ("nofChannels", QVariant::Int);
    return attrs;
}

AbstractPlugin::AttributeMap IntToDoublePlugin::getAttributeMap () const {
    return getIntToDoubleAttributeMap ();
}

AbstractPlugin::Attributes IntToDoublePlugin::getAttributes () const {
    return attrs_;
}

void IntToDoublePlugin::process () {
    for (int i = 0; i < nofChannels_; ++i) {
        if (inputs->at (i)->dataAvailable ()) {
            const std::vector<uint32_t> *pdata = reinterpret_cast<const std::vector<uint32_t>*> (inputs->at (i)->getData ());
            outData_ [i].assign (pdata->begin (), pdata->end ());
            outputs->at (i)->setData (&outData_ [i]);
            inputs->at (i)->useData ();
        }
    }
}
