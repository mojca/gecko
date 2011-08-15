#ifndef SIS3302MODULE_H
#define SIS3302MODULE_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QTime>
#include <cstdio>
#include <ctime>
#include "sys/time.h"
#include <cstring>
#include <cerrno>
#include <vector>
#include <inttypes.h>

#include "basemodule.h"
#include "confmap.h"
#include "sis3302ui.h"
#include "sis3302.h"
#include "sis3302dmx.h"

using namespace std;

class Sis3302config
{
public:
    enum AcMode{singleEvent,multiEvent};
    enum TrgMode{ledRising,ledFalling,firRising,firFalling};
    enum WrapSize{ws64,ws128,ws256,ws512,ws1k,ws4k,ws16k,ws64k,ws256k,ws1M,ws4M,ws16M};
    enum AvgMode{av1,av2,av4,av8,av16,av32,av64,av128};
    enum ClockSource{ci100,ci50,ci25,ci10,ci1,crandom,cexternal,creal100};
    enum IrqSource{endOfEvent,endOfMultiEvent};
    enum IrqMode{modeRora,modeRoak};

    AcMode acMode;
    WrapSize wrapSize;
    AvgMode avgMode;
    ClockSource clockSource;
    IrqSource irqSource;
    IrqMode irqMode;

    TrgMode trgMode[8];

    uint32_t base_addr;
    uint32_t event_length;
    uint32_t event_sample_start_addr;
    uint32_t start_delay;
    uint32_t stop_delay;
    uint32_t nof_events;
    uint32_t poll_count;

    uint32_t irq_level;
    uint32_t irq_vector;

    bool autostart_acq;
    bool internal_trg_as_stop;
    bool event_length_as_stop;
    bool adc_value_big_endian;
    bool enable_page_wrap;
    bool enable_irq;
    bool enable_external_trg;

    uint32_t trigger_pulse_length[8];
    uint32_t trigger_gap_length[8];
    uint32_t trigger_peak_length[8];
    int32_t trigger_threshold[8];

    uint32_t dac_offset[8];
    bool ch_enabled[8];

    Sis3302config() : acMode(singleEvent),
                    wrapSize(ws16M),
                    avgMode(av1),
                    clockSource(creal100),
                    irqSource(endOfEvent),
                    irqMode(modeRoak),
                    base_addr(0x0),
                    event_length(100),
                    event_sample_start_addr(0),
                    start_delay(0x0),
                    stop_delay(50),
                    nof_events(1),
                    poll_count(0x10000000),
                    irq_level(7),
                    irq_vector(0xff),
                    autostart_acq(true),
                    internal_trg_as_stop(true),
                    event_length_as_stop(true),
                    adc_value_big_endian(false),
                    enable_page_wrap(false),
                    enable_irq(false),
                    enable_external_trg(false)
    {
        for (int i=0; i<8; i++)
        {
            trgMode[i] = firRising;
            trigger_pulse_length[i] = 10;
            trigger_gap_length[i]   = 8;
            trigger_peak_length[i]  = 4;
            trigger_threshold[i]    = -50;
            dac_offset[i]           = 37000; // About the middle of the input range
            ch_enabled[i]           = true;
        }
    }
};

class Sis3302Module : public BaseModule
{
    Q_OBJECT


public:

    Sis3302Module(int _id, QString _name);
    ~Sis3302Module();

    // Configuration class
    Sis3302config conf;

    // Factory method
    static AbstractModule *create (int id, const QString &name) {
        return new Sis3302Module (id, name);
    }

    // Mandatory to implement
    virtual void saveSettings(QSettings*);
    virtual void applySettings(QSettings*);
    void setChannels();
    virtual int acquire(Event *);
    virtual bool dataReady();
    virtual int configure();
    virtual int reset();
    virtual uint32_t getBaseAddress () const { return conf.base_addr; }
    virtual void setBaseAddress (uint32_t baddr) { conf.base_addr = baddr; }

    // Supplementary methods
    int getModuleId(uint32_t* _modId);
    int getEventCounter(uint32_t*);
    int getNextSampleAddr(int adc, uint32_t* _addr);
    int getTimeStampDir();
    int getEventDir(int ch);
    int arm();
    int disarm();
    int start_sampling();
    int stop_sampling();
    int reset_DDR2_logic();
    int timestamp_clear();
    int waitForSamplingComplete();
    int readAdcChannel(int ch, uint32_t _reqNofWords);
    int acquisitionStartSingle();
    int acquisitionStartMulti();
    int checkConfig();
    int writeToBuffer(Event *);
    bool isArmedNotBusy();
    bool isNotArmedNotBusy();

    uint32_t getWrapSizeFromConfig(Sis3302config::WrapSize);

    int sis3302_write_dac_offset(unsigned int *offset_value_array);

public slots:
    virtual void prepareForNextAcquisition() {}

private:
    unsigned int addr,data;

    uint32_t readBuffer[8][SIS3302_MAX_NOF_LWORDS]; // 512 MB total
    uint32_t readLength[8];

    EventDirEntry_t eventDir[8][512];
    TimestampDir_t timestampDir[512];
    QList<EventSlot*> evslots;
    Sis3302Demux dmx;
};

#endif // SIS3302MODULE_H
