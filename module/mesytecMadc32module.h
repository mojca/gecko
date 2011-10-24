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

#ifndef MESYTECMADC32_H
#define MESYTECMADC32_H

#include "basemodule.h"
#include "baseplugin.h"
#include "mesytecMadc32dmx.h"
#include "pluginmanager.h"
#include "mesytec_madc_32_v2.h"

struct MesytecMadc32ModuleConfig {
    enum AddressSource{asBoard,asRegister};
    enum DataLengthFormat{dl8bit,dl16bit,dl32bit,dl64bit};
    enum MultiEventMode{meSingle,meMulti1,meMulti2,meMulti3};
    enum MarkingType{mtEventCounter,mtTimestamp,mtReserved2,mtExtendedTs};
    enum BankOperation{boConnected,boIndependent,boReserved2,boToggle};
    enum AdcResolution{ar2k,ar4k,ar4kHiRes,ar8k,ar8kHiRes};
    enum OutputFormat{ofMesytec};
    enum GateGeneratorMode{ggNone,ggUseGG0,ggUseGG1,ggUseGGBoth};
    enum InputRange{ir4V,ir10V,ir8V};
    enum EclGate1Mode{egGate,egOscillator};
    enum EclFClearMode{efFClear,efTimestampReset};
    enum EclBusyMode{ebBusy,ebReserved1};
    enum NimGate1Mode{ngGate,ngOscillator};
    enum NimFClearMode{nfFClear,nfTimestampReset};
    enum NimBusyMode{nbBusy,nbGate0Out,nbGate1Out,nbCbusOut,
                     nbBufFull,nbRes5,nbRes6,nbRes7,nbBufOverThr};
    enum TestPulserMode{tpOff,tpRes1,tpRes2,tpRes3,
                        tpAmp0,tpAmpLow,tpAmpHigh,tpToggle};
    enum TimeStampSource{tsVme,tsExternal};

    AddressSource addr_source;
    uint32_t base_addr;
    uint16_t base_addr_register;
    uint8_t module_id;

    uint16_t firmware_expected;

    uint8_t irq_level;
    uint8_t irq_vector;
    uint16_t irq_threshold;
    uint16_t max_transfer_data;

    uint8_t cblt_mcst_ctrl;
    uint8_t cblt_addr;
    uint8_t mcst_addr;

    uint16_t thresholds[32];
    bool enable_channel[32];

    DataLengthFormat data_length_format;

    MultiEventMode multi_event_mode;
    bool enable_multi_event_send_different_eob_marker;
    bool enable_multi_event_compare_with_max_transfer_data;

    MarkingType marking_type;
    BankOperation bank_operation;
    AdcResolution adc_resolution;
    OutputFormat output_format;

    bool enable_adc_override;
    bool enable_switch_off_sliding_scale;
    bool enable_skip_out_of_range;
    bool enable_ignore_thresholds;

    uint8_t hold_delay[2];
    uint8_t hold_width[2];
    GateGeneratorMode gate_generator_mode;

    InputRange input_range;

    bool enable_termination_input_gate0;
    bool enable_termination_input_fast_clear;

    EclGate1Mode ecl_gate1_mode;
    EclFClearMode ecl_fclear_mode;
    EclBusyMode ecl_busy_mode;

    NimGate1Mode nim_gate1_mode;
    NimFClearMode nim_fclear_mode;
    NimBusyMode nim_busy_mode;

    TestPulserMode test_pulser_mode;

    TimeStampSource time_stamp_source;
    bool enable_external_time_stamp_reset;
    uint16_t time_stamp_divisor;

    unsigned int pollcount;

    MesytecMadc32ModuleConfig ()
        : addr_source(asBoard), base_addr(0),
          base_addr_register(0),module_id(0xFF),
          firmware_expected(MADC32V2_2_EXPECTED_FIRMWARE),
          irq_level(0),irq_vector(0),irq_threshold(1),
          max_transfer_data(1),cblt_mcst_ctrl(0),
          cblt_addr(0xAA),mcst_addr(0xBB),
          data_length_format(dl64bit),
          multi_event_mode(meSingle),
          enable_multi_event_send_different_eob_marker(false),
          enable_multi_event_compare_with_max_transfer_data(false),
          marking_type(mtEventCounter),
          bank_operation(boConnected),
          adc_resolution(ar8kHiRes),
          output_format(ofMesytec),
          enable_adc_override(false),
          enable_switch_off_sliding_scale(false),
          enable_skip_out_of_range(false),
          enable_ignore_thresholds(false),
          gate_generator_mode(ggNone),
          input_range(ir10V),
          enable_termination_input_gate0(false),
          enable_termination_input_fast_clear(false),
          ecl_gate1_mode(egGate),
          ecl_fclear_mode(efFClear),
          ecl_busy_mode(ebBusy),
          nim_gate1_mode(ngGate),
          nim_fclear_mode(nfFClear),
          nim_busy_mode(nbBusy),
          test_pulser_mode(tpOff),
          time_stamp_source(tsVme),
          time_stamp_divisor(1),
          pollcount (10000)
    {
        for (int i = 0; i < MADC32V2_NUM_CHANNELS; ++i) {
            enable_channel[i] = true;
            thresholds[i] = 0;
        }
        hold_delay[0] = (20);
        hold_delay[1] = (20);
        hold_width[0] = (50);
        hold_width[1] = (50);
    }
};

class MesytecMadc32Module : public BaseModule {
	Q_OBJECT
public:
    // Factory method
    static AbstractModule *create (int id, const QString &name) {
        return new MesytecMadc32Module (id, name);
    }

    // Settings
    virtual void saveSettings (QSettings*);
    virtual void applySettings (QSettings*);

    // Functions
    int softReset();

    int irqTest();
    int irqReset();
    int irqWithdraw();

    int readoutReset();
    int startAcquisition();
    int fifoReset();

    int counterResetAB(uint8_t counter);
    int stopCounter(uint8_t counter);

    inline uint16_t getFirmwareRevision();
    inline uint16_t getBufferDataLength(); // Units are as set in data_length_format
    inline bool getDataReady();
    inline uint32_t getEventCounter();
    inline uint32_t getTimestampCounter();
    inline uint32_t getAdcBusyTime();
    inline uint32_t getGate1Time();
    inline uint64_t getTime();

    // Remote control bus control
    int writeRcBus(uint8_t addr, uint16_t data);
    uint16_t readRcBus(uint8_t addr);
    int setRcBusNumber(uint8_t number); // makes no sense here
    int setRcModuleNumber(uint8_t number);
    inline uint8_t getRcReturnStatus();
    uint8_t getRcBusNumber();
    uint8_t getRcModuleNumber();
    bool rcBusReady();

    // Mandatory virtual functions
    virtual void setChannels ();
    virtual int acquire (Event* ev);
    virtual bool dataReady ();
    virtual int reset ();
    virtual int configure ();

    virtual uint32_t getBaseAddress () const;
    virtual void setBaseAddress (uint32_t baddr);
    virtual void runStartingEvent() { dmx_.runStartingEvent(); }

    MesytecMadc32ModuleConfig *getConfig () { return &conf_; }

    int acquireSingle (uint32_t *data, uint32_t *rd);

private:
    MesytecMadc32Module (int _id, const QString &);
    void writeToBuffer(Event *ev);

public slots:
    virtual void prepareForNextAcquisition () {}
    void singleShot (uint32_t *data, uint32_t *rd);

private:
    MesytecMadc32ModuleConfig conf_;

    uint32_t event_counter;
    uint32_t data [MADC32V2_NUM_CHANNELS];
    uint32_t buffer_data_length;

    MesytecMadc32Demux dmx_;
    QVector<EventSlot*> evslots_;
};

#endif // MESYTECMADC32_H
