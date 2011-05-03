#include "fanoutplugin.h"

static PluginRegistrar registrarInt ("fanoutInt", FanOutPlugin<uint32_t>::create, AbstractPlugin::GroupAux, FanOutPlugin<uint32_t>::getFanoutAttributeMap ());
static PluginRegistrar registrarDouble ("fanoutDouble", FanOutPlugin<double>::create, AbstractPlugin::GroupAux, FanOutPlugin<double>::getFanoutAttributeMap ());

/*!
\page fanoutplg FanOut Plugin
<b>Plugin names:</b> \c fanoutInt, \c fanoutDouble
<b>Group:</b> Aux

\section desc Plugin Description
The fanout plugin is an auxiliary plugin.
It takes a single input and makes it available on \c nofOutputs output connectors.
It is available for int and double vector data.

Due to program limitations, it is not currently possible to create/delete connectors after the plugin has been created.
Therefore the number of output connectors has to be set in the Add Plugin dialog.

\section attrs Attributes
\c nofOutputs: Integer value describing the number of outputs to create.

\section conf Configuration
None necessary.

\section inputs Input Connectors
\c in: Input for the data to be fanned out

\section outputs Output Connectors
\c out[0..n]: Outputs for the fanned-out data
*/
