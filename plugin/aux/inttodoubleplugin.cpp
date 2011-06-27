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
            QVector<uint32_t> idata = inputs->at (i)->getData ().value< QVector<uint32_t> > ();
            QVector<double> odata;

            odata.reserve (idata.size ());
            for (int j = 0; j < idata.size (); ++j)
                odata << idata.at (j);
            outputs->at (i)->setData (QVariant::fromValue (odata));
            inputs->at (i)->useData ();
        }
    }
}

/*!
\page inttodoubleplg IntToDouble Plugin
\li <b>Plugin names:</b> \c inttodouble
\li <b>Group:</b> Aux

\section pdesc Plugin Description
The IntToDouble plugin is an auxiliary plugin.
It takes a uint32 input value on each of its inputs, converts it to a double and outputs the converted value to the respective output connector.

Due to program limitations, it is not currently possible to create/delete connectors after the plugin has been created.
Therefore the number of input/output connectors has to be set in the Add Plugin dialog.

\section attrs Attributes
\c nofChannels: Integer value describing the number of input/output pairs to create.

\section conf Configuration
None necessary.

\section inputs Input Connectors
\li \c in [0..n] \c &lt;uint32_t>: Input for the uint32 data to be converted

\section outputs Output Connectors
\li \c out [0..n] \c &lt;double>: Outputs for the double-converted data
*/
