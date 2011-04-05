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

    unsigned int base_addr;
    unsigned int event_length;
    unsigned int start_delay;
    unsigned int stop_delay;
    unsigned int nof_events;

    int irq_level;
    int irq_vector;

    bool autostart_acq;
    bool internal_trg_as_stop;
    bool event_length_as_stop;
    bool adc_value_big_endian;
    bool enable_page_wrap;
    bool enable_irq;
    bool enable_external_trg;

    int trigger_pulse_length[8];
    int trigger_gap_length[8];
    int trigger_peak_length[8];
    int trigger_threshold[8];

    unsigned int dac_offset[8];

    Sis3302config() : acMode(singleEvent),
                    wrapSize(ws16M),
                    avgMode(av1),
                    clockSource(creal100),
                    irqSource(endOfEvent),
                    irqMode(modeRoak),
                    base_addr(0x0),
                    event_length(100),
                    start_delay(0x0),
                    stop_delay(50),
                    nof_events(1),
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
        }
    }
};

class Sis3302Module : public virtual BaseDAqModule
{
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
    virtual int acquire() {return 0;}
    virtual bool dataReady() {return false;}
    virtual int configure();
    virtual int reset();
    virtual uint32_t getBaseAddress () const {return 0xffff0000;}
    virtual void setBaseAddress (uint32_t baddr) {};

    // Supplementary methods
    int getModuleId(unsigned int* _modId);
    int arm();
    int disarm();
    int start_sampling();
    int stop_sampling();
    int reset_DDR2_logic();
    int timestamp_clear();

    int sis3302_write_dac_offset(unsigned int *offset_value_array);

public slots:
    virtual void prepareForNextAcquisition() {};

private:
    ThreadBuffer<uint32_t> *buffer;
    virtual void createOutputPlugin();
    unsigned int addr,data;
};

#endif // SIS3302MODULE_H
