#ifndef DEMUXSIS3302PLUGIN_H
#define DEMUXSIS3302PLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include <QList>

#include "sis3302.h"

class Event;
class EventSlot;

class Sis3302Demux
{

public:
    Sis3302Demux (const QList<EventSlot*> &);

    virtual void process(Event *ev, uint32_t* _data, uint32_t _len);
    void setMetaData(uint32_t, EventDirEntry_t*, TimestampDir_t*);

protected:
    DataStruct_t* data;
    uint32_t len;
    EventDirEntry_t* evDir;
    TimestampDir_t* tsDir;

    uint32_t* curEvent[8];
    uint32_t nofTraces;
    const QList<EventSlot*> &evslots;

    bool pageWrap;
};

#endif // DEMUXSIS3302PLUGIN_H
