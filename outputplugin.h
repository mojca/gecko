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

    AbstractPlugin::Group getPluginGroup () { return AbstractPlugin::GroupDemux; }
    void applySettings(QSettings *) { }
    void saveSettings(QSettings *) { }

public slots:
    void userProcess () { }

    void latchData (Event *);

protected:
    void createSettings (QGridLayout *) {}

private:
    std::map<const EventSlot*, PluginConnector*> datamap_;
};

#endif // OUTPUTPLUGIN_H
