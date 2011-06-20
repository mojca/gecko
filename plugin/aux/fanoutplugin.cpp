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
