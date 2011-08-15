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
