#ifndef OUTPUTPLUGIN_H
#define OUTPUTPLUGIN_H

#include "baseplugin.h"

class AbstractModule;
class Event;
class EventSlot;

class OutputPlugin : public BasePlugin {
    Q_OBJECT
public:
    OutputPlugin (AbstractModule *mod);

    AbstractPlugin::Group getPluginGroup () { return AbstractPlugin::GroupUnspecified; }
    void applySettings(QSettings *) { }
    void saveSettings(QSettings *) { }

public slots:
    void userProcess () { }

    void latchData (Event *);

private:
    std::map<const EventSlot*, PluginConnector*> datamap_;
};

#endif // OUTPUTPLUGIN_H
