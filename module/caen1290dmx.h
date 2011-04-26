#ifndef DEMUXCAEN1290PLUGIN_H
#define DEMUXCAEN1290PLUGIN_H

#include <stdint.h>
#include <vector>

template <typename T> class QVector;
class EventSlot;
class Event;

class Caen1290Demux
{
public:
    Caen1290Demux (const QVector<EventSlot*> &evslots, int channels, bool hires);

    bool processData (Event *ev, const std::vector<uint32_t> &data, bool singleev);

private:
    void startEvent (uint32_t info);
    bool endEvent (Event *ev);
    void processEvent (uint32_t ev);

private:
    enum Status {Start, Measurements};
    enum Tag {GlobalHeader = 0x08, GlobalTrailer = 0x10, Filler = 0x18, TrigTimeTag = 0x11,
              TDCHeader = 0x01, TDCMeasurement = 0x00, TDCError = 0x04, TDCTrailer = 0x03};

    Status status_;
    std::vector< QVector<uint32_t> > evbuf_;
    int chans_;
    int measurementBits_;
    int channelBits_;
    const QVector<EventSlot*> &evslots_;
};

#endif // DEMUXCAEN1290PLUGIN_H
