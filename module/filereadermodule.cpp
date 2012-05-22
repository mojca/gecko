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

#include "filereadermodule.h"
#include "filereaderui.h"
#include "modulemanager.h"
#include "runmanager.h"
#include "confmap.h"

#include <cstdio>
#include <cstring>
#include <unistd.h> // usleep()

using namespace std;
static ModuleRegistrar reg1 ("fileReader", FileReaderModule::create);

FileReaderModule::FileReaderModule (int i, const QString &n)
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
    std::cout << "Before starting FileReader module" << std::endl;
    setChannels ();
    std::cout << "After setting channels in FileReader module" << std::endl;
    createOutputPlugin();
    std::cout << "After createOutputPlugin in FileReader module" << std::endl;

    setUI (new FileReaderUI (this));
    std::cout << "Instantiated FileReader module" << std::endl;
}

void FileReaderModule::setChannels () {
    // EventBuffer *evbuf = RunManager::ref ().getEventBuffer ();
    // 
    // // Per channel outputs
    // for(int i = 0; i < MADC32V2_NUM_CHANNELS; i++)
    //     evslots_ << evbuf->registerSlot (this, tr("out %1").arg(i,1,10), PluginConnector::VectorUint32);
    // 
    // // Output for raw data -> to event builder
    // evslots_ << evbuf->registerSlot(this, "raw out", PluginConnector::VectorUint32);
}

int FileReaderModule::configure () {
    // AbstractInterface *iface = getInterface ();
    // 
    // uint32_t baddr = conf_.base_addr;
    // uint16_t data;
    int ret = 0;
    // 
    // if(!iface) return 2;
    // if(!iface->isOpen()) return 1;
    // 
    // // set address source
    // ret = iface->writeA32D16 (baddr + MADC32V2_ADDR_SOURCE, conf_.addr_source);
    // if (ret) printf ("Error %d at MADC32V2_ADDR_SOURCE", ret);
    // 
    // // set address register
    // ret = iface->writeA32D16(baddr + MADC32V2_ADDR_REGISTER, conf_.base_addr_register);
    // if (ret) printf ("Error %d at MADC32V2_ADDR_REGISTER", ret);
    // 
    // // set module id
    // ret = iface->writeA32D16(baddr + MADC32V2_MODULE_ID, conf_.module_id);
    // if (ret) printf ("Error %d at MADC32V2_MODULE_ID", ret);
    // 
    // // set irq level
    // ret = iface->writeA32D16(baddr + MADC32V2_IRQ_LEVEL, conf_.irq_level);
    // if (ret) printf ("Error %d at MADC32V2_", ret);
    // 
    // // set irq vector
    // ret = iface->writeA32D16(baddr + MADC32V2_IRQ_VECTOR, conf_.irq_vector);
    // if (ret) printf ("Error %d at MADC32V2_IRQ_VECTOR", ret);
    // 
    // // set irq threshold
    // ret = iface->writeA32D16(baddr + MADC32V2_IRQ_THRESHOLD, conf_.irq_threshold);
    // if (ret) printf ("Error %d at MADC32V2_IRQ_THRESHOLD", ret);
    // 
    // // set max transfer data
    // ret = iface->writeA32D16(baddr + MADC32V2_MAX_TRANSFER_DATA, conf_.max_transfer_data);
    // if (ret) printf ("Error %d at MADC32V2_MAX_TRANSFER_DATA", ret);
    // 
    // // set cblt address
    // ret = iface->writeA32D16(baddr + MADC32V2_CBLT_ADDRESS,
    //                          (conf_.cblt_addr << MADC32V2_OFF_CBLT_ADDRESS));
    // if (ret) printf ("Error %d at MADC32V2_CBLT_ADDRESS", ret);
    // 
    // // set mcst address
    // ret = iface->writeA32D16(baddr + MADC32V2_MCST_ADDRESS,
    //                          (conf_.mcst_addr << MADC32V2_OFF_MCST_ADDRESS));
    // if (ret) printf ("Error %d at MADC32V2_MCST_ADDRESS", ret);
    // 
    // // set data length format
    // ret = iface->writeA32D16(baddr + MADC32V2_DATA_LENGTH_FORMAT, conf_.data_length_format);
    // if (ret) printf ("Error %d at MADC32V2_DATA_LENGTH_FORMAT", ret);
    // 
    // // set multi event mode
    // data = conf_.multi_event_mode;
    // if(conf_.enable_multi_event_compare_with_max_transfer_data)
    //     data |= MADC32V2_VAL_MULTIEVENT_MODE_MAX_DATA;
    // if(conf_.enable_multi_event_send_different_eob_marker)
    //     data |= MADC32V2_VAL_MULTIEVENT_MODE_EOB_BERR;
    // ret = iface->writeA32D16(baddr + MADC32V2_MULTIEVENT_MODE, data);
    // if (ret) printf ("Error %d at MADC32V2_MULTIEVENT_MODE", ret);
    // 
    // // set marking type
    // ret = iface->writeA32D16(baddr + MADC32V2_MARKING_TYPE, conf_.marking_type);
    // if (ret) printf ("Error %d at MADC32V2_MARKING_TYPE", ret);
    // 
    // // set bank mode
    // ret = iface->writeA32D16(baddr + MADC32V2_BANK_MODE, conf_.bank_operation);
    // if (ret) printf ("Error %d at MADC32V2_BANK_MODE", ret);
    // 
    // // set adc resolution
    // ret = iface->writeA32D16(baddr + MADC32V2_ADC_RESOLUTION, conf_.adc_resolution);
    // if (ret) printf ("Error %d at MADC32V2_ADC_RESOLUTION", ret);
    // 
    // // set output format (not necessary at the moment)
    // ret = iface->writeA32D16(baddr + MADC32V2_OUTPUT_FORMAT, conf_.output_format);
    // if (ret) printf ("Error %d at MADC32V2_OUTPUT_FORMAT", ret);
    // 
    // // set adc override
    // ret = iface->writeA32D16(baddr + MADC32V2_ADC_OVERRIDE,
    //                          (conf_.enable_adc_override ?
    //                               conf_.adc_override_resolution : conf_.adc_resolution));
    // if (ret) printf ("Error %d at MADC32V2_ADC_OVERRIDE", ret);
    // 
    // // set sliding scale disable
    // ret = iface->writeA32D16(baddr + MADC32V2_SLIDING_SCALE_OFF,
    //                          (conf_.enable_switch_off_sliding_scale ? 1 : 0));
    // if (ret) printf ("Error %d at MADC32V2_SLIDING_SCALE_OFF", ret);
    // 
    // // set skip out of range
    // ret = iface->writeA32D16(baddr + MADC32V2_SKIP_OUT_OF_RANGE,
    //                          (conf_.enable_skip_out_of_range ? 1 : 0));
    // if (ret) printf ("Error %d at MADC32V2_SKIP_OUT_OF_RANGE", ret);
    // 
    // // set ignore thresholds
    // ret = iface->writeA32D16(baddr + MADC32V2_IGNORE_THRESHOLDS,
    //                          (conf_.enable_ignore_thresholds ? 1 : 0));
    // if (ret) printf ("Error %d at MADC32V2_IGNORE_THRESHOLDS", ret);
    // 
    // // set hold delay 0
    // ret = iface->writeA32D16(baddr + MADC32V2_HOLD_DELAY_0, conf_.hold_delay[0]);
    // if (ret) printf ("Error %d at MADC32V2_HOLD_DELAY_0", ret);
    // 
    // // set hold delay 1
    // ret = iface->writeA32D16(baddr + MADC32V2_HOLD_DELAY_1, conf_.hold_delay[1]);
    // if (ret) printf ("Error %d at MADC32V2_HOLD_DELAY_1", ret);
    // 
    // // set hold width 0
    // ret = iface->writeA32D16(baddr + MADC32V2_HOLD_WIDTH_0, conf_.hold_width[0]);
    // if (ret) printf ("Error %d at MADC32V2_HOLD_WIDTH_0", ret);
    // 
    // // set hold width 1
    // ret = iface->writeA32D16(baddr + MADC32V2_HOLD_WIDTH_1, conf_.hold_width[1]);
    // if (ret) printf ("Error %d at MADC32V2_HOLD_WIDTH_1", ret);
    // 
    // // set gate generator mode
    // ret = iface->writeA32D16(baddr + MADC32V2_USE_GATE_GENERATOR, conf_.gate_generator_mode);
    // if (ret) printf ("Error %d at MADC32V2_USE_GATE_GENERATOR", ret);
    // 
    // // set input range
    // ret = iface->writeA32D16(baddr + MADC32V2_INPUT_RANGE, conf_.input_range);
    // if (ret) printf ("Error %d at MADC32V2_INPUT_RANGE", ret);
    // 
    // // set ecl termination
    // data = 0;
    // if(conf_.enable_termination_input_gate0) data |= (1 << MADC32V2_OFF_ECL_TERMINATION_GATE_0);
    // if(conf_.enable_termination_input_fast_clear) data |= (1 << MADC32V2_OFF_ECL_TERMINATION_FCLEAR);
    // ret = iface->writeA32D16(baddr + MADC32V2_ECL_TERMINATED, data);
    // if (ret) printf ("Error %d at MADC32V2_ECL_TERMINATED", ret);
    // 
    // // set ecl gate 1 mode
    // ret = iface->writeA32D16(baddr + MADC32V2_ECL_GATE1_OSC, conf_.ecl_gate1_mode);
    // if (ret) printf ("Error %d at MADC32V2_ECL_GATE1_OSC", ret);
    // 
    // // set ecl fast clear mode
    // ret = iface->writeA32D16(baddr + MADC32V2_ECL_FAST_CLEAR_RST, conf_.ecl_fclear_mode);
    // if (ret) printf ("Error %d at MADC32V2_ECL_FAST_CLEAR_RST", ret);
    // 
    // // set ecl busy mode
    // ret = iface->writeA32D16(baddr + MADC32V2_ECL_BUSY, conf_.ecl_busy_mode);
    // if (ret) printf ("Error %d at MADC32V2_ECL_BUSY", ret);
    // 
    // // set nim gate 1 mode
    // ret = iface->writeA32D16(baddr + MADC32V2_NIM_GATE1_OSC, conf_.nim_gate1_mode);
    // if (ret) printf ("Error %d at MADC32V2_NIM_GATE1_OSC", ret);
    // 
    // // set nim fast clear mode
    // ret = iface->writeA32D16(baddr + MADC32V2_NIM_FAST_CLEAR_RST, conf_.nim_fclear_mode);
    // if (ret) printf ("Error %d at MADC32V2_NIM_FAST_CLEAR_RST", ret);
    // 
    // // set nim busy mode
    // ret = iface->writeA32D16(baddr + MADC32V2_NIM_BUSY, conf_.nim_busy_mode);
    // if (ret) printf ("Error %d at MADC32V2_NIM_BUSY", ret);
    // 
    // // set pulser status
    // ret = iface->writeA32D16(baddr + MADC32V2_PULSER_STATUS, conf_.test_pulser_mode);
    // if (ret) printf ("Error %d at MADC32V2_PULSER_STATUS", ret);
    // 
    // // set template
    // //ret = iface->writeA32D16(baddr + MADC32V2_, conf_.);
    // //if (ret) printf ("Error %d at MADC32V2_", ret);
    // 
    // // set channel thresholds
    // for (int i = 0; i < MADC32V2_NUM_CHANNELS; ++i) {
    //     uint16_t thr = (conf_.enable_channel[i] ?
    //                         conf_.thresholds[i] :
    //                         MADC32V2_VAL_THRESHOLD_SWITCH_OFF);
    //     ret = iface->writeA32D16 (baddr + MADC32V2_THRESHOLD_MEM + 2*i,thr);
    //     if (ret) printf ("Error %d at MADC32V2_THRESHOLD_MEM[%d]\n", ret, i);
    // }
    // 
    // //REG_DUMP();
    // 
    // ret = counterResetAll ();
    return ret;
}

void FileReaderModule::REG_DUMP() {
    // int nof_reg_groups = 2;
    // uint32_t start_addr[] = {0x00004000, 0x00006000};
    // uint32_t end_addr[]   = {0x0000403e, 0x000060ae};
    // uint16_t data;
    // 
    // printf("FileReaderModule::REG_DUMP:\n");
    // for(int i = 0; i < nof_reg_groups; ++i) {
    //     for(uint32_t addr = conf_.base_addr + start_addr[i]; addr < conf_.base_addr + end_addr[i]; addr += 2) {
    //         if(getInterface()->readA32D16(addr,&data) == 0) {
    //             printf("0x%08x: 0x%04x\n",addr,data);
    //         }
    //     }
    //     printf("*\n");
    // }
    // fflush(stdout);
}

int FileReaderModule::counterResetAll () {
    // return counterResetAB(MADC32V2_VAL_RST_COUNTER_AB_ALL);
    return 0;
}

int FileReaderModule::counterResetAB(uint8_t counter) {
    // return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_RESET_COUNTER_AB,counter);
    return 0;
}

int FileReaderModule::irqReset() {
    // return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_IRQ_RESET, 1);
    return 0;
}

int FileReaderModule::irqTest() {
    // return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_IRQ_TEST, 1);
    return 0;
}

int FileReaderModule::irqWithdraw() {
    // return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_IRQ_WITHDRAW, 1);
    return 0;
}

int FileReaderModule::readoutReset () {
    // return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_READOUT_RESET, 1);
    return 0;
}

int FileReaderModule::startAcquisition() {
    // return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_START_ACQUISITION, 1);
    return 0;
}

int FileReaderModule::stopAcquisition() {
    // return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_START_ACQUISITION, 0);
    return 0;
}

int FileReaderModule::fifoReset () {
    // return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_FIFO_RESET, 1);
    return 0;
}

int FileReaderModule::stopCounter (uint8_t counter) {
    // return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_STOP_COUNTER, counter);
    return 0;
}

// Warning: This reset also resets parameters! Do not issue after configuration!
int FileReaderModule::softReset () {
    // return getInterface()->writeA32D16(conf_.base_addr + MADC32V2_SOFT_RESET, 1);
    return 0;
}

int FileReaderModule::reset () {
    // irqReset();
    // counterResetAll();
    // fifoReset();
    // readoutReset();
    // //dataReset();
    // return softReset ();
    return 0;
}

uint16_t FileReaderModule::getBufferDataLength() const {
        uint16_t data = 0;
        // getInterface ()->readA32D16 (conf_.base_addr + MADC32V2_BUFFER_DATA_LENGTH, &data);
        return data;
}

uint16_t FileReaderModule::getFirmwareRevision() {
        // if (!firmware)
        // getInterface ()->readA32D16 (conf_.base_addr + MADC32V2_FIRMWARE_REVISION, &firmware);
        // conf_.firmware_revision_minor = firmware & 0xff;
        // conf_.firmware_revision_major = (firmware >> 16) & 0xff;
        // return firmware;
    return 0;
}

uint16_t FileReaderModule::getModuleIdConfigured() {
        // if (!module_id)
        // getInterface ()->readA32D16 (conf_.base_addr + MADC32V2_MODULE_ID, &module_id);
        // return module_id;
    return 0;
}

bool FileReaderModule::checkFirmware()  {
    // if(!firmware) getFirmwareRevision();
    // if(firmware != conf_.firmware_expected) {
    //     printf("FileReaderModule::checkFirmware : Firmware mismatch (0x%04x, expected: 0x%04x)\n",
    //            firmware,conf_.firmware_expected);
    //     return false;
    // }
    return true;
}

int FileReaderModule::getAllCounters() {
    // event_counter = getEventCounter();
    // timestamp_counter = getTimestampCounter();
    // adc_busy_counter = getAdcBusyTime();
    // gate1_time_counter = getGate1Time();
    // time_counter = getTime();
    return 0;
}

uint32_t FileReaderModule::getEventCounter() {
    int ret = 0;
    uint32_t data32;
    uint16_t data;

    // AbstractInterface *iface = getInterface ();
    // 
    // ret = iface->readA32D16 (conf_.base_addr + MADC32V2_EVENT_COUNTER_LOW, &data);
    // if (ret) printf ("Error %d at MADC32V2_EVENT_COUNTER_LOW\n", ret);
    // data32 = data;
    // 
    // ret = iface->readA32D16 (conf_.base_addr + MADC32V2_EVENT_COUNTER_HIGH, &data);
    // if (ret) printf ("Error %d at MADC32V2_EVENT_COUNTER_HIGH\n", ret);
    // data32 |= (((uint32_t)(data) & 0x0000ffff) << 16);
    // 
    // printf("Event counter: %d\n",data32);

    return data32;
}

uint32_t FileReaderModule::getTimestampCounter() {
    int ret = 0;
    uint32_t data32;
    uint16_t data;

    // AbstractInterface *iface = getInterface ();
    // 
    // ret = iface->readA32D16 (conf_.base_addr + MADC32V2_TIMESTAMP_CNT_L, &data);
    // if (ret) printf ("Error %d at MADC32V2_TIMESTAMP_CNT_L", ret);
    // data32 = data;
    // 
    // ret = iface->readA32D16 (conf_.base_addr + MADC32V2_TIMESTAMP_CNT_H, &data);
    // if (ret) printf ("Error %d at MADC32V2_TIMESTAMP_CNT_H", ret);
    // data32 |= (((uint32_t)(data) & 0x0000ffff) << 16);

    return data32;
}

uint32_t FileReaderModule::getAdcBusyTime() {
    int ret = 0;
    uint32_t data32;
    uint16_t data;

    // AbstractInterface *iface = getInterface ();
    // 
    // ret = iface->readA32D16 (conf_.base_addr + MADC32V2_ADC_BUSY_TIME_LOW, &data);
    // if (ret) printf ("Error %d at MADC32V2_ADC_BUSY_TIME_LOW", ret);
    // data32 = data;
    // 
    // ret = iface->readA32D16 (conf_.base_addr + MADC32V2_ADC_BUSY_TIME_HIGH, &data);
    // if (ret) printf ("Error %d at MADC32V2_ADC_BUSY_TIME_HIGH", ret);
    // data32 |= (((uint32_t)(data) & 0x0000ffff) << 16);

    return data32;
}

uint32_t FileReaderModule::getGate1Time() {
    int ret = 0;
    uint32_t data32;
    uint16_t data;

    // AbstractInterface *iface = getInterface ();
    // 
    // ret = iface->readA32D16 (conf_.base_addr + MADC32V2_GATE_1_TIME_LOW, &data);
    // if (ret) printf ("Error %d at MADC32V2_GATE_1_TIME_LOW", ret);
    // data32 = data;
    // 
    // ret = iface->readA32D16 (conf_.base_addr + MADC32V2_GATE_1_TIME_HIGH, &data);
    // if (ret) printf ("Error %d at MADC32V2_GATE_1_TIME_HIGH", ret);
    // data32 |= (((uint32_t)(data) & 0x0000ffff) << 16);

    return data32;
}

uint64_t FileReaderModule::getTime() {
    int ret = 0;
    uint32_t data64 = 0LL;
    uint16_t data;

    // AbstractInterface *iface = getInterface ();
    // 
    // ret = iface->readA32D16 (conf_.base_addr + MADC32V2_TIME_0, &data);
    // if (ret) printf ("Error %d at MADC32V2_TIME_0", ret);
    // data64 = data;
    // 
    // ret = iface->readA32D16 (conf_.base_addr + MADC32V2_TIME_1, &data);
    // if (ret) printf ("Error %d at MADC32V2_TIME_1", ret);
    // data64 |= (((uint64_t)(data) & 0xffffLL) << 16);
    // 
    // ret = iface->readA32D16 (conf_.base_addr + MADC32V2_TIME_2, &data);
    // if (ret) printf ("Error %d at MADC32V2_TIME_2", ret);
    // data64 |= (((uint64_t)(data) & 0xffffLL) << 32);

    return data64;
}

bool FileReaderModule::getDataReady() {
    int ret;
    uint16_t data;
    // ret = getInterface ()->readA32D16 (conf_.base_addr + MADC32V2_DATA_READY, &data);
    // if (ret) printf ("Error %d at MADC32V2_DATA_READY", ret);

    return (data == 1);
}

bool FileReaderModule::dataReady () {
    //return getDataReady();
    return (getBufferDataLength() > 0);
}

int FileReaderModule::acquire (Event* ev) {
    // int ret;
    // 
    // ret = acquireSingle (data, &buffer_data_length);
    // if (ret == 0) writeToBuffer(ev);
    // else printf("FileReaderModule::Error at acquireSingle\n");
    // 
    // return buffer_data_length;
    return 0;
}

void FileReaderModule::writeToBuffer(Event *ev)
{
    // bool go_on = dmx_.processData (ev, data, buffer_data_length, RunManager::ref ().isSingleEventMode ());
    // if (!go_on) {
    //     // Do what has to be done to finish this acquisition cycle
    //     // fifoReset();
    // }
}

int FileReaderModule::acquireSingle (uint32_t *data, uint32_t *rd) {
//     *rd = 0;
// 
//     // Get buffer data length
//     uint32_t words_to_read = 0;
//     buffer_data_length = getBufferDataLength();
//     //printf("madc32: Event length (buffer_data_length): %d\n",buffer_data_length);
// 
//     // Translate buffer data length to number of words to read
//     switch(conf_.data_length_format) {
//     case FileReaderModuleConfig::dl8bit:
//         words_to_read = buffer_data_length / 4;
//         break;
//     case FileReaderModuleConfig::dl16bit:
//         words_to_read = buffer_data_length / 2;
//         break;
//     case FileReaderModuleConfig::dl64bit:
//         words_to_read = buffer_data_length * 2;
//         break;
//     case FileReaderModuleConfig::dl32bit:
//     default:
//         words_to_read = buffer_data_length;
//     }
// 
//     ++words_to_read;
//     //printf("madc32: Words to read: %d\n",words_to_read);
// 
//     // Read the data fifo
//     uint32_t addr = conf_.base_addr + MADC32V2_DATA_FIFO;
//     //printf("madc32: acquireSingle from addr = 0x%08x\n",addr);
// 
//     switch(conf_.vme_mode) {
// 
//     case FileReaderModuleConfig::vmFIFO:
//     {
//         //printf("FileReaderModuleConfig::vmFIFO\n");
//         int ret = getInterface ()->readA32FIFO(addr, data, words_to_read, rd);
//         if (!*rd && ret && !getInterface ()->isBusError (ret)) {
//             printf ("Error %d at MADC32V2_DATA_FIFO with FIFO read\n", ret);
//             return ret;
//         }
//         break;
//     }
//     case FileReaderModuleConfig::vmDMA32:
//     {
//         //printf("FileReaderModuleConfig::vmDMA32\n");
//         int ret = getInterface ()->readA32DMA32(addr, data, words_to_read, rd);
//         if (!*rd && ret && !getInterface ()->isBusError (ret)) {
//             printf ("Error %d at MADC32V2_DATA_FIFO with DMA32\n", ret);
//             return ret;
//         }
//         break;
//     }
//     case FileReaderModuleConfig::vmBLT32:
//     {
//         //printf("FileReaderModuleConfig::vmBLT32\n");
//         int ret = getInterface ()->readA32BLT32(addr, data, words_to_read, rd);
//         if (!*rd && ret && !getInterface ()->isBusError (ret)) {
//             printf ("Error %d at MADC32V2_DATA_FIFO with BLT32\n", ret);
//             return ret;
//         }
//         break;
//     }
//     case FileReaderModuleConfig::vmBLT64:
//     {
//         //printf("FileReaderModuleConfig::vmBLT64\n");
//         int ret = getInterface ()->readA32MBLT64(addr, data, words_to_read, rd);
//         if (!*rd && ret && !getInterface ()->isBusError (ret)) {
//             printf ("Error %d at MADC32V2_DATA_FIFO with MBLT64\n", ret);
//             return ret;
//         }
//         break;
//     }
//     case FileReaderModuleConfig::vm2ESST: // Not handled by the module
//     case FileReaderModuleConfig::vmSingle:
//     {
//         //printf("FileReaderModuleConfig::vmSingle\n");
//         int idx = 0;
//         for(uint i = 0; i < words_to_read-1; ++i) {
//             int ret = getInterface()->readA32D32(addr,&(data[idx]));
//             if(ret) {
//                 printf ("Error %d at MADC32V2_DATA_FIFO with D32\n", ret);
//                 return ret;
//             }
//             ++idx;
//         }
//         (*rd) = words_to_read-1;
//         break;
//     }
//     }
// 
// 
//     // Reset readout logic
// //    int ret = fifoReset();
// //    if(ret) {
// //        printf ("Error %d at MADC32V2_FIFO_RESET with D32\n", ret);
// //    }
//     //usleep(1000);
//     //int ret = readoutReset();
//     //usleep(1000);
//     int ret = readoutReset();
//     if(ret) {
//         printf ("Error %d at MADC32V2_READOUT_RESET with D32\n", ret);
//     }
// 
//     // Dump the data
// //    printf("\nEvent dump:\n");
// //    for(int i = 0; i < (*rd); ++i) {
// //        printf("<%d> 0x%08x\n",i,data[i]);
// //    }
// //    printf("\n"); fflush(stdout);
// 
    return 0;
}

void FileReaderModule::singleShot (uint32_t *data, uint32_t *rd) {
//     bool triggered = false;
// 
//     if(!getInterface()->isOpen()) {
//         getInterface()->open();
//     }
// 
//     reset();
//     configure();
// 
//     usleep(500);
// 
//     fifoReset();
//     startAcquisition();
//     readoutReset();
// 
//     uint tmp_poll_cnt = conf_.pollcount;
//     conf_.pollcount = 100000;
// 
//     for (unsigned int i = 0; i < conf_.pollcount; ++i) {
//         if (dataReady ()) {
//             triggered = true;
//             break;
//         }
//     }
// 
//     if (!triggered)
//         std::cout << "MADC32: No data after " << conf_.pollcount
//                   << " trigger loops" << std::endl << std::flush;
//     else
//         acquireSingle (data, rd);
// 
//     conf_.pollcount = tmp_poll_cnt;
// 
//     stopAcquisition();
// 
//     // Unpack the data
// 
//     madc32_header_t header;
//     int idx = 0;
//     header.data = data[idx++];
// 
//     if(header.bits.signature != MADC32V2_SIG_HEADER) {
//         printf("Invalid header word signature: %d\n",header.bits.signature);
//     }
// 
// //    printf("Header info:\t(0x%08x)\n",header.data);
// //    printf("ADC resolution: %d\n",header.bits.adc_resolution);
// //    printf("Module ID: %d\n",header.bits.module_id);
// //    printf("Data length: %d\n",header.bits.data_length);
// 
// //    printf("###############\n");
// 
//     if(header.bits.data_length != (*rd)-1) {
//         printf("Data length mismatch: (header.bits.data_length = %d) (read-1 = %d)\n",
//                header.bits.data_length,(*rd)-1);
//         return;
//     }
// 
//     for(int i = 0; i < MADC32V2_NUM_CHANNELS; ++i) {
//         current_energy[i] = 0;
//     }
// 
//     for(int i = 0; i < header.bits.data_length-1; ++i) {
//         madc32_data_t datum;
//         datum.data = data[idx++];
// 
//         if(datum.bits.signature != MADC32V2_SIG_DATA) {
//             printf("Invalid data word signature: %d\n",datum.bits.signature);
//         }
// 
//         if(datum.bits.sub_signature == MADC32V2_SIG_DATA_EVENT) {
// //            printf("Energy (%d): %d\t(0x%08x)\n",datum.bits.channel,datum.bits.value,datum.data);
//         } else {
//             //printf("Found word with sub-signature: %d\n",datum.bits.sub_signature);
//         }
//         current_energy[datum.bits.channel] = datum.bits.value;
//     }
// 
//     madc32_end_of_event_t trailer;
//     trailer.data = data[idx++];
// 
//     if(trailer.bits.signature != MADC32V2_SIG_END) {
//         printf("Invalid trailer word signature: %d\n",trailer.bits.signature);
//     }
// 
// //    printf("###############\n");
// 
// //    printf("Trailer info:\t(0x%08x)\n",trailer.data);
// //    printf("Trigger counter / Time stamp: %d\n",trailer.bits.trigger_counter);
// 
// //    printf("###############\n\n");
// 
//     current_module_id = header.bits.module_id;
//     current_resolution = header.bits.adc_resolution;
//     current_time_stamp = trailer.bits.trigger_counter;
// 
//     fflush(stdout);
}

int FileReaderModule::updateModuleInfo() {
    // uint16_t firmware_from_module = getFirmwareRevision();
    // uint16_t module_id_from_module = getModuleIdConfigured();
    // 
    // Q_UNUSED (firmware_from_module)
    // Q_UNUSED (module_id_from_module)

    return 0;
}

typedef ConfMap::confmap_t<FileReaderModuleConfig> confmap_t;
static const confmap_t confmap [] = {
    // confmap_t ("base_addr", &FileReaderModuleConfig::base_addr),
    // confmap_t ("base_addr_register", &FileReaderModuleConfig::base_addr_register),
    // confmap_t ("module_id", &FileReaderModuleConfig::module_id),
    // confmap_t ("firmware_expected", &FileReaderModuleConfig::firmware_expected),
    // confmap_t ("irq_level", &FileReaderModuleConfig::irq_level),
    // confmap_t ("irq_vector", &FileReaderModuleConfig::irq_vector),
    // confmap_t ("irq_threshold", &FileReaderModuleConfig::irq_threshold),
    // confmap_t ("rc_module_id_read", &FileReaderModuleConfig::rc_module_id_read),
    // confmap_t ("rc_module_id_write", &FileReaderModuleConfig::rc_module_id_write),
    // confmap_t ("max_transfer_data", &FileReaderModuleConfig::max_transfer_data),
    // confmap_t ("cblt_mcst_ctrl", &FileReaderModuleConfig::cblt_mcst_ctrl),
    // confmap_t ("cblt_addr", &FileReaderModuleConfig::cblt_addr),
    // confmap_t ("mcst_addr", &FileReaderModuleConfig::mcst_addr),
    // confmap_t ("enable_multi_event_compare_with_max_transfer_data", &FileReaderModuleConfig::enable_multi_event_compare_with_max_transfer_data),
    // confmap_t ("enable_multi_event_send_different_eob_marker", &FileReaderModuleConfig::enable_multi_event_send_different_eob_marker),
    // confmap_t ("enable_adc_override", &FileReaderModuleConfig::enable_adc_override),
    // confmap_t ("enable_switch_off_sliding_scale", &FileReaderModuleConfig::enable_switch_off_sliding_scale),
    // confmap_t ("enable_skip_out_of_range", &FileReaderModuleConfig::enable_skip_out_of_range),
    // confmap_t ("enable_ignore_thresholds", &FileReaderModuleConfig::enable_ignore_thresholds),
    // confmap_t ("enable_termination_input_gate0", &FileReaderModuleConfig::enable_termination_input_gate0),
    // confmap_t ("enable_termination_input_fast_clear", &FileReaderModuleConfig::enable_termination_input_fast_clear),
    // confmap_t ("time_stamp_divisor", &FileReaderModuleConfig::time_stamp_divisor),
    // confmap_t ("mcst_cblt_none", &FileReaderModuleConfig::mcst_cblt_none),
    // confmap_t ("enable_cblt_mode", &FileReaderModuleConfig::enable_cblt_mode),
    // confmap_t ("enable_mcst_mode", &FileReaderModuleConfig::enable_mcst_mode),
    // confmap_t ("enable_cblt_first", &FileReaderModuleConfig::enable_cblt_first),
    // confmap_t ("enable_cblt_last", &FileReaderModuleConfig::enable_cblt_last),
    // confmap_t ("enable_cblt_middle", &FileReaderModuleConfig::enable_cblt_middle),
    // confmap_t ("pollcount", &FileReaderModuleConfig::pollcount),
    // confmap_t ("vme_mode", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::vme_mode),
    // confmap_t ("addr_source", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::addr_source),
    // confmap_t ("data_length_format", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::data_length_format),
    // confmap_t ("multi_event_mode", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::multi_event_mode),
    // confmap_t ("marking_type", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::marking_type),
    // confmap_t ("bank_operation", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::bank_operation),
    // confmap_t ("adc_resolution", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::adc_resolution),
    // confmap_t ("output_format", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::output_format),
    // confmap_t ("gate_generator_mode", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::gate_generator_mode),
    // confmap_t ("input_range", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::input_range),
    // confmap_t ("ecl_gate1_mode", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::ecl_gate1_mode),
    // confmap_t ("ecl_fclear_mode", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::ecl_fclear_mode),
    // confmap_t ("ecl_busy_mode", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::ecl_busy_mode),
    // confmap_t ("nim_gate1_mode", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::nim_gate1_mode),
    // confmap_t ("nim_fclear_mode", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::nim_fclear_mode),
    // confmap_t ("nim_busy_mode", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::nim_busy_mode),
    // confmap_t ("test_pulser_mode", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::test_pulser_mode),
    // confmap_t ("time_stamp_source", (uint16_t FileReaderModuleConfig::*)
    //             &FileReaderModuleConfig::time_stamp_source),
};

void FileReaderModule::applySettings (QSettings *settings) {
    // std::cout << "Applying settings for " << getName ().toStdString () << "... ";
    // settings->beginGroup (getName ());
    // 
    // ConfMap::apply (settings, &conf_, confmap);
    // 
    // for (int i = 0; i < MADC32V2_NUM_CHANNELS; ++i) {
    //     QString key = QString ("thresholds%1").arg (i);
    //     if (settings->contains (key)) {
    //         conf_.thresholds[i] = settings->value (key).toUInt ();
    //         //printf("Found key %s with value %d\n",key.toStdString().c_str(),conf_.thresholds[i]);
    //     }
    //     key = QString ("enable_channel%1").arg (i);
    //     if (settings->contains (key))
    //         conf_.enable_channel[i] = settings->value (key).toBool ();
    // }
    // for (int i = 0; i < 2; ++i) {
    //     QString key = QString ("hold_delay_%1").arg (i);
    //     if (settings->contains (key)) {
    //         conf_.hold_delay[i] = settings->value (key).toUInt ();
    //         //printf("Found key %s with value %d\n",key.toStdString().c_str(),conf_.thresholds[i]);
    //     }
    //     key = QString ("hold_width_%1").arg (i);
    //     if (settings->contains (key)) {
    //         conf_.hold_width[i] = settings->value (key).toUInt ();
    //         //printf("Found key %s with value %d\n",key.toStdString().c_str(),conf_.thresholds[i]);
    //     }
    // }
    // 
    // settings->endGroup ();
    // std::cout << "done" << std::endl;
    // 
    // if(getUI()) getUI ()->applySettings ();
}

void FileReaderModule::saveSettings (QSettings *settings) {
    // std::cout << "Saving settings for " << getName ().toStdString () << "... ";
    // settings->beginGroup (getName ());
    // ConfMap::save (settings, &conf_, confmap);
    // for (int i = 0; i < MADC32V2_NUM_CHANNELS; ++i) {
    //     QString key = QString ("thresholds%1").arg (i);
    //     settings->setValue (key, conf_.thresholds [i]);
    //     key = QString ("enable_channel%1").arg (i);
    //     settings->setValue (key, conf_.enable_channel [i]);
    // }
    // for (int i = 0; i < 2; ++i) {
    //     QString key = QString ("hold_delay_%1").arg (i);
    //     settings->setValue (key, conf_.hold_delay [i]);
    //     key = QString ("hold_width_%1").arg (i);
    //     settings->setValue (key, conf_.hold_width [i]);
    // }
    // settings->endGroup ();
    // std::cout << "done" << std::endl;
}

void FileReaderModule::setBaseAddress (uint32_t baddr) {
    conf_.base_addr = baddr;
    if(getUI()) getUI ()->applySettings ();
}

uint32_t FileReaderModule::getBaseAddress () const {
    return conf_.base_addr;
}
/*!
\page Mesytec MADC-32 module
<b>Module name:</b> \c fileReader

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

