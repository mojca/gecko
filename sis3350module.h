#ifndef SIS3350CONTROL_H
#define SIS3350CONTROL_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QTime>
#include <cstdio>
#include <ctime>
#include "sys/time.h"
#include <cstring>
#include <cerrno>
#include <vector>
#include <inttypes.h>

//#include "configmanager.h"
#include "basemodule.h"
#include "sis3350ui.h"
#include "sis3350.h"
#include "baseplugin.h"
#include "sis3350dmx.h"
#include "pluginmanager.h"

//#define MAXNUMDEV 10
//#define MAX_NOF_LWORDS 0x4000000 // 256 MByte
#define MAX_NOF_LWORDS 0x2000000 // 128 MByte

using namespace std;

class Sis3350config
{
public:
    enum AcMode{ringBufferAsync,ringBufferSync,directMemGateAsync,directMemGateSync,directMemStop,directMemStart};
    enum ClockSource{intFixed,intVar,extBnc,extLvds};
    enum IrqSource{endAddrLevel,endAddrEdge,endOfEvent,endOfMultiEvent};

    unsigned int base_addr;
    unsigned int sample_length;
    unsigned int direct_mem_wrap_length;
    unsigned int direct_mem_sample_length;
    unsigned int direct_mem_pretrigger_length;
    unsigned int pre_delay;
    unsigned int clock1;
    unsigned int clock2;
    unsigned int nof_reads;
    unsigned int multievent_max_nof_events;
    //unsigned int acq_mode;
    bool acq_enable_lemo;
    bool acq_multi_event;
    bool ctrl_lemo_invert;

    bool trigger_enable_lemo_out;
    int trigger_pulse_length[4];
    int trigger_gap_length[4];
    int trigger_peak_length[4];
    unsigned short trigger_threshold[4];
    unsigned short trigger_offset[4];
    bool trigger_enable[4];
    bool trigger_fir[4];
    bool trigger_gt[4];

    unsigned int ext_clock_trigger_dac_control;
    unsigned int ext_clock_trigger_daq_data;

    unsigned short variable_gain[4];
    unsigned int adc_offset[4];

    AcMode acMode;
    ClockSource clockSource;
    IrqSource irqSource;
    unsigned int pollcount;

};

class Sis3350Module : public virtual BaseModule
{
    Q_OBJECT

public:
    //enum AcMode{ringBufferAsync,ringBufferSync,directMemGateAsync,directMemGateSync,directMemStop,directMemStart};
    //enum ClockSource{intFixed,intVar,extBnc,extLvds};

    Sis3350Module(int _id, QString _name);
    ~Sis3350Module();

    // Factory method
    static AbstractModule *create (int id, const QString &name) {
        return new Sis3350Module (id, name);
    }

    void setDefaultConfig();
    virtual void saveSettings(QSettings*);
    virtual void applySettings(QSettings*);

    bool isIfaceOpen () { return getInterface()->isOpen (); }

    // Basic functions
    int arm();
    int disarm();
    int trigger();
    int timestampClear();
    bool pollTrigger();
    int ledOn();
    int ledOff();

    void setChannels();

    // Internal functions
    unsigned int configureControlStatus();
    unsigned int configureAcquisitionMode();
    unsigned int configureDirectMemory();
    unsigned int configureMultiEventRegister();
    unsigned int configureTriggers();
    unsigned int configureExternalDAC();
    unsigned int configureInputGains();
    unsigned int configureClock();
    unsigned int configureRingBuffer();
    unsigned int configureEndAddressThresholds();

    // Aliases
    int sis3350_DMA_Read_MBLT64_ADC_DataBuffer(uint32_t module_address, uint32_t adc_channel /* 0 to 3 */,
                                               uint32_t adc_buffer_sample_start_addr, uint32_t adc_buffer_sample_length,   // 16-bit words
                                               uint32_t*  dma_got_no_of_words, uint32_t* uint_adc_buffer);
    int write_dac_offset(uint32_t module_dac_control_status_addr, uint32_t dac_select_no, uint32_t dac_value);

    // VME wrapper
    int readDmaMblt64AdcDataBuffer(uint32_t module_address, uint32_t adc_channel /* 0 to 3 */,
                                   uint32_t adc_buffer_sample_start_addr, uint32_t adc_buffer_sample_length,   // 16-bit words
                                   uint32_t*  dma_got_no_of_words, uint32_t* uint_adc_buffer);
    int writeDacOffset(uint32_t module_dac_control_status_addr, uint32_t dac_select_no, uint32_t dac_value);

    // Configuration class
    Sis3350config conf;

    unsigned int addr,data;

    //uint32_t wblt_data[4][MAX_NOF_LWORDS];
    uint32_t rblt_data[4][MAX_NOF_LWORDS]; // 1 GB total
    uint32_t read_data_block_length[4];

    virtual int acquire(Event *);
    virtual bool dataReady();
    virtual int configure();
    virtual int reset();

    virtual uint32_t getBaseAddress () const;
    virtual void setBaseAddress (uint32_t baddr);

    int singleShot();
    int writeToBuffer(Event* ev);
    int acquisitionStart();
    int acquireRingBufferSync();
    int acquireRingBufferASync();
    int acquireDirectMemStart();

public slots:
    virtual void prepareForNextAcquisition();

private:
    QVector<EventSlot*> evslots;
    Sis3350Demux demux;
};



#endif // SIS3350CONTROL_H
