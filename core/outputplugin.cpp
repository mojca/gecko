#include "outputplugin.h"
#include "abstractmodule.h"
#include "runmanager.h"
#include "eventbuffer.h"
#include "pluginconnectorplain.h"

#include <algorithm>

OutputPlugin::OutputPlugin (AbstractModule *mod)
: BasePlugin (-1, mod->getName () + " out")
{
    const QList<EventSlot*>* evslots = RunManager::ref ().getEventBuffer()->getEventSlots (mod);
    foreach (EventSlot *i, *evslots)
    {
        PluginConnector *conn = new PluginConnectorPlain (this, ScopeCommon::out, i->getName (), i->getDataType ());
        datamap_ [i] = conn;
        addConnector (conn);
    }
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
