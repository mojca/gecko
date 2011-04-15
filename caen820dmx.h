#ifndef DEMUXCAEN820PLUGIN_H
#define DEMUXCAEN820PLUGIN_H

#include <stdint.h>
#include <QVector>

class Event;
class EventSlot;

class Caen820Demux
{
public:
    Caen820Demux (const QVector<EventSlot*> &);

    bool processData (Event *ev, const uint32_t *data, int len);
    void setChannelBitmap (uint32_t bmp) { enabledch_ = bmp; }
    void setHeaderEnabled (bool en) { hdrenabled_ = en; }
    void setShortDataFmt (bool isshort) { shortfmt_ = isshort; }

private:
    uint32_t enabledch_;
    bool shortfmt_;
    bool hdrenabled_;
    const QVector<EventSlot*> &evslots_;
};

#endif // DEMUXCAEN820PLUGIN_H
