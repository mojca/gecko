#ifndef DEMUXCAEN820PLUGIN_H
#define DEMUXCAEN820PLUGIN_H

#include "baseplugin.h"

class DemuxCaen820Plugin : public BasePlugin
{
public:
    DemuxCaen820Plugin (int id, QString name);

    bool processData (const uint32_t *data, int len);
    virtual void process () {}
    virtual void userProcess () {}
    virtual void applySettings (QSettings*) {}
    virtual void saveSettings (QSettings*) {}

    void setChannelBitmap (uint32_t bmp) { enabledch_ = bmp; }
    void setHeaderEnabled (bool en) { hdrenabled_ = en; }
    void setShortDataFmt (bool isshort) { shortfmt_ = isshort; }

protected:
    void createSettings (QGridLayout*) {}

private:
    uint32_t enabledch_;
    bool shortfmt_;
    bool hdrenabled_;
};

#endif // DEMUXCAEN820PLUGIN_H
