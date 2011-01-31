#ifndef DEMUXCAENADCPLUGIN_H
#define DEMUXCAENADCPLUGIN_H

#include <algorithm>
#include <map>
#include <cstdio>
#include <stdint.h>
#include <QPushButton>
#include "baseplugin.h"
#include "caen_v785.h"

class BasePlugin;

class DemuxCaenADCPlugin : public virtual BasePlugin
{
    Q_OBJECT

private:
    bool scheduleReset;
    bool inEvent;
    int cnt;

    uint8_t nofChannels;
    uint8_t nofChannelsInEvent;
    uint8_t nofBits;
    uint8_t crateNumber;
    uint32_t eventCounter;
    uint8_t id;
    std::map<uint8_t, uint16_t> chData;

    uint32_t* it;

    void startNewEvent();
    void continueEvent();
    bool finishEvent();
    void printHeader();
    void printEob();

    QPushButton* resetButton;

protected:
    virtual void createSettings(QGridLayout*);

public:
    DemuxCaenADCPlugin(int _id, QString _name, const Attributes &attrs);

    virtual void process() {}
    virtual void userProcess() {}
    bool processData (uint32_t* data, uint32_t len, bool singleev);

    virtual void applySettings(QSettings*) {}
    virtual void saveSettings(QSettings*) {}
    AttributeMap getAttributeList () const;

public slots:
    void resetSpectra();
};

#endif // DEMUXCAENADCPLUGIN_H
