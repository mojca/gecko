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

#include "outputplugin.h"
#include "abstractmodule.h"
#include "runmanager.h"
#include "eventbuffer.h"
#include "pluginconnectorplain.h"

#include <algorithm>

OutputPlugin::OutputPlugin (AbstractModule *mod)
: BasePlugin (-1, mod->getName () + " out"),
  owner(mod)
{
    const QList<EventSlot*>* evslots = RunManager::ref ().getEventBuffer()->getEventSlots (mod);
    foreach (EventSlot *i, *evslots)
    {
        PluginConnector *conn = new PluginConnectorPlain (this, ScopeCommon::out, i->getName (), i->getDataType ());
        datamap_ [i] = conn;
        addConnector (conn);
    }
}

void OutputPlugin::runStartingEvent() {
    owner->runStartingEvent();
}

void OutputPlugin::latchData (Event *ev) {
    for (std::map<const EventSlot*, PluginConnector*>::const_iterator i = datamap_.begin ();
         i != datamap_.end ();
         ++i)
    {
        QVariant d = ev->get (i->first);
        if (!d.isNull ())
            i->second->setData (d);
    }
}
