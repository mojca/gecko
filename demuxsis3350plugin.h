#ifndef DEMUXSIS3350PLUGIN_H
#define DEMUXSIS3350PLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include "baseplugin.h"
#include "pluginconnectorthreadbuffered.h"

class BasePlugin;

struct Sis3350Event
{
    uint8_t channel;
    uint64_t timeStamp;
    uint32_t sampleLen;
    uint8_t triggerCount;
    uint8_t stopDelay;
    uint8_t extraHeader;
    bool wrap;
    std::vector<uint32_t> data;
};

class DemuxSis3350Plugin : public virtual BasePlugin
{
    Q_OBJECT

protected:
    bool inHeader, inTrace;
    int cnt;

    struct Sis3350Event* curEvent[4];
    struct Sis3350Event* outEvent[4];
    int curChannel;

    uint32_t* it;
    uint32_t* data;
    uint32_t len;

    virtual void createSettings(QGridLayout*);
    void startNewHeader();
    void startNewTrace();
    void continueHeader();
    void continueTrace();
    void printHeader();

public:
    DemuxSis3350Plugin(int _id, QString _name);

    virtual void process();
    virtual void userProcess(){;}
    virtual void setData(uint32_t* _data, uint32_t _len);

    virtual void applySettings(QSettings*) {}
    virtual void saveSettings(QSettings*) {}
};

#endif // DEMUXSIS3350PLUGIN_H
