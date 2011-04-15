#ifndef SIS3302MODULE_H
#define SIS3302MODULE_H

#include <QObject>
#include <QMap>
#include <QTime>
#include <cstdio>
#include <ctime>
#include "sys/time.h"
#include <cstring>
#include <cerrno>
#include <vector>
#include <inttypes.h>

#include "basedaqmodule.h"
#include "confmap.h"
#include "sis3302ui.h"
#include "sis3302.h"
#include "baseplugin.h"
#include "demuxsis3302plugin.h"
#include "pluginmanager.h"

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
    uint32_t trigger_threshold[8];

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
            trigger_threshold[i]    = 50;
            dac_offset[i]           = 0;
            ch_enabled[i]           = true;
        }
    }
};

class Sis3302Module : public virtual BaseDAqModule
{
    Q_OBJECT

    typedef union { struct {
        uint8_t unused  :2;
        bool trigger    :1;
        bool wrap       :1;
        uint8_t unused2 :3;
        uint32_t addr   :25;
    }; uint32_t data; } EventDirEntry_t;

    typedef union { struct {
        uint32_t high;
        uint32_t low;
    }; uint32_t data[2]; } TimestampDir_t;

public:
    Sis3302Module(int _id, QString _name);
    ~Sis3302Module();

    // Configuration class
    Sis3302config conf;

    // Factory method
    static BaseModule *create (int id, const QString &name) {
        return new Sis3302Module (id, name);
    }

    // Mandatory to implement
    virtual void saveSettings(QSettings*);
    virtual void applySettings(QSettings*);
    void setChannels();
    ThreadBuffer<uint32_t> *getBuffer () { return buffer; }
    virtual int acquire();
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
    int arm();
    int disarm();
    int start_sampling();
    int stop_sampling();
    int reset_DDR2_logic();
    int timestamp_clear();
    int waitForSamplingComplete();
    int readAdcChannel(int ch);
    int writeToBuffer();
    int acquisitionStart();
    bool isArmedNotBusy();
    bool isNotArmedNotBusy();

    int sis3302_write_dac_offset(unsigned int *offset_value_array);

public slots:
    virtual void prepareForNextAcquisition() {}

private:
    ThreadBuffer<uint32_t> *buffer;
    virtual void createOutputPlugin();
    unsigned int addr,data;

    uint32_t readBuffer[8][SIS3302_MAX_NOF_LWORDS]; // 512 MB total
    uint32_t readLength[8];

    EventDirEntry_t eventDir[512];
    TimestampDir_t timestampDir[512];
};

#endif // SIS3302MODULE_H
