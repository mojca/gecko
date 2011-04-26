#ifndef DEMUXSIS3302PLUGIN_H
#define DEMUXSIS3302PLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include <QList>

class Event;
class EventSlot;

class Sis3302Demux
{
public:
    Sis3302Demux (const QList<EventSlot*> &);

    virtual void process(Event *ev, uint32_t* _data, uint32_t _len);

protected:
    uint32_t* data;
    uint32_t len;

    uint32_t* curEvent[8];
    const QList<EventSlot*> &evslots;
};

#endif // DEMUXSIS3302PLUGIN_H
