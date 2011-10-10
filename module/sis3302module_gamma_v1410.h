/*
Copyright 2011 Bastian Loeher, Roland Wirth

This file is part of GECKO.

GECKO is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

GECKO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SIS3302MODULE_GAMMA_V1410_H
#define SIS3302MODULE_GAMMA_V1410_H

/* This changes between MCA mode and normal mode */
//#define SIS3302_V1410_MCA_MODE
/* Change between normal and reduced addressing mode */
//#define SIS3302_V1410_REDUCED_ADDR_SPACE

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
#include "sis3302ui_gamma_v1410.h"
#include "sis3302_gamma_v1410.h"
#include "sis3302dmx_gamma_v1410.h"

using namespace std;

class Sis3302V1410config
{
public:
    enum AcMode{singleEvent,multiEvent};
    enum ClockSource{ci100,ci50,ci25,ci10,ci1,ci100k,cexternal,creal100};
    enum IrqSource{endOfAddrThrEdge,endOfAddrThr};
    enum IrqMode{modeRora,modeRoak};
    enum TrgDecimMode{trgDecimOff,trgDecim2,trgDecim4,trgDecim8};
    enum EnDecimMode{enDecimOff,enDecim2,enDecim4,enDecim8};
    enum EnTestMode{testMwdTrapez,testMwTrapez,testTrapez};
    enum EnSampleMode{enModeTrapez,enModeNothing,enMode3Parts,enModeCustom};
    enum LemoInMode{lemoInVeto,lemoInGate};
    enum VmeMode{vmeSingle,vmeDMA32,vmeFIFO,vmeBLT32,vmeMBLT64,vme2E};
#if defined(SIS3302_V1410_MCA_MODE)
    enum LemoOutMode{lemoOutTrg,lemoOutMscan,lemoOutScan};
#else
    // With respect to output 1,2,3 ... 4 is always clock out
    enum LemoOutMode{lemoOutTrgBsyArm,lemoOutTrgVetoArm,lemoOutNmTrgNp,lemoOutNmVetoNp};
#endif

    // Generic setup parameters
    uint32_t base_addr;
    uint32_t poll_count;
    AcMode acMode;
    VmeMode vmeMode;
    ClockSource clockSource;
    bool enable_user_led;

    // Addressing setup values
    bool enable_vipa;
    bool enable_geo_addressing;
    bool enable_reduced_addressing;
    bool enable_a32_addressing;

    // Firmware
    uint16_t module_id;
    uint8_t  firmware_major_rev;
    uint8_t  firmware_minor_rev;

    // Event config parameters
    uint16_t header_id[4];
    uint16_t raw_sample_length[4]; /* in samples, quad aligned */
    uint16_t raw_data_sample_start_idx[4]; /* in samples, only even */
    uint32_t end_addr_thr_in_samples[4]; /* given in samples */
    bool enable_input_invert[8];

    // Lemo I/O setup parameters
    LemoInMode lemo_in_mode;
    LemoOutMode lemo_out_mode;
    bool enable_lemo_in[3];
    bool send_int_trg_to_ext_as_or;

    // MCA parameters
#if defined(SIS3302_V1410_MCA_MODE)
    enum McaHistogramSize{hist1k,hist2k,hist4k,hist8k};
    enum LneSource{lneExternalPulse,lneInternal10Mhz};
    McaHistogramSize mca_histogram_size[4];
    LneSource load_next_source;
    uint32_t load_next_prescale_devider;
    uint32_t nof_mca_scan_histograms;
    uint32_t nof_mca_multiscans;
    uint32_t energy_subtract_offset[8];
    uint8_t  energy_multiplier[8];
    uint8_t  energy_divider[8];
    bool start_scan_on_bank_2;
    bool disable_scan_histogram_autoclear;
    bool enable_50kHz_trigger[8];
    bool enable_mem_write_test_mode[4];
    bool enable_allow_pileup[4];
    bool enable_histogramming[8];
#endif

    // IRQ setup parameters
    IrqMode irqMode;
    IrqSource irqSource;
    uint8_t irq_level;
    uint8_t irq_vector;
    uint16_t nof_events;
    bool enable_irq;
    bool enable_external_trg;
    bool update_irq;

    // Trigger config parameters
    TrgDecimMode trigger_decim_mode[8];
    uint32_t trigger_pulse_length[8];
    uint32_t trigger_sumg_length[8];
    uint32_t trigger_peak_length[8];
    uint16_t trigger_gate_length[4];
    uint16_t trigger_pretrigger_delay[4];
    uint8_t  trigger_int_gate_length[8];
    uint8_t  trigger_int_trg_delay[8];
    int32_t  trigger_threshold[8];
    bool enable_ch[8];
    bool enable_next_adc_gate[8];
    bool enable_prev_adc_gate[8];
    bool enable_next_adc_trg[8];
    bool enable_prev_adc_trg[8];
    bool enable_ext_gate[8];
    bool enable_ext_trg[8];
    bool enable_int_gate[8];
    bool enable_int_trg[8];
    bool disable_trg_out[8];

    // Energy config parameters
    EnDecimMode energy_decim_mode[4];
    EnTestMode energy_test_mode[4];
    EnSampleMode energy_sample_mode[4];
    uint32_t energy_peak_length[4];
    uint32_t energy_sumg_length[4];
    uint16_t energy_gate_length[4];
    uint16_t energy_sample_length[4]; /* in samples, only even */
    uint16_t energy_sample_start_idx[4][3];
    uint8_t  energy_tau[8];
    bool enable_energy_extra_filter[4];

    // Daq config parameters
    uint32_t dac_offset[8];


    Sis3302V1410config() :
        base_addr(0x0),
        poll_count(0x10000000),
        acMode(singleEvent),
        vmeMode(vmeSingle),
        clockSource(creal100),
        enable_user_led(false),
        enable_vipa(false),
        enable_geo_addressing(false),
        enable_reduced_addressing(false),
        enable_a32_addressing(true),
        module_id(0x3302),
        firmware_major_rev(0x14),
        firmware_minor_rev(0x10),
        lemo_in_mode(lemoInVeto),
        lemo_out_mode(lemoOutTrgBsyArm),
        send_int_trg_to_ext_as_or(false),
#if defined(SIS3302_V1410_MCA_MODE)
        load_next_source(lneExternalPulse),
        load_next_prescale_devider(0),
        nof_mca_scan_histograms(0),
        nof_mca_multiscans(0),
        start_scan_on_bank_2(false),
        disable_scan_histogram_autoclear(false),
#endif
        irqMode(modeRoak),
        irqSource(endOfAddrThrEdge),
        irq_level(7),
        irq_vector(0xff),
        nof_events(1),
        enable_irq(false),
        enable_external_trg(false),
        update_irq(false)
    {
        enable_lemo_in[0] = true;
        enable_lemo_in[1] = true;
        enable_lemo_in[2] = true;

        for (int i=0; i<4; i++) {
            header_id[i]                    = 0x3302;
            raw_sample_length[i]            = 2048;
            raw_data_sample_start_idx[i]    = 0;
            end_addr_thr_in_samples[i]      = 10*(raw_sample_length[i]+4); /* TODO */
            trigger_gate_length[i]          = 400;
            trigger_pretrigger_delay[i]     = 20;
            energy_decim_mode[i]            = enDecimOff;
            energy_test_mode[i]             = testMwdTrapez;
            energy_sample_mode[i]           = enModeTrapez;
            energy_peak_length[i]           = 180;
            energy_sumg_length[i]           = 40;
            energy_gate_length[i]           = 1000;
            energy_sample_length[i]         = 0;
            energy_sample_start_idx[i][0]   = 0;
            energy_sample_start_idx[i][1]   = 0;
            energy_sample_start_idx[i][2]   = 0;
            enable_energy_extra_filter[i]   = false;
#if defined(SIS3302_V1410_MCA_MODE)
            mca_histogram_size[i]           = hist8k;
            enable_mem_write_test_mode[i]   = false;
            enable_allow_pileup[i]          = true;
#endif
        }

        for (int i=0; i<8; i++) {
            enable_input_invert[i]      = false;
            trigger_decim_mode[i]       = trgDecimOff;
            trigger_pulse_length[i]     = 10;
            trigger_sumg_length[i]      = 12;
            trigger_peak_length[i]      = 8;
            trigger_int_gate_length[i]  = 10;
            trigger_int_trg_delay[i]    = 0;
            trigger_threshold[i]        = 0;
            dac_offset[i]               = 37000; // About the middle of the input range
            energy_tau[i]               = 0;
            enable_ch[i]                = true;
            enable_next_adc_gate[i]     = false;
            enable_prev_adc_gate[i]     = false;
            enable_next_adc_trg[i]      = false;
            enable_prev_adc_trg[i]      = false;
            enable_ext_gate[i]          = false;
            enable_ext_trg[i]           = false;
            enable_int_gate[i]          = false;
            enable_int_trg[i]           = true;
            disable_trg_out[i]          = false;
#if defined(SIS3302_V1410_MCA_MODE)
            energy_subtract_offset[i]   = 0;
            energy_multiplier[i]        = 0x8;
            energy_divider[i]           = 0x3; // This is full range for 8k
            enable_50kHz_trigger[i]     = false;
            enable_histogramming[i]     = true;
#endif
        }
    }
};

class Sis3302V1410Module : public BaseModule
{
    Q_OBJECT

public	:
    static const uint8_t NOF_CHANNELS = 8;
    static const uint8_t NOF_ADC_GROUPS = 4;
    static const char*	 MODULE_NAME;

public:

    Sis3302V1410Module(int _id, QString _name);
    ~Sis3302V1410Module();

    // Configuration class
    Sis3302V1410config conf;

    // Factory method
    static AbstractModule *create (int id, const QString &name) {
        return new Sis3302V1410Module (id, name);
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
    void init();
    int getModuleId(uint32_t* _modId);
    int getEventCounter(uint32_t*);
    int getNextSampleAddr(int adc, uint32_t* _addr);
    int getTimeStampDir();
    int getEventDir(int ch);
    int arm(uint8_t bank);
    int disarm();
    int trigger();
    int start_sampling();
    int stop_sampling();
    int resetSampleLogic();
    int reset_DDR2_logic();
    int timestamp_clear();
    int waitForNotBusy();
    int waitForAddrThreshold();
    int readAdcChannelSinglePage(int ch, uint32_t _reqNofWords);
    int readAdcChannel(int ch, uint32_t _reqNofWords);
    int singleShot();
    int acquisitionStartSingle();
    int acquisitionStartMulti();
    int checkConfig();
    int writeToBuffer(Event *);
    int getMcaTrgStartCounter(uint8_t ch, uint32_t* _evCnt);
    int updateModuleInfo();
    int getDecimationFactor(Sis3302V1410config::EnDecimMode);
    int getDecimationFactor(Sis3302V1410config::TrgDecimMode); /*! Overload */
    void updateEndAddrThresholds();
    void updateGateLengths();
    bool isArmed(uint8_t bank);
    bool isArmedOrBusy();
    bool isArmedNotBusy(uint8_t bank);
    bool isNotArmedNotBusy();

    int write_dac_offset(unsigned int *offset_value_array);

    // Info reporting
    void INFO(const char* msg);
    void INFO(const char* msg, uint32_t a);
    void INFO(const char* msg, uint32_t a, uint32_t b);
    void INFO_i(const char* msg, int i);
    void INFO_i(const char* msg, int i, uint32_t a);
    void INFO_i(const char* msg, int i, uint32_t a, uint32_t b);

    // Error reporting
    void ERROR_i(const char* e, int i, uint32_t v);
    void ERROR_i(const char* e, int i, uint32_t a, uint32_t b); /*! Overload */
    void ERROR(const char* e, uint32_t v);
    void ERROR(const char* e, uint32_t a, uint32_t b); /*! Overload */

    // Data dumping
    void DUMP(const char* name, uint32_t* buf, uint32_t len);
    void DUMP(const char* name, int32_t* buf, uint32_t len); /*! Overload */

public slots:
    virtual void prepareForNextAcquisition() {}

private:
    unsigned int addr,data;

    uint32_t readBuffer[NOF_CHANNELS][SIS3302_V1410_MAX_NOF_LWORDS]; // 8 MB total
    uint32_t readLength[NOF_CHANNELS];
    uint32_t endSampleAddr_words[NOF_CHANNELS];

public:
    // Values for display in the module
    uint64_t currentTimestamp[NOF_CHANNELS];
    uint16_t currentRawBuffer[NOF_CHANNELS][SIS3302_V1410_MAX_NOF_RAW_SAMPLES];
    uint16_t currentHeader[NOF_CHANNELS];
    uint16_t currentRawLengthFromHeader[NOF_CHANNELS];
    uint8_t currentTriggerCounter[NOF_CHANNELS];
    int32_t currentEnergyBuffer[NOF_CHANNELS][SIS3302_V1410_MAX_NOF_ENERGY_SAMPLES];
    int32_t currentEnergyMaxValue[NOF_CHANNELS];
    int32_t currentEnergyFirstValue[NOF_CHANNELS];
    bool currentPileupFlag[NOF_CHANNELS];
    bool currentRetriggerFlag[NOF_CHANNELS];

private:
    QList<EventSlot*> evslots;
    Sis3302V1410Demux dmx;

};

#endif // SIS3302MODULE_GAMMA_V1410_H
