#ifndef DEMUXCAENADCPLUGIN_H
#define DEMUXCAENADCPLUGIN_H

#include <algorithm>
#include <map>
#include <cstdio>
#include <stdint.h>
#include "caen_v785.h"

class Event;
class EventSlot;
class AbstractModule;
template <typename T> class QVector;

class CaenADCDemux
{
private:
    bool inEvent;
    int cnt;

    uint8_t nofChannels;
    uint8_t nofChannelsInEvent;
    uint8_t nofBits;
    uint8_t crateNumber;
    uint32_t eventCounter;
    uint8_t id;
    std::map<uint8_t, uint16_t> chData;
    const QVector<EventSlot*>& evslots;
    const AbstractModule *owner;

    uint32_t* it;

    void startNewEvent();
    void continueEvent();
    bool finishEvent(Event *ev);
    void printHeader();
    void printEob();

public:
    CaenADCDemux(const QVector<EventSlot*>& _evslots, const AbstractModule* op, uint chans = 32, uint bits = 12);

    bool processData (Event *ev, uint32_t* data, uint32_t len, bool singleev);
};

#endif // DEMUXCAENADCPLUGIN_H
