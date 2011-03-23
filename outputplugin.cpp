#include "outputplugin.h"
#include "abstractmodule.h"
#include "runmanager.h"
#include "eventbuffer.h"
#include "pluginconnectorplain.h"

#include <QLabel>

OutputPlugin::OutputPlugin (AbstractModule *mod)
: BasePlugin (-1, mod->getName () + " out")
{
    std::vector<const EventSlot*> evslots = RunManager::ref ().getEventBuffer()->getEventSlots (mod);
    for (std::vector<const EventSlot*>::const_iterator i = evslots.begin ();
         i != evslots.end ();
         ++i)
    {
        PluginConnector *conn = new PluginConnectorPlain (this, ScopeCommon::out, (*i)->getName ());
        datamap_ [*i] = conn;
        addConnector (conn);
    }
}

void OutputPlugin::latchData (Event *ev) {
    for (std::map<const EventSlot*, PluginConnector*>::const_iterator i = datamap_.begin ();
         i != datamap_.end ();
         ++i)
    {
        const void* d = ev->get (i->first);
        if (d != NULL)
            i->second->setData (d);
    }
}
