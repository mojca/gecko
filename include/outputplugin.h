#ifndef OUTPUTPLUGIN_H
#define OUTPUTPLUGIN_H

#include "baseplugin.h"

class AbstractModule;
class Event;
class EventSlot;

/*! The OutputPlugin class handles the transition from the module world to the plugin world.
 *  It generates output connectors from the EventSlots registered by its owning module and
 *  transfers the corrsponding data from the Event objects to them.
 */
class OutputPlugin : public BasePlugin {
    Q_OBJECT
public:
    OutputPlugin (AbstractModule *mod);

    AbstractPlugin::Group getPluginGroup () const { return AbstractPlugin::GroupDemux; }
    void applySettings(QSettings *) { }
    void saveSettings(QSettings *) { }

    bool isSlotConnected (const EventSlot *sl) const {
        return (datamap_.find (sl) != datamap_.end() && datamap_.at (sl)->hasOtherSide ());
    }

public slots:
    void userProcess () { }

    void latchData (Event *);

protected:
    void createSettings (QGridLayout *) {}

private:
    std::map<const EventSlot*, PluginConnector*> datamap_;
};

#endif // OUTPUTPLUGIN_H
