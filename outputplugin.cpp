#include "outputplugin.h"
#include "abstractmodule.h"
#include "runmanager.h"
#include "eventbuffer.h"
#include "pluginconnectorplain.h"

#include <algorithm>

static bool evslsort (EventSlot* a, EventSlot* b) {
    return a->getName() < b->getName ();
}

OutputPlugin::OutputPlugin (AbstractModule *mod)
: BasePlugin (-1, mod->getName () + " out")
{
    const QSet<EventSlot*>* evslots = RunManager::ref ().getEventBuffer()->getEventSlots (mod);
    QList<EventSlot*> l = evslots->toList ();
    std::sort (l.begin(), l.end (), evslsort);
    foreach (EventSlot *i, l)
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
