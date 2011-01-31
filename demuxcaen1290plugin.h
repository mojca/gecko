#ifndef DEMUXCAEN1290PLUGIN_H
#define DEMUXCAEN1290PLUGIN_H

#include "baseplugin.h"

class DemuxCaen1290Plugin : public BasePlugin
{
public:
    DemuxCaen1290Plugin (int id, const QString &name, int channels, bool hires);

    bool processData (const std::vector<uint32_t> &data, bool singleev);
    virtual void process () {}
    virtual void userProcess () {}
    virtual void applySettings (QSettings*) {}
    virtual void saveSettings (QSettings*) {}

protected:
    void createSettings (QGridLayout*) {}

private:
    void startEvent (uint32_t info);
    bool endEvent ();
    void processEvent (uint32_t ev);

private:
    enum Status {Start, Measurements};
    enum Tag {GlobalHeader = 0x08, GlobalTrailer = 0x10, Filler = 0x18, TrigTimeTag = 0x11,
              TDCHeader = 0x01, TDCMeasurement = 0x00, TDCError = 0x04, TDCTrailer = 0x03};

    Status status_;
    std::vector< std::vector<uint32_t> > evbuf_;
    int chans_;
    int measurementBits_;
    int channelBits_;
};

#endif // DEMUXCAEN1290PLUGIN_H
