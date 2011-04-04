#ifndef DEMUXSIS3302PLUGIN_H
#define DEMUXSIS3302PLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include "baseplugin.h"
#include "pluginconnectorthreadbuffered.h"

class BasePlugin;

class DemuxSis3302Plugin : public virtual BasePlugin
{
    Q_OBJECT


public:
    DemuxSis3302Plugin(int _id, QString _name);

    virtual void createSettings(QGridLayout*) {;}

    virtual void process() {;}
    virtual void userProcess(){;}
    virtual void setData(uint32_t* _data, uint32_t _len) {;}

    virtual void applySettings(QSettings*) {}
    virtual void saveSettings(QSettings*) {}
};

#endif // DEMUXSIS3302PLUGIN_H
