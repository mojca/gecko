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

#include "mesytecMadc32module.h"
#include "mesytecMadc32ui.h"
#include "modulemanager.h"
#include "runmanager.h"
#include "confmap.h"

#include <cstdio>
#include <cstring>
#include <unistd.h> // usleep()

using namespace std;
static ModuleRegistrar reg1 ("mesytecMadc32", MesytecMadc32Module::create);

MesytecMadc32Module::MesytecMadc32Module (int i, const QString &n)
    : BaseModule (i, n)
    , firmware(0)
    , module_id(0)
    , event_counter(0)
    , timestamp_counter(0)
    , adc_busy_counter(0)
    , gate1_time_counter(0)
    , time_counter(0)
    , buffer_data_length(0)
    , dmx_ (evslots_, this)
{
    setChannels ();
    createOutputPlugin();

    setUI (new MesytecMadc32UI (this));
        std::cout << "Instantiated MesytecMadc32 module" << std::endl;
}

void MesytecMadc32Module::setChannels () {
    EventBuffer *evbuf = RunManager::ref ().getEventBuffer ();

    // Per channel outputs
    for(int i = 0; i < MADC32V2_NUM_CHANNELS; i++)
        evslots_ << evbuf->registerSlot (this, tr("out %1").arg(i,1,10), PluginConnector::VectorUint32);

    // Output for raw data -> to event builder
    evslots_ << evbuf->registerSlot(this, "raw out", PluginConnector::VectorUint32);
}

int MesytecMadc32Module::configure () {
    AbstractInterface *iface = getInterface ();

    uint32_t baddr = conf_.base_addr;
    uint16_t data;
    int ret = 0;

    if(!iface) return 2;
    if(!iface->isOpen()) return 1;

    // set address source
    ret = iface->writeA32D16 (baddr + MADC32V2_ADDR_SOURCE, conf_.addr_source);
    if (ret) printf ("Error %d at MADC32V2_ADDR_SOURCE", ret);

    // set address register
    ret = iface->writeA32D16(baddr + MADC32V2_ADDR_REGISTER, conf_.base_addr_register);
    if (ret) printf ("Error %d at MADC32V2_ADDR_REGISTER", ret);

    // set module id
    ret = iface->writeA32D16(baddr + MADC32V2_MODULE_ID, conf_.module_id);
    if (ret) printf ("Error %d at MADC32V2_MODULE_ID", ret);

    // set irq level
    ret = iface->writeA32D16(baddr + MADC32V2_IRQ_LEVEL, conf_.irq_level);
    if (ret) printf ("Error %d at MADC32V2_", ret);

    // set irq vector
    ret = iface->writeA32D16(baddr + MADC32V2_IRQ_VECTOR, conf_.irq_vector);
    if (ret) printf ("Error %d at MADC32V2_IRQ_VECTOR", ret);

    // set irq threshold
    ret = iface->writeA32D16(baddr + MADC32V2_IRQ_THRESHOLD, conf_.irq_threshold);
    if (ret) printf ("Error %d at MADC32V2_IRQ_THRESHOLD", ret);

    // set max transfer data
    ret = iface->writeA32D16(baddr + MADC32V2_MAX_TRANSFER_DATA, conf_.max_transfer_data);
    if (ret) printf ("Error %d at MADC32V2_MAX_TRANSFER_DATA", ret);

    // set cblt address
    ret = iface->writeA32D16(baddr + MADC32V2_CBLT_ADDRESS,
                             (conf_.cblt_addr << MADC32V2_OFF_CBLT_ADDRESS));
    if (ret) printf ("Error %d at MADC32V2_CBLT_ADDRESS", ret);

    // set mcst address
    ret = iface->writeA32D16(baddr + MADC32V2_MCST_ADDRESS,
                             (conf_.mcst_addr << MADC32V2_OFF_MCST_ADDRESS));
    if (ret) printf ("Error %d at MADC32V2_MCST_ADDRESS", ret);

    // set data length format
    ret = iface->writeA32D16(baddr + MADC32V2_DATA_LENGTH_FORMAT, conf_.data_length_format);
    if (ret) printf ("Error %d at MADC32V2_DATA_LENGTH_FORMAT", ret);

    // set multi event mode
    data = conf_.multi_event_mode;
    if(conf_.enable_multi_event_compare_with_max_transfer_data)
        data |= MADC32V2_VAL_MULTIEVENT_MODE_MAX_DATA;
    if(conf_.enable_multi_event_send_different_eob_marker)
        data |= MADC32V2_VAL_MULTIEVENT_MODE_EOB_BERR;
    ret = iface->writeA32D16(baddr + MADC32V2_MULTIEVENT_MODE, data);
    if (ret) printf ("Error %d at MADC32V2_MULTIEVENT_MODE", ret);

    // set marking type
    ret = iface->writeA32D16(baddr + MADC32V2_MARKING_TYPE, conf_.marking_type);
    if (ret) printf ("Error %d at MADC32V2_MARKING_TYPE", ret);

    // set bank mode
    ret = iface->writeA32D16(baddr + MADC32V2_BANK_MODE, conf_.bank_operation);
    if (ret) printf ("Error %d at MADC32V2_BANK_MODE", ret);

    // set adc resolution
    ret = iface->writeA32D16(baddr + MADC32V2_ADC_RESOLUTION, conf_.adc_resolution);
    if (ret) printf ("Error %d at MADC32V2_ADC_RESOLUTION", ret);

    // set output format (not necessary at the moment)
    ret = iface->writeA32D16(baddr + MADC32V2_OUTPUT_FORMAT, conf_.output_format);
    if (ret) printf ("Error %d at MADC32V2_OUTPUT_FORMAT", ret);

    // set adc override
    ret = iface->writeA32D16(baddr + MADC32V2_ADC_OVERRIDE,
                             (conf_.enable_adc_override ?
                                  conf_.adc_override_resolution : conf_.adc_resolution));
    if (ret) printf ("Error %d at MADC32V2_ADC_OVERRIDE", ret);

    // set sliding scale disable
    ret = iface->writeA32D16(baddr + MADC32V2_SLIDING_SCALE_OFF,
                             (conf_.enable_switch_off_sliding_scale ? 1 : 0));
    if (ret) printf ("Error %d at MADC32V2_SLIDING_SCALE_OFF", ret);

    // set skip out of range
    ret = iface->writeA32D16(baddr + MADC32V2_SKIP_OUT_OF_RANGE,
                             (conf_.enable_skip_out_of_range ? 1 : 0));
    if (ret) printf ("Error %d at MADC32V2_SKIP_OUT_OF_RANGE", ret);

    // set ignore thresholds
    ret = iface->writeA32D16(baddr + MADC32V2_IGNORE_THRESHOLDS,
                             (conf_.enable_ignore_thresholds ? 1 : 0));
    if (ret) printf ("Error %d at MADC32V2_IGNORE_THRESHOLDS", ret);

    // set hold delay 0
    ret = iface->writeA32D16(baddr + MADC32V2_HOLD_DELAY_0, conf_.hold_delay[0]);
    if (ret) printf ("Error %d at MADC32V2_HOLD_DELAY_0", ret);

    // set hold delay 1
    ret = iface->writeA32D16(baddr + MADC32V2_HOLD_DELAY_1, conf_.hold_delay[1]);
    if (ret) printf ("Error %d at MADC32V2_HOLD_DELAY_1", ret);

    // set hold width 0
    ret = iface->writeA32D16(baddr + MADC32V2_HOLD_WIDTH_0, conf_.hold_width[0]);
    if (ret) printf ("Error %d at MADC32V2_HOLD_WIDTH_0", ret);

    // set hold width 1
    ret = iface->writeA32D16(baddr + MADC32V2_HOLD_WIDTH_1, conf_.hold_width[1]);
    if (ret) printf ("Error %d at MADC32V2_HOLD_WIDTH_1", ret);

    // set gate generator mode
    ret = iface->writeA32D16(baddr + MADC32V2_USE_GATE_GENERATOR, conf_.gate_generator_mode);
    if (ret) printf ("Error %d at MADC32V2_USE_GATE_GENERATOR", ret);

    // set input range
    ret = iface->writeA32D16(baddr + MADC32V2_INPUT_RANGE, conf_.input_range);
    if (ret) printf ("Error %d at MADC32V2_INPUT_RANGE", ret);

    // set ecl termination
    data = 0;
    if(conf_.enable_termination_input_gate0) data |= (1 << MADC32V2_OFF_ECL_TERMINATION_GATE_0);
    if(conf_.enable_termination_input_fast_clear) data |= (1 << MADC32V2_OFF_ECL_TERMINATION_FCLEAR);
    ret = iface->writeA32D16(baddr + MADC32V2_ECL_TERMINATED, data);
    if (ret) printf ("Error %d at MADC32V2_ECL_TERMINATED", ret);

    // set ecl gate 1 mode
    ret = iface->writeA32D16(baddr + MADC32V2_ECL_GATE1_OSC, conf_.ecl_gate1_mode);
    if (ret) printf ("Error %d at MADC32V2_ECL_GATE1_OSC", ret);

    // set ecl fast clear mode
    ret = iface->writeA32D16(baddr + MADC32V2_ECL_FAST_CLEAR_RST, conf_.ecl_fclear_mode);
    if (ret) printf ("Error %d at MADC32V2_ECL_FAST_CLEAR_RST", ret);

    // set ecl busy mode
    ret = iface->writeA32D16(baddr + MADC32V2_ECL_BUSY, conf_.ecl_busy_mode);
    if (ret) printf ("Error %d at MADC32V2_ECL_BUSY", ret);

    // set nim gate 1 mode
    ret = iface->writeA32D16(baddr + MADC32V2_NIM_GATE1_OSC, conf_.nim_gate1_mode);
    if (ret) printf ("Error %d at MADC32V2_NIM_GATE1_OSC", ret);

    // set nim fast clear mode
    ret = iface->writeA32D16(baddr + MADC32V2_NIM_FAST_CLEAR_RST, conf_.nim_fclear_mode);
    if (ret) printf ("Error %d at MADC32V2_NIM_FAST_CLEAR_RST", ret);

    // set nim busy mode
    ret = iface->writeA32D16(baddr + MADC32V2_NIM_BUSY, conf_.nim_busy_mode);
    if (ret) printf ("Error %d at MADC32V2_NIM_BUSY", ret);

    // set pulser status
    ret = iface->writeA32D16(baddr + MADC32V2_PULSER_STATUS, conf_.test_pulser_mode);
    if (ret) printf ("Error %d at MADC32V2_PULSER_STATUS", ret);

    // set template
    //ret = iface->writeA32D16(baddr + MADC32V2_, conf_.);
    //if (ret) printf ("Error %d at MADC32V2_", ret);

    // set channel thresholds
    for (int i = 0; i < MADC32V2_NUM_CHANNELS; ++i) {
        uint16_t thr = (conf_.enable_channel[i] ?
                            conf_.thresholds[i] :
                            MADC32V2_VAL_THRESHOLD_SWITCH_OFF);
        ret = iface->writeA32D16 (baddr + MADC32V2_THRESHOLD_MEM + 2*i,thr);
        if (ret) printf ("Error %d at MADC32V2_THRESHOLD_MEM[%d]\n", ret, i);
    }

    //REG_DUMP();

    ret = counterResetAll ();
    return ret;
}

void MesytecMadc32Module::REG_DUMP() {
    int nof_reg_groups = 2;
    uint32_t start_addr[] = {0x00004000, 0x00006000};
    uint32_t end_addr[]   = {0x0000403e, 0x000060ae};
    uint16_t data;

    printf("MesytecMadc32Module::REG_DUMP:\n");
    for(int i = 0; i < nof_reg_groups; ++i) {
        for(uint32_t addr = conf_.base_addr + start_addr[i]; addr < conf_.base_addr + end_addr[i]; addr += 2) {
            if(getInterface()->readA32D16(addr,&data) == 0) {
                printf("0x%08x: 0x%04x\n",addr,data);
            }
        }
        printf("*\n");
    }
    fflush(stdout);
}

int MesytecMadc32Module::counterResetAll () {
    return counterResetAB(MADC32V2_VAL_RST_COUNTER_AB_ALL);
}

int MesytecMadc32Module::counterResetAB(uint8_t counter) {
    return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_RESET_COUNTER_AB,counter);
}

int MesytecMadc32Module::irqReset() {
    return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_IRQ_RESET, 1);
}

int MesytecMadc32Module::irqTest() {
    return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_IRQ_TEST, 1);
}

int MesytecMadc32Module::irqWithdraw() {
    return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_IRQ_WITHDRAW, 1);
}

int MesytecMadc32Module::readoutReset () {
    return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_READOUT_RESET, 1);
}

int MesytecMadc32Module::startAcquisition() {
    return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_START_ACQUISITION, 1);
}

int MesytecMadc32Module::stopAcquisition() {
    return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_START_ACQUISITION, 0);
}

int MesytecMadc32Module::fifoReset () {
    return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_FIFO_RESET, 1);
}

int MesytecMadc32Module::stopCounter (uint8_t counter) {
    return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_STOP_COUNTER, counter);
}

// Warning: This reset also resets parameters! Do not issue after configuration!
int MesytecMadc32Module::softReset () {
    return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_SOFT_RESET, 1);
}

int MesytecMadc32Module::reset () {
    irqReset();
    counterResetAll();
    fifoReset();
    readoutReset();
    //dataReset();
    return softReset ();

}

uint16_t MesytecMadc32Module::getBufferDataLength() const {
        uint16_t data = 0;
        getInterface ()->readA32D16 (conf_.base_addr + MADC32V2_BUFFER_DATA_LENGTH, &data);
        return data;
}

uint16_t MesytecMadc32Module::getFirmwareRevision() {
        if (!firmware)
        getInterface ()->readA32D16 (conf_.base_addr + MADC32V2_FIRMWARE_REVISION, &firmware);
        conf_.firmware_revision_minor = firmware & 0xff;
        conf_.firmware_revision_major = (firmware >> 16) & 0xff;
        return firmware;
}

uint16_t MesytecMadc32Module::getModuleIdConfigured() {
        if (!module_id)
        getInterface ()->readA32D16 (conf_.base_addr + MADC32V2_MODULE_ID, &module_id);
        return module_id;
}

bool MesytecMadc32Module::checkFirmware()  {
    if(!firmware) getFirmwareRevision();
    if(firmware != conf_.firmware_expected) {
        printf("MesytecMadc32Module::checkFirmware : Firmware mismatch (0x%04x, expected: 0x%04x)\n",
               firmware,conf_.firmware_expected);
        return false;
    }
    return true;
}

int MesytecMadc32Module::getAllCounters() {
    event_counter = getEventCounter();
    timestamp_counter = getTimestampCounter();
    adc_busy_counter = getAdcBusyTime();
    gate1_time_counter = getGate1Time();
    time_counter = getTime();
    return 0;
}

uint32_t MesytecMadc32Module::getEventCounter() {
    int ret = 0;
    uint32_t data32;
    uint16_t data;

    AbstractInterface *iface = getInterface ();

    ret = iface->readA32D16 (conf_.base_addr + MADC32V2_EVENT_COUNTER_LOW, &data);
    if (ret) printf ("Error %d at MADC32V2_EVENT_COUNTER_LOW\n", ret);
    data32 = data;

    ret = iface->readA32D16 (conf_.base_addr + MADC32V2_EVENT_COUNTER_HIGH, &data);
    if (ret) printf ("Error %d at MADC32V2_EVENT_COUNTER_HIGH\n", ret);
    data32 |= (((uint32_t)(data) & 0x0000ffff) << 16);

    printf("Event counter: %d\n",data32);

    return data32;
}

uint32_t MesytecMadc32Module::getTimestampCounter() {
    int ret = 0;
    uint32_t data32;
    uint16_t data;

    AbstractInterface *iface = getInterface ();

    ret = iface->readA32D16 (conf_.base_addr + MADC32V2_TIMESTAMP_CNT_L, &data);
    if (ret) printf ("Error %d at MADC32V2_TIMESTAMP_CNT_L", ret);
    data32 = data;

    ret = iface->readA32D16 (conf_.base_addr + MADC32V2_TIMESTAMP_CNT_H, &data);
    if (ret) printf ("Error %d at MADC32V2_TIMESTAMP_CNT_H", ret);
    data32 |= (((uint32_t)(data) & 0x0000ffff) << 16);

    return data32;
}

uint32_t MesytecMadc32Module::getAdcBusyTime() {
    int ret = 0;
    uint32_t data32;
    uint16_t data;

    AbstractInterface *iface = getInterface ();

    ret = iface->readA32D16 (conf_.base_addr + MADC32V2_ADC_BUSY_TIME_LOW, &data);
    if (ret) printf ("Error %d at MADC32V2_ADC_BUSY_TIME_LOW", ret);
    data32 = data;

    ret = iface->readA32D16 (conf_.base_addr + MADC32V2_ADC_BUSY_TIME_HIGH, &data);
    if (ret) printf ("Error %d at MADC32V2_ADC_BUSY_TIME_HIGH", ret);
    data32 |= (((uint32_t)(data) & 0x0000ffff) << 16);

    return data32;
}

uint32_t MesytecMadc32Module::getGate1Time() {
    int ret = 0;
    uint32_t data32;
    uint16_t data;

    AbstractInterface *iface = getInterface ();

    ret = iface->readA32D16 (conf_.base_addr + MADC32V2_GATE_1_TIME_LOW, &data);
    if (ret) printf ("Error %d at MADC32V2_GATE_1_TIME_LOW", ret);
    data32 = data;

    ret = iface->readA32D16 (conf_.base_addr + MADC32V2_GATE_1_TIME_HIGH, &data);
    if (ret) printf ("Error %d at MADC32V2_GATE_1_TIME_HIGH", ret);
    data32 |= (((uint32_t)(data) & 0x0000ffff) << 16);

    return data32;
}

uint64_t MesytecMadc32Module::getTime() {
    int ret = 0;
    uint32_t data64 = 0LL;
    uint16_t data;

    AbstractInterface *iface = getInterface ();

    ret = iface->readA32D16 (conf_.base_addr + MADC32V2_TIME_0, &data);
    if (ret) printf ("Error %d at MADC32V2_TIME_0", ret);
    data64 = data;

    ret = iface->readA32D16 (conf_.base_addr + MADC32V2_TIME_1, &data);
    if (ret) printf ("Error %d at MADC32V2_TIME_1", ret);
    data64 |= (((uint64_t)(data) & 0xffffLL) << 16);

    ret = iface->readA32D16 (conf_.base_addr + MADC32V2_TIME_2, &data);
    if (ret) printf ("Error %d at MADC32V2_TIME_2", ret);
    data64 |= (((uint64_t)(data) & 0xffffLL) << 32);

    return data64;
}

 bool MesytecMadc32Module::getDataReady() {
    int ret;
    uint16_t data;
    ret = getInterface ()->readA32D16 (conf_.base_addr + MADC32V2_DATA_READY, &data);
    if (ret) printf ("Error %d at MADC32V2_DATA_READY", ret);

    return (data == 1);
}

bool MesytecMadc32Module::dataReady () {
    return getDataReady();
}

int MesytecMadc32Module::acquire (Event* ev) {
    int ret;

    ret = acquireSingle (data, &buffer_data_length);
    if (ret == 0) writeToBuffer(ev);
    else printf("MesytecMadc32Module::Error at acquireSingle\n");

    return buffer_data_length;
}

void MesytecMadc32Module::writeToBuffer(Event *ev)
{
    bool go_on = dmx_.processData (ev, data, buffer_data_length, RunManager::ref ().isSingleEventMode ());
    if (!go_on) {
        // Do what has to be done to finish this acquisition cycle
        // fifoReset();
    }
}

int MesytecMadc32Module::acquireSingle (uint32_t *data, uint32_t *rd) {
    *rd = 0;

    // Get buffer data length
    uint32_t words_to_read = 0;
    buffer_data_length = getBufferDataLength();
    //printf("madc32: Event length (buffer_data_length): %d\n",buffer_data_length);

    // Translate buffer data length to number of words to read
    switch(conf_.data_length_format) {
    case MesytecMadc32ModuleConfig::dl8bit:
        words_to_read = buffer_data_length / 4;
        break;
    case MesytecMadc32ModuleConfig::dl16bit:
        words_to_read = buffer_data_length / 2;
        break;
    case MesytecMadc32ModuleConfig::dl64bit:
        words_to_read = buffer_data_length * 2;
        break;
    case MesytecMadc32ModuleConfig::dl32bit:
    default:
        words_to_read = buffer_data_length;
    }

    ++words_to_read;
    //printf("madc32: Words to read: %d\n",words_to_read);

    // Read the data fifo
    uint32_t addr = conf_.base_addr + MADC32V2_DATA_FIFO;
    //printf("madc32: acquireSingle from addr = 0x%08x\n",addr);

    switch(conf_.vme_mode) {

    case MesytecMadc32ModuleConfig::vmFIFO:
    {
        //printf("MesytecMadc32ModuleConfig::vmFIFO\n");
        int ret = getInterface ()->readA32FIFO(addr, data, words_to_read, rd);
        if (!*rd && ret && !getInterface ()->isBusError (ret)) {
            printf ("Error %d at MADC32V2_DATA_FIFO with FIFO read\n", ret);
            return ret;
        }
        break;
    }
    case MesytecMadc32ModuleConfig::vmDMA32:
    {
        //printf("MesytecMadc32ModuleConfig::vmDMA32\n");
        int ret = getInterface ()->readA32DMA32(addr, data, words_to_read, rd);
        if (!*rd && ret && !getInterface ()->isBusError (ret)) {
            printf ("Error %d at MADC32V2_DATA_FIFO with DMA32\n", ret);
            return ret;
        }
        break;
    }
    case MesytecMadc32ModuleConfig::vmBLT32:
    {
        //printf("MesytecMadc32ModuleConfig::vmBLT32\n");
        int ret = getInterface ()->readA32BLT32(addr, data, words_to_read, rd);
        if (!*rd && ret && !getInterface ()->isBusError (ret)) {
            printf ("Error %d at MADC32V2_DATA_FIFO with BLT32\n", ret);
            return ret;
        }
        break;
    }
    case MesytecMadc32ModuleConfig::vmBLT64:
    {
        //printf("MesytecMadc32ModuleConfig::vmBLT64\n");
        int ret = getInterface ()->readA32MBLT64(addr, data, words_to_read, rd);
        if (!*rd && ret && !getInterface ()->isBusError (ret)) {
            printf ("Error %d at MADC32V2_DATA_FIFO with MBLT64\n", ret);
            return ret;
        }
        break;
    }
    case MesytecMadc32ModuleConfig::vmSingle:
    {
        //printf("MesytecMadc32ModuleConfig::vmSingle\n");
        int idx = 0;
        for(uint i = 0; i < words_to_read-1; ++i) {
            int ret = getInterface()->readA32D32(addr,&(data[idx]));
            if(ret) {
                printf ("Error %d at MADC32V2_DATA_FIFO with D32\n", ret);
                return ret;
            }
            ++idx;
        }
        (*rd) = words_to_read-1;
        break;
    }
    }

    // Reset readout logic
    readoutReset();

    // Dump the data
//    printf("\nEvent dump:\n");
//    for(int i = 0; i < (*rd); ++i) {
//        printf("<%d> 0x%08x\n",i,data[i]);
//    }
//    printf("\n"); fflush(stdout);

    return 0;
}

void MesytecMadc32Module::singleShot (uint32_t *data, uint32_t *rd) {
    bool triggered = false;

    if(!getInterface()->isOpen()) {
        getInterface()->open();
    }

    reset();
    configure();

    usleep(500);

    fifoReset();
    startAcquisition();
    readoutReset();

    uint tmp_poll_cnt = conf_.pollcount;
    conf_.pollcount = 100000;

    for (unsigned int i = 0; i < conf_.pollcount; ++i) {
        if (dataReady ()) {
            triggered = true;
            break;
        }
    }

    if (!triggered)
        std::cout << "MADC32: No data after " << conf_.pollcount
                  << " trigger loops" << std::endl << std::flush;
    else
        acquireSingle (data, rd);

    conf_.pollcount = tmp_poll_cnt;

    stopAcquisition();

    // Unpack the data

    madc32_header_t header;
    int idx = 0;
    header.data = data[idx++];

    if(header.bits.signature != MADC32V2_SIG_HEADER) {
        printf("Invalid header word signature: %d\n",header.bits.signature);
    }

//    printf("Header info:\t(0x%08x)\n",header.data);
//    printf("ADC resolution: %d\n",header.bits.adc_resolution);
//    printf("Module ID: %d\n",header.bits.module_id);
//    printf("Data length: %d\n",header.bits.data_length);

//    printf("###############\n");

    if(header.bits.data_length != (*rd)-1) {
        printf("Data length mismatch: (header.bits.data_length = %d) (read-1 = %d)\n",
               header.bits.data_length,(*rd)-1);
        return;
    }

    for(int i = 0; i < header.bits.data_length-1; ++i) {
        madc32_data_t datum;
        datum.data = data[idx++];

        if(datum.bits.signature != MADC32V2_SIG_DATA) {
            printf("Invalid data word signature: %d\n",datum.bits.signature);
        }

        if(datum.bits.sub_signature == MADC32V2_SIG_DATA_EVENT) {
//            printf("Energy (%d): %d\t(0x%08x)\n",datum.bits.channel,datum.bits.value,datum.data);
        } else {
            printf("Found word with sub-signature: %d\n",datum.bits.sub_signature);
        }
        current_energy[datum.bits.channel] = datum.bits.value;
    }

    madc32_end_of_event_t trailer;
    trailer.data = data[idx++];

    if(trailer.bits.signature != MADC32V2_SIG_END) {
        printf("Invalid trailer word signature: %d\n",trailer.bits.signature);
    }

//    printf("###############\n");

//    printf("Trailer info:\t(0x%08x)\n",trailer.data);
//    printf("Trigger counter / Time stamp: %d\n",trailer.bits.trigger_counter);

//    printf("###############\n\n");

    current_module_id = header.bits.module_id;
    current_resolution = header.bits.adc_resolution;
    current_time_stamp = trailer.bits.trigger_counter;

    fflush(stdout);
}

int MesytecMadc32Module::updateModuleInfo() {
    uint16_t firmware_from_module = getFirmwareRevision();
    uint16_t module_id_from_module = getModuleIdConfigured();

    Q_UNUSED (firmware_from_module)
    Q_UNUSED (module_id_from_module)

    return 0;
}

typedef ConfMap::confmap_t<MesytecMadc32ModuleConfig> confmap_t;
static const confmap_t confmap [] = {
    confmap_t ("base_addr", &MesytecMadc32ModuleConfig::base_addr),
    confmap_t ("base_addr_register", &MesytecMadc32ModuleConfig::base_addr_register),
    confmap_t ("module_id", &MesytecMadc32ModuleConfig::module_id),
    confmap_t ("firmware_expected", &MesytecMadc32ModuleConfig::firmware_expected),
    confmap_t ("irq_level", &MesytecMadc32ModuleConfig::irq_level),
    confmap_t ("irq_vector", &MesytecMadc32ModuleConfig::irq_vector),
    confmap_t ("irq_threshold", &MesytecMadc32ModuleConfig::irq_threshold),
    confmap_t ("rc_module_id_read", &MesytecMadc32ModuleConfig::rc_module_id_read),
    confmap_t ("rc_module_id_write", &MesytecMadc32ModuleConfig::rc_module_id_write),
    confmap_t ("max_transfer_data", &MesytecMadc32ModuleConfig::max_transfer_data),
    confmap_t ("cblt_mcst_ctrl", &MesytecMadc32ModuleConfig::cblt_mcst_ctrl),
    confmap_t ("cblt_addr", &MesytecMadc32ModuleConfig::cblt_addr),
    confmap_t ("mcst_addr", &MesytecMadc32ModuleConfig::mcst_addr),
    confmap_t ("enable_multi_event_compare_with_max_transfer_data", &MesytecMadc32ModuleConfig::enable_multi_event_compare_with_max_transfer_data),
    confmap_t ("enable_multi_event_send_different_eob_marker", &MesytecMadc32ModuleConfig::enable_multi_event_send_different_eob_marker),
    confmap_t ("enable_adc_override", &MesytecMadc32ModuleConfig::enable_adc_override),
    confmap_t ("enable_switch_off_sliding_scale", &MesytecMadc32ModuleConfig::enable_switch_off_sliding_scale),
    confmap_t ("enable_skip_out_of_range", &MesytecMadc32ModuleConfig::enable_skip_out_of_range),
    confmap_t ("enable_ignore_thresholds", &MesytecMadc32ModuleConfig::enable_ignore_thresholds),
    confmap_t ("enable_termination_input_gate0", &MesytecMadc32ModuleConfig::enable_termination_input_gate0),
    confmap_t ("enable_termination_input_fast_clear", &MesytecMadc32ModuleConfig::enable_termination_input_fast_clear),
    confmap_t ("time_stamp_divisor", &MesytecMadc32ModuleConfig::time_stamp_divisor),
    confmap_t ("mcst_cblt_none", &MesytecMadc32ModuleConfig::mcst_cblt_none),
    confmap_t ("enable_cblt_mode", &MesytecMadc32ModuleConfig::enable_cblt_mode),
    confmap_t ("enable_mcst_mode", &MesytecMadc32ModuleConfig::enable_mcst_mode),
    confmap_t ("enable_cblt_first", &MesytecMadc32ModuleConfig::enable_cblt_first),
    confmap_t ("enable_cblt_last", &MesytecMadc32ModuleConfig::enable_cblt_last),
    confmap_t ("enable_cblt_middle", &MesytecMadc32ModuleConfig::enable_cblt_middle),
    confmap_t ("pollcount", &MesytecMadc32ModuleConfig::pollcount),
    confmap_t ("vme_mode", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::vme_mode),
    confmap_t ("addr_source", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::addr_source),
    confmap_t ("data_length_format", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::data_length_format),
    confmap_t ("multi_event_mode", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::multi_event_mode),
    confmap_t ("marking_type", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::marking_type),
    confmap_t ("bank_operation", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::bank_operation),
    confmap_t ("adc_resolution", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::adc_resolution),
    confmap_t ("output_format", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::output_format),
    confmap_t ("gate_generator_mode", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::gate_generator_mode),
    confmap_t ("input_range", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::input_range),
    confmap_t ("ecl_gate1_mode", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::ecl_gate1_mode),
    confmap_t ("ecl_fclear_mode", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::ecl_fclear_mode),
    confmap_t ("ecl_busy_mode", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::ecl_busy_mode),
    confmap_t ("nim_gate1_mode", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::nim_gate1_mode),
    confmap_t ("nim_fclear_mode", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::nim_fclear_mode),
    confmap_t ("nim_busy_mode", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::nim_busy_mode),
    confmap_t ("test_pulser_mode", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::test_pulser_mode),
    confmap_t ("time_stamp_source", (uint16_t MesytecMadc32ModuleConfig::*)
                &MesytecMadc32ModuleConfig::time_stamp_source),
};

void MesytecMadc32Module::applySettings (QSettings *settings) {
    std::cout << "Applying settings for " << getName ().toStdString () << "... ";
    settings->beginGroup (getName ());

    ConfMap::apply (settings, &conf_, confmap);

    for (int i = 0; i < MADC32V2_NUM_CHANNELS; ++i) {
        QString key = QString ("thresholds%1").arg (i);
        if (settings->contains (key)) {
            conf_.thresholds[i] = settings->value (key).toUInt ();
            //printf("Found key %s with value %d\n",key.toStdString().c_str(),conf_.thresholds[i]);
        }
        key = QString ("enable_channel%1").arg (i);
        if (settings->contains (key))
            conf_.enable_channel[i] = settings->value (key).toBool ();
    }
    for (int i = 0; i < 2; ++i) {
        QString key = QString ("hold_delay_%1").arg (i);
        if (settings->contains (key)) {
            conf_.hold_delay[i] = settings->value (key).toUInt ();
            //printf("Found key %s with value %d\n",key.toStdString().c_str(),conf_.thresholds[i]);
        }
        key = QString ("hold_width_%1").arg (i);
        if (settings->contains (key)) {
            conf_.hold_width[i] = settings->value (key).toUInt ();
            //printf("Found key %s with value %d\n",key.toStdString().c_str(),conf_.thresholds[i]);
        }
    }

    settings->endGroup ();
    std::cout << "done" << std::endl;

    if(getUI()) getUI ()->applySettings ();
}

void MesytecMadc32Module::saveSettings (QSettings *settings) {
    std::cout << "Saving settings for " << getName ().toStdString () << "... ";
    settings->beginGroup (getName ());
    ConfMap::save (settings, &conf_, confmap);
    for (int i = 0; i < MADC32V2_NUM_CHANNELS; ++i) {
        QString key = QString ("thresholds%1").arg (i);
        settings->setValue (key, conf_.thresholds [i]);
        key = QString ("enable_channel%1").arg (i);
        settings->setValue (key, conf_.enable_channel [i]);
    }
    for (int i = 0; i < 2; ++i) {
        QString key = QString ("hold_delay_%1").arg (i);
        settings->setValue (key, conf_.hold_delay [i]);
        key = QString ("hold_width_%1").arg (i);
        settings->setValue (key, conf_.hold_width [i]);
    }
    settings->endGroup ();
    std::cout << "done" << std::endl;
}

void MesytecMadc32Module::setBaseAddress (uint32_t baddr) {
    conf_.base_addr = baddr;
    if(getUI()) getUI ()->applySettings ();
}

uint32_t MesytecMadc32Module::getBaseAddress () const {
    return conf_.base_addr;
}
/*!
\page Mesytec MADC-32 module
<b>Module name:</b> \c mesytecMadc32

\section desc Module Description
The Mesytec MADC-32 module is a 32 channel ADC.

\section cpanel Configuration Panel

\subsection settings Settings

\subsection irq IRQ
The IRQ panel controls the conditions on which VME interrupt requests are generated. IRQs are not yet used in data readout.

\subsection info Info
The Info panel provides some basic information about the firmware that runs on the module.
This page is not fully implemented yet.

\section outs Outputs

*/

