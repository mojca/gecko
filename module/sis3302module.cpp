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

#include "sis3302module.h"
#include "modulemanager.h"
#include "eventbuffer.h"

/******************************** Struck SIS3302 8 ch FADC *****************************
 *
 *
 * Some notes about configuration:
 *
 * - Two modes are generally supported (and make sense):
 *   - Short traces mode (multi-event)
 *   - Long traces mode (single-event)
 *
 * - Short traces mode can be achieved with the following settings
 *   - Multi Event: on
 *   - Autostart Acquisition: on
 *   - Internal trigger as stop: on
 *   - Correct trigger settings! (Check the green led next to the channel input)
 *   - Stop delay: >= 2
 *   - Number of events: == Number of pages, but <= 512
 *   - Page wrap mode: on
 *   - Page size: 32M/number of events
 *   -> Start with Autostart
 *   -> Stop with internal trigger
 *   -> Traces are stored in each page of memory
 *   -> Trace length == page size, but it is possible to read out less
 *
 * - Long traces mode settings:
 *   - Single Event Mode: on
 *   - triggers off, do not stop on trigger
 *   - Event Length Stop Mode: on
 *   - all delays 0
 *   - event length [4..2^25]
 *   - Page Wrap: off
 *   -> Start acq with KEY_START
 *   -> Stop acq with Event length stop
 *   -> Start each acq at addr 0
 *   -> All of the memory is used in one chunk
 *
 * Note on the DAC settings:
 *
 * - The power on default setting of 0 is way too low
 * - To have a 0 V signal about in the middle of the acceptance range set DAC to 37000
 *
 *
 ***************************************************************************************/

static ModuleRegistrar registrar ("sis3302_standard", Sis3302Module::create);

Sis3302Module::Sis3302Module(int _id, QString _name)
    : BaseModule(_id, _name)
    , dmx (evslots)
{
    setUI (new Sis3302UI(this));

    setChannels();
    createOutputPlugin();
    getUI()->applySettings();

    std::cout << "Instantiated Sis3302 Module" << std::endl;
}

Sis3302Module::~Sis3302Module()
{
    //delete buffer;
}

void Sis3302Module::setChannels()
{
    EventBuffer *evb = RunManager::ref ().getEventBuffer ();
    // Setup channels
    for (int ch=0; ch<8; ch++)
    {
        evslots << evb->registerSlot (this, tr("Raw %1").arg(ch), PluginConnector::VectorUint32);
    }

    evslots << evb->registerSlot (this, tr("Meta"), PluginConnector::VectorUint32);
}

int Sis3302Module::configure()
{   
    printf("sis3302::configure\n");
    int ret = 0x0;
    AbstractInterface *iface = getInterface ();

    ret = checkConfig();

    // Set Control Status register
    addr = conf.base_addr + SIS3302_CONTROL_STATUS;
    data = 0;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SET STATUS\n",ret);

    // Set IRQ config register
    addr = conf.base_addr + SIS3302_IRQ_CONFIG;
    data = 0;
    if(conf.irqMode == conf.modeRoak) { data |= (1 << 12); }
    if(conf.enable_irq == true) { data |= (1 << 11); }
    data |= (conf.irq_level << 8) & 0x700;
    data |= (conf.irq_vector) & 0xff;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SET IRQ CONFIG\n",ret);

    // Set IRQ control register
    addr = conf.base_addr + SIS3302_IRQ_CONTROL;
    data = 0;
    data |= (1 << (int)conf.irqSource); // Enable the correct irq source
    data |= (~((1 << (int)conf.irqSource) & 0xffff)) << 16; // Disable all the others
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SET IRQ CONTROL",ret);

    // Set acquisition control register
    addr = conf.base_addr + SIS3302_ACQUISITION_CONTROL;
    data = 0;
    if(conf.autostart_acq) { data |= SIS3302_ACQ_ENABLE_AUTOSTART; }
    else data |= SIS3302_ACQ_DISABLE_AUTOSTART;
    if(conf.acMode == conf.multiEvent) { data |= SIS3302_ACQ_ENABLE_MULTIEVENT; }
    else data |= SIS3302_ACQ_DISABLE_MULTIEVENT;
    if(conf.internal_trg_as_stop) { data |= SIS3302_ACQ_ENABLE_INTERNAL_TRIGGER; }
    else data |= SIS3302_ACQ_DISABLE_INTERNAL_TRIGGER;
    if(conf.enable_external_trg) { data |= SIS3302_ACQ_ENABLE_LEMO_START_STOP; }
    else data |= SIS3302_ACQ_DISABLE_LEMO_START_STOP;
    if(conf.adc_value_big_endian) { data |= (1 << 11); }
    else data |= (1 << 27);
    data |= (conf.clockSource & 0x7) << 12;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SET ACQ CONTROL",ret);

    // Set delay registers
    addr = conf.base_addr + SIS3302_START_DELAY;
    data = conf.start_delay & 0xffffff;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SET START DELAY",ret);
    addr = conf.base_addr + SIS3302_STOP_DELAY;
    data = conf.stop_delay & 0xffffff;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SET STOP DELAY",ret);

    // Set multi event register
    addr = conf.base_addr + SIS3302_MAX_NOF_EVENT;
    data = conf.nof_events & 0xfffff;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SET MAX NOF EVENT",ret);

    // Setup DACs
    sis3302_write_dac_offset(&(conf.dac_offset[0]));

    // Setup event config
    addr = conf.base_addr + SIS3302_EVENT_CONFIG_ALL_ADC;
    data = 0;
    data |= (0xb-conf.wrapSize) & 0xf;
    if(conf.enable_page_wrap) { data |= EVENT_CONF_ENABLE_WRAP_PAGE_MODE; }
    if(conf.event_length_as_stop) { data |= EVENT_CONF_ENABLE_SAMPLE_LENGTH_STOP; }
    data |= (conf.avgMode & 0x7) << 12;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_EVENT_CONFIG_ALL_ADC",ret);

    // Setup event length
    addr = conf.base_addr + SIS3302_SAMPLE_LENGTH_ALL_ADC;
    data = (conf.event_length-4) & 0xfffffc; // manual suggests data = (event_sample_length & 0xfffffffc) - 4 ;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_SAMPLE_LENGTH_ALL_ADC",ret);

    // Set event sample start address
    addr = conf.base_addr + SIS3302_SAMPLE_START_ADDRESS_ALL_ADC;
    data = conf.event_sample_start_addr & 0xfffff;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SET MAX NOF EVENT",ret);

    // Setup trigger registers
    for (int i=0; i<8; i++)
    {
        addr = conf.base_addr + SIS3302_TRIGGER_SETUP_ADC1;
        addr += 8*(i%2);
        addr += (i/2) * SIS3302_NEXT_ADC_OFFSET;

        data = 0;
        data |= conf.trigger_peak_length[i] & 0x1f;
        data |= (conf.trigger_gap_length[i] & 0x1f) << 8;
        data |= (conf.trigger_pulse_length[i] & 0xff) << 16;

        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_TRIGGER_SETUP_ADC%d",ret,i);

        addr = conf.base_addr + SIS3302_TRIGGER_THRESHOLD_ADC1;
        addr += 8*(i%2);
        addr += (i/2) * SIS3302_NEXT_ADC_OFFSET;

        data = 0;
        uint32_t triggervalue = 0;
        if(conf.trgMode[i] == conf.firFalling || conf.trgMode[i] == conf.firRising) {
            triggervalue = conf.trigger_threshold[i] + 0x10000;
        } else {
            triggervalue = conf.trigger_threshold[i];
        }
        data |= (triggervalue & 0x1ffff);
        if(conf.trgMode[i] == conf.ledFalling)      data |= (1<<24) | (1<<26);
        else if(conf.trgMode[i] == conf.ledRising)  data |= (1<<25) | (1<<26);
        else if(conf.trgMode[i] == conf.firFalling) data |= (1<<24);
        else if(conf.trgMode[i] == conf.firRising)  data |= (1<<25);

        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_TRIGGER_THRESHOLD_ADC%d",ret,i);
    }

    return ret;
}

//! Check, if configuration is sane
int Sis3302Module::checkConfig()
{
    // Check, if DACs are greater than zero
    for(int i=0; i<8; i++)
    {
        if(conf.dac_offset[i] == 0) printf("sis3302: Config warning: DAC %d is 0.\n",i);
    }

    // General checks
    if(conf.nof_events > 512)
        printf("sis3302: Config warning: Number of events is greater than 512. More events can not be stored.\n");
    if(conf.clockSource != Sis3302config::creal100)
        printf("sis3302: Config warning: Normally the 2nd internal clock should be used.\n");
    if(conf.internal_trg_as_stop == false &&
       conf.enable_external_trg == false &&
       conf.event_length_as_stop == false)
        printf("sis3302: Config warning: No stop condition set. Sampling will not stop automatically after KEY_START.\n");

    // Check if single event mode is set up in a sane way
    if(conf.acMode == Sis3302config::singleEvent)
    {
        if(conf.internal_trg_as_stop == true)
            printf("sis3302: Config warning: Internal trigger as stop is used in single event mode.\n");
        if(conf.start_delay > 0 || conf.stop_delay > 0)
            printf("sis3302: Config warning: Delays are greater than 0 in single event mode.\n");
        if(conf.nof_events > 1)
            printf("sis3302: Config warning: Number of events > 1 in single event mode is inefficient.\n");
        if(conf.event_length_as_stop == false)
            printf("sis3302: Config warning: Event Length as Stop is not set in single event mode.\n");
        if(conf.enable_page_wrap == true)
            printf("sis3302: Config warning: Page Wrap is enabled in single event mode.\n");
    }

    // Check if multi event mode is set up in a sane way
    if(conf.acMode == Sis3302config::multiEvent)
    {
        if(conf.autostart_acq == false)
            printf("sis3302: Config warning: Autostart acquisition should be used in multi event mode.\n");
        if(conf.internal_trg_as_stop == false &&
           conf.enable_external_trg == false)
            printf("sis3302: Config warning: Internal trigger or external trigger as stop should be used in multi event mode.\n");
        if(conf.event_length_as_stop == true)
            printf("sis3302: Config warning: Event length stop mode should not be used in multi event mode.\n");
        if(conf.stop_delay < 2)
            printf("sis3302: Config warning: Stop delay should be at least 2 in multi event mode to prevent data loss.\n");
        if(conf.nof_events > 0x2000000 / getWrapSizeFromConfig(conf.wrapSize))
            printf("sis3302: Config warning: Number of events is greater than the number of memory pages. Data loss will occur.\n");
        if(conf.enable_page_wrap == false)
            printf("sis3302: Config warning: Page wrap should be enabled in multi event mode.\n");
    }

    return 0;
}

// Return the number of samples per page (wrap mode)
uint32_t Sis3302Module::getWrapSizeFromConfig(Sis3302config::WrapSize ws)
{
    switch(ws)
    {
    case Sis3302config::ws16M: return 16*1024*1024; break;
    case Sis3302config::ws4M: return 4*1024*1024; break;
    case Sis3302config::ws1M: return 1*1024*1024; break;
    case Sis3302config::ws256k: return 256*1024; break;
    case Sis3302config::ws64k: return 64*1024; break;
    case Sis3302config::ws16k: return 16*1024; break;
    case Sis3302config::ws4k: return 4*1024; break;
    case Sis3302config::ws1k: return 1*1024; break;
    case Sis3302config::ws512: return 512; break;
    case Sis3302config::ws256: return 256; break;
    case Sis3302config::ws128: return 128; break;
    case Sis3302config::ws64: return 64; break;
    default: return 1;
    }

    return 0;
}

int Sis3302Module::getModuleId(uint32_t* _modId)
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_MODID;
    data = 0;
    ret = getInterface()->readA32D32(addr,&data);
    if(ret != 0) { printf("Error %d at VME READ ID\n",ret); (*_modId) = 0;}
    else (*_modId) = data;
    return ret;
}

int Sis3302Module::getEventCounter(uint32_t* _evCnt)
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_ACTUAL_EVENT_COUNTER;
    data = 0;
    ret = getInterface()->readA32D32(addr,&data);
    if(ret != 0) { printf("Error %d at VME READ EVENT COUNTER\n",ret); (*_evCnt) = -1;}
    else (*_evCnt) = data;
    return ret;
}

int Sis3302Module::getTimeStampDir()
{
    int ret = 0x0;
    unsigned int vmeMode = 0;
    uint32_t* ptr = timestampDir[0].data;
    uint32_t reqNofLwords = 1024;
    uint32_t gotNofLwords;
    AbstractInterface *iface = getInterface ();

    addr = conf.base_addr + SIS3302_TIMESTAMP_DIRECTORY;
    data = 0;

    if(vmeMode == 0)
    {
        for(unsigned int i=0; i<reqNofLwords; i++)
        {
            ret = iface->readA32D32(addr,&data);
            if(ret != 0) { printf("Error %d at VME READ SIS3302_TIMESTAMP_DIRECTORY\n",ret);}
            else
            {
                (*ptr) = data;
                ptr++;
            }
            addr += 4;
        }
    }
    else
    {
        ret = iface->readA32BLT32(addr,ptr,reqNofLwords,&gotNofLwords);
        if(reqNofLwords != gotNofLwords) { printf("Error %d at VME DMA READ SIS3302_TIMESTAMP_DIRECTORY\n",ret);}
    }

    /*for(unsigned int i=0; i<reqNofLwords/2; i++)
    {
        if(timestampDir[i].low != 0)
            printf("TimestampDir %d: high: 0x%x, low: 0x%x\n",i,timestampDir[i].high,timestampDir[i].low);
    }*/
    return ret;
}

int Sis3302Module::getEventDir(int ch)
{
    int ret = 0x0;
    int vmeMode = 0;
    uint32_t* ptr = &(eventDir[ch][0].data);
    uint32_t reqNofLwords = 512;
    uint32_t gotNofLwords;
    AbstractInterface *iface = getInterface ();

    uint32_t chOff = SIS3302_EVENT_DIRECTORY_ADC1
                     + SIS3302_NEXT_ADC_OFFSET * (ch/2)
                     + 0x8000 * (ch%2);

    addr = conf.base_addr + chOff;
    data = 0;

    if(vmeMode == 0)
    {
        for(unsigned int i=0; i<reqNofLwords; i++)
        {
            ret = iface->readA32D32(addr,&data);
            if(ret != 0) { printf("Error %d at VME READ SIS3302_EVENT_DIRECTORY_ADC\n",ret);}
            else
            {
                (*ptr) = data;
                ptr++;
            }
            addr += 4;
        }
    }
    else
    {
        ret = iface->readA32BLT32(addr,ptr,reqNofLwords,&gotNofLwords);
        if(reqNofLwords != gotNofLwords) { printf("Error %d at VME DMA READ SIS3302_EVENT_DIRECTORY_ADC\n",ret);}
    }

    for(unsigned int i=0; i<reqNofLwords; i++)
    {
        /*if(eventDir[ch][i].trigger == true)
        {
            uint32_t off = (eventDir[ch][i].addr & 0x1fffffc) - i*getWrapSizeFromConfig(conf.wrapSize);
            printf("Event Dir ch %d event %d: Stop pointer 0x%x, wrap bit: 0x%x (0x%08x)\n",ch,i,eventDir[ch][i].addr,eventDir[ch][i].wrap,eventDir[ch][i].data);
            printf("Derived offset for single trace wrt page border: %d (bytes)\n",off);
        }*/
    }
    return ret;
}

int Sis3302Module::getNextSampleAddr(int adc, uint32_t* _addr)
{
    int ret = 0x0;
    addr = conf.base_addr
           + SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC1
           + (adc/2 * SIS3302_NEXT_ADC_OFFSET)
           + (adc%2)*4;
    data = 0;
    ret = getInterface ()->readA32D32(addr,&data);
    //printf("sis3302: ch %d, nextsampleaddr: 0x%x from addr 0x%x\n",adc,data,addr);
    if(ret != 0) { printf("Error %d at VME READ SIS3302_ADCx_OFFSET\n",ret); (*_addr) = 0;}
    else (*_addr) = data;
    return ret;
}

int Sis3302Module::reset()
{
    printf("sis3302::reset\n");
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_KEY_RESET; data = 0;
    ret = getInterface()->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_KEY_RESET",ret);
    return ret;
}

int Sis3302Module::arm()
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_KEY_ARM; data = 0;
    ret = getInterface()->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_KEY_ARM",ret);
    return ret;
}

int Sis3302Module::disarm()
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_KEY_DISARM; data = 0;
    ret = getInterface()->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_KEY_DISARM",ret);
    return ret;
}

int Sis3302Module::start_sampling()
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_KEY_START; data = 0;
    ret = getInterface()->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_KEY_START",ret);
    return ret;
}

int Sis3302Module::stop_sampling()
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_KEY_STOP; data = 0;
    ret = getInterface()->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_KEY_STOP",ret);
    return ret;
}

int Sis3302Module::reset_DDR2_logic()
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_KEY_RESET_DDR2_LOGIC; data = 0;
    ret = getInterface()->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_KEY_RESET_DDR2_LOGIC",ret);
    return ret;
}

int Sis3302Module::timestamp_clear()
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_KEY_TIMESTAMP_CLEAR; data = 0;
    ret = getInterface()->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_KEY_TIMESTAMP_CLEAR",ret);
    return ret;
}

bool Sis3302Module::isArmedNotBusy()
{
    bool ret = false;
    int test = 0;
    addr = conf.base_addr + SIS3302_ACQUISITION_CONTROL;
    data = 0;
    test = getInterface()->readA32D32(addr,&data);
    if(test != 0) { printf("Error %d at VME READ ARM AND BUSY STATE\n",ret);}
    else ret = ((data & 0x30000) == 0x10000);
    return ret;
}

bool Sis3302Module::isNotArmedNotBusy()
{
    bool ret = false;
    int test = 0;
    addr = conf.base_addr + SIS3302_ACQUISITION_CONTROL;
    data = 0;
    test = getInterface()->readA32D32(addr,&data);
    if(test != 0) { printf("Error %d at VME READ ARM AND BUSY STATE\n",ret);}
    else ret = ((data & 0x30000) == 0x00000);
    return ret;
}

int Sis3302Module::waitForSamplingComplete()
{
    //printf("sis3302: waitForSamplingComplete \n");

    bool ret = false;
    int test = 0;
    AbstractInterface *iface = getInterface ();
    addr = conf.base_addr + SIS3302_ACQUISITION_CONTROL;
    data = 0;
    unsigned int cnt = 0;
    do
    {
        test = iface->readA32D32(addr,&data);
        if(test != 0) { printf("Error %d at VME READ SIS3302_ACQUISITION_CONTROL\n",ret);}
        else ret = ((data & 0x30000) == 0x10000);
        cnt++;
    } while(((data & 0x10000) == 0x10000) && (cnt < conf.poll_count));
    //printf("Waited for %d poll loops.\n",cnt);
    ret = (cnt < conf.poll_count);
    return ret;
}

int Sis3302Module::acquire(Event *ev)
{
    int ret = 0;

    printf("sis3302: Acquiring event %p\n", ev);

    if(conf.acMode == Sis3302config::singleEvent ||
       conf.enable_page_wrap == false) {
        ret = acquisitionStartSingle(); }
    if(conf.acMode == Sis3302config::multiEvent &&
       conf.enable_page_wrap == true) {
        ret = acquisitionStartMulti(); }
    if(ret == 0) writeToBuffer(ev);
    else printf("sis3302: Failed acquiring event %p, ret = 0x%x\n", ev,ret);

    return ret;
}

int Sis3302Module::acquisitionStartSingle()
{
    int ret = 0;
    uint32_t evCnt = 0;
    uint32_t curEvent = 0;
    uint32_t nextSampleAddrExp = 0; // Expected next sample address after sampling
    uint32_t startAddr = conf.event_sample_start_addr; // Sampling start address

    for(int i=0; i<8; i++)
    {
        readLength[i] = 0;
    }

    ret = this->arm();

    if(conf.autostart_acq == false) {
        if(!this->isArmedNotBusy()) printf("sis3302 is not armed or busy!\n");
        ret = this->getEventCounter(&evCnt);
        if (evCnt != 0) printf("sis3302 Event counter is not 0 after arm!\n");
    }

    for(curEvent = 0; curEvent < conf.nof_events; curEvent++)
    {
        if(conf.autostart_acq == false && conf.multiEvent == true) ret = this->start_sampling();
        ret = waitForSamplingComplete();
        if(ret == false) printf("sis3302 timeout while waiting for sampling to complete\n");

        ret = this->getEventCounter(&evCnt);
        if(evCnt != curEvent+1) printf("sis3302 event counter mismatch evCnt: %u expected: %u\n",evCnt,curEvent+1);

        if(evCnt == conf.nof_events && isArmedNotBusy()) printf("sis3302 still armed at last event.\n");
        else if(evCnt < conf.nof_events && isNotArmedNotBusy()) printf("sis3302 not armed before last event.\n");
    }


    // Check if the next sample address matches the expected value
    nextSampleAddrExp = (((startAddr &  0xfffffffc) + (conf.event_length & 0xfffffffc) ) & 0x1ffffff)  ;
    nextSampleAddrExp =  nextSampleAddrExp * conf.nof_events ;

    for(int i=0; i<8; i++)
    {
        if(conf.ch_enabled[i])
        {
            uint32_t nextSampleAddrActual = 0x0;
            this->getNextSampleAddr(i,&nextSampleAddrActual);
            if(nextSampleAddrExp != nextSampleAddrActual)
                printf("sis3302 adc %d: next sample addr mismatch (expected: 0x%x, got: 0x%x)\n",i,nextSampleAddrExp,nextSampleAddrActual);
        }
    }

    // Read the timestamp dir (only one entry)
    getTimeStampDir();

    for(int i=0; i<8; i++)
    {
        readAdcChannel(i, (conf.event_length/2)*evCnt);
        /*if(conf.ch_enabled[i])
        {
            for(uint32_t s = 0; s < readLength[i]; s++)
            {
                printf("Data: %d: 0x%x\n",s,readBuffer[i][s]);
            }
        }*/
        // Store channel information in highest 3 bits of readLength[i];
        readLength[i] |= (i << 29);
    }

    return ret;
}

int Sis3302Module::acquisitionStartMulti()
{
    int ret = 0;
    uint32_t evCnt = 0;
    uint32_t nofReqWords = 0;

    for(int i=0; i<8; i++)
    {
        readLength[i] = 0;
    }

    // Acquire

    //printf("sis3302: acquisitionStartMulti\n");

    ret = this->arm(); // After arm, sampling should start automatically

    //printf("sis3302: arm \n");

    if(conf.autostart_acq == false) ret = this->start_sampling(); // If not, force start

    //printf("sis3302: start_sampling \n");

    ret = waitForSamplingComplete(); // Wait and poll

    //printf("sis3302: getEventCounter \n");

    ret = this->getEventCounter(&evCnt);
    if(evCnt != conf.nof_events) printf("sis3302 event counter mismatch (expected: %u, got: %u).\n",conf.nof_events,evCnt);
    nofReqWords = evCnt * (getWrapSizeFromConfig(conf.wrapSize) / 2);
    printf("sis3302 nofReqWords = %d\n",nofReqWords);

    // Read the timestamp dir
    getTimeStampDir();

    //printf("sis3302: getTimeStampDir \n");

    // Read the event dirs
    for(int ch=0; ch<8; ch++)
    {
        if(conf.ch_enabled[ch] == true)
            getEventDir(ch);
    }

    // Read data from the ADC buffers
    for(int ch=0; ch<8; ch++)
    {
        readAdcChannel(ch, nofReqWords);
    }

    return ret;
}

int Sis3302Module::writeToBuffer(Event *ev)
{
    for(unsigned int i = 0; i < 8; i++)
    {
        //printf("sis3302: ch %d: Trying to write to buffer (ev = 0x%x)\n",i,ev);
        if(conf.ch_enabled[i] == false) continue;
        if(conf.enable_page_wrap == true) dmx.setMetaData(conf.nof_events,eventDir[i],timestampDir);
        dmx.process (ev, readBuffer[i], readLength[i]);
        //printf("sis3302: ch %d: Success!\n",i);
    }
    return 0;
}

bool Sis3302Module::dataReady()
{
    bool dready = isNotArmedNotBusy();
    return dready;
}

int Sis3302Module::readAdcChannel(int ch, uint32_t _reqNofWords)
{
    //printf("sis3302 Starting ADC ch %d read of %d lwords\n",ch,_reqNofWords);

    const int vmeMode = 1;
    AbstractInterface *iface = getInterface ();

    const uint32_t pageLength = 0x400000; // 4 MSample == 8 MByte
    uint32_t pageLengthMask = pageLength - 1;

    uint32_t nextEventSampleStartAddr = (conf.event_sample_start_addr & 0x01fffffc)  ; // max 32 MSample == 64 MByte
    uint32_t restEventSampleLength = ((_reqNofWords*2) & 0x01fffffc); // max 32 MSample == 64 MByte
    if (restEventSampleLength  >= 0x2000000) {restEventSampleLength =  0x2000000 ;}

    int ret = 0 ;

    readLength[ch] = 0;

    do {
        uint32_t subEventSampleAddr = 0;
        uint32_t subMaxPageSampleLength = 0;
        uint32_t subEventSampleLength = 0;
        uint32_t subPageAddrOffset = 0;

        uint32_t dmaRequestNofLwords = 0;   // LWord aligned
        uint32_t dmaAdcAddrOffsetBytes = 0; // Byte aligned
        uint32_t reqNofLwords = 0;  // LWord aligned
        uint32_t gotNofLwords = 0;  // LWord aligned

        uintptr_t readBufferPtr = 0;    // Byte aligned

        uint32_t startPos = 0x0;    // Byte aligned

        subEventSampleAddr      =  (nextEventSampleStartAddr & pageLengthMask) ;
        subMaxPageSampleLength  =  pageLength - subEventSampleAddr ;

        if (restEventSampleLength >= subMaxPageSampleLength) {
            subEventSampleLength = subMaxPageSampleLength  ;
        }
        else {
            subEventSampleLength = restEventSampleLength  ;
        }

        subPageAddrOffset =  (nextEventSampleStartAddr >> 22) & 0x7 ;

        dmaRequestNofLwords    =  (subEventSampleLength) / 2  ; // Lwords
        dmaAdcAddrOffsetBytes  =  (subEventSampleAddr) * 2    ; // Bytes

        // set page
        addr = conf.base_addr + SIS3302_ADC_MEMORY_PAGE_REGISTER  ;
        data = subPageAddrOffset ;
        ret = iface->writeA32D32(addr,data);
        if (ret != 0) printf("sis3302 return_code = 0x%08x at addr = 0x%08x\n",ret,addr);
        // printf("sis3302: requested subPageAddr %d from 0x%x\n",data,addr);

        // read
        addr =	  conf.base_addr
                + SIS3302_ADC1_OFFSET
                + (SIS3302_NEXT_ADC_OFFSET * ch)
                + (dmaAdcAddrOffsetBytes);

        reqNofLwords = dmaRequestNofLwords;

        //printf("sis3302: Starting read from adc %d from addr: 0x%x\n",ch,addr);

        readBufferPtr = (uintptr_t) &(readBuffer[ch][startPos]);

        if (vmeMode == 0) { // Single Cycles
            for (uint32_t i=0; i<reqNofLwords; i++) {
                uint32_t* ptr = (uint32_t*)(readBufferPtr+4*i);
                ret = iface->readA32D32(addr,ptr);
                if (ret != 0) printf("sis3302 return_code = 0x%08x at addr = 0x%08x\n",ret,addr);
                //printf("sis3302: read from addr 0x%x value: 0x%x to buffer 0x%x\n",addr,(*ptr),ptr);
                addr = addr + 4 ;
            }
            startPos += reqNofLwords*4;
            readLength[ch] += reqNofLwords;
        }
        else { // DMA
            ret = iface->readA32MBLT64(addr,(uint32_t*)(readBufferPtr),reqNofLwords,&gotNofLwords);
            if(ret != 0) {
                printf("sis3302 return_code = 0x%08x at addr = 0x%08x\n", ret, addr );
                printf("sis3302 reqNofLwords = 0x%08x  gotNofLwords = 0x%08x\n", reqNofLwords, gotNofLwords );
                return -1;
            }
            if(reqNofLwords != gotNofLwords) {
                printf("sis3302 Length Error sis1100w_Vme_Dma_Read:   reqNofLwords = 0x%08x  gotNofLwords = 0x%08x\n", reqNofLwords, gotNofLwords );
                return -1;
            }
            startPos += gotNofLwords*4;
            readLength[ch] += gotNofLwords;
        }

	// Store channel info
	readLength[ch] |= ((ch & 0x7) << 29);

        //printf("\nsis3302 After read Ch %d: StartPos: %d byte, ReadLength: %d lwords\n",ch,startPos,readLength[ch]);

        /*printf("\nsis3302 Dump data ch %d\n",ch);
        for(uint32_t i=0; i<gotNofLwords; i++)
        {
            printf(" %d:0x%x  ",i,readBuffer[ch][i]);
        }*/

        nextEventSampleStartAddr =  nextEventSampleStartAddr + subEventSampleLength     ;
        restEventSampleLength    =  restEventSampleLength - subEventSampleLength     ;

    } while ((ret == 0) && (restEventSampleLength > 0)) ;
    return ret;
}


// STRUCK legacy methods

/* --------------------------------------------------------------------------
   SIS3302 Offset
   offset Value_array		DAC offset value (16 bit)
-------------------------------------------------------------------------- */
int Sis3302Module::sis3302_write_dac_offset(unsigned int *offset_value_array)
{
        uint32_t i, error;
        uint32_t data, addr;
        uint32_t max_timeout, timeout_cnt;
        int ret = 0;
        AbstractInterface *iface = getInterface ();

        for (i=0;i<8;i++) {

                data =  offset_value_array[i] ;
                addr = conf.base_addr + SIS3302_DAC_DATA  ;
                if ((error = iface->writeA32D32(addr,data )) != 0) {
                        printf ("Error at sub_vme_A32D32_write");
                        ret++;
                }

                data =  1 + (i << 4); // write to DAC Register
                addr = conf.base_addr + SIS3302_DAC_CONTROL_STATUS  ;
                if ((error = iface->writeA32D32(addr,data )) != 0) {
                         printf ("Error at sub_vme_A32D32_write");
                         ret++;
                }

                max_timeout = 5000 ;
                timeout_cnt = 0 ;
                addr = conf.base_addr + SIS3302_DAC_CONTROL_STATUS  ;
                do {
                        if ((error = iface->readA32D32(addr,&data )) != 0) {
                                printf ("Error at sub_vme_A32D32_read");
                                ret++;
                        }
                        timeout_cnt++;
                } while ( ((data & 0x8000) == 0x8000) && (timeout_cnt <  max_timeout) )    ;

                if (timeout_cnt >=  max_timeout) {
                        ret++;
                }


                data =  2 + (i << 4); // Load DACs
                addr = conf.base_addr + SIS3302_DAC_CONTROL_STATUS  ;
                if ((error = iface->writeA32D32(addr,data )) != 0) {
                        printf ("Error at sub_vme_A32D32_write");
                        ret++;
                }
                timeout_cnt = 0 ;
                addr = conf.base_addr + SIS3302_DAC_CONTROL_STATUS  ;
                do {
                        if ((error = iface->readA32D32(addr,&data )) != 0) {
                                printf ("Error at sub_vme_A32D32_read");
                                ret++;
                        }
                        timeout_cnt++;
                } while ( ((data & 0x8000) == 0x8000) && (timeout_cnt <  max_timeout) )    ;

                if (timeout_cnt >=  max_timeout) {
                        ret++;
                }
        }

        return ret;
}




// Configuration handling

typedef ConfMap::confmap_t<Sis3302config> confmap_t;
static const confmap_t confmap [] = {
    confmap_t ("base_addr", &Sis3302config::base_addr),
    confmap_t ("event_length", &Sis3302config::event_length),
    confmap_t ("start_delay", &Sis3302config::start_delay),
    confmap_t ("stop_delay", &Sis3302config::stop_delay),
    confmap_t ("nof_events", &Sis3302config::nof_events),
    confmap_t ("irq_level", &Sis3302config::irq_level),
    confmap_t ("irq_vector", &Sis3302config::irq_vector),
    confmap_t ("autostart_acq", &Sis3302config::autostart_acq),
    confmap_t ("internal_trg_as_stop", &Sis3302config::internal_trg_as_stop),
    confmap_t ("event_length_as_stop", &Sis3302config::event_length_as_stop),
    confmap_t ("adc_value_big_endian", &Sis3302config::adc_value_big_endian),
    confmap_t ("enable_page_wrap", &Sis3302config::enable_page_wrap),
    confmap_t ("enable_irq", &Sis3302config::enable_irq),
    confmap_t ("enable_external_trg", &Sis3302config::enable_external_trg),
    confmap_t ("acMode", (uint32_t Sis3302config::*) &Sis3302config::acMode),
    confmap_t ("wrapSize", (uint32_t Sis3302config::*) &Sis3302config::wrapSize),
    confmap_t ("avgMode", (uint32_t Sis3302config::*) &Sis3302config::avgMode),
    confmap_t ("clockSource", (uint32_t Sis3302config::*) &Sis3302config::clockSource),
    confmap_t ("irqSource", (uint32_t Sis3302config::*) &Sis3302config::irqSource),
    confmap_t ("irqMode", (uint32_t Sis3302config::*) &Sis3302config::irqMode),
};

void Sis3302Module::applySettings (QSettings *s) {
    if(s==0) return;

    std::cout << "Applying settings for " << getName ().toStdString () << "... ";
    s->beginGroup (getName ());
    ConfMap::apply (s, &conf, confmap);

    QString key;
    for (int i = 0; i < 8; ++i) {
        key = QString ("trgMode%1").arg (i);
        if (s->contains (key)) conf.trgMode [i] = (Sis3302config::TrgMode)s->value (key).toUInt ();
        key = QString ("trigger_pulse_length%1").arg (i);
        if (s->contains (key)) conf.trigger_pulse_length [i] = s->value (key).toUInt ();
        key = QString ("trigger_gap_length%1").arg (i);
        if (s->contains (key)) conf.trigger_gap_length [i] = s->value (key).toUInt ();
        key = QString ("trigger_peak_length%1").arg (i);
        if (s->contains (key)) conf.trigger_peak_length [i] = s->value (key).toUInt ();
        key = QString ("trigger_threshold%1").arg (i);
        if (s->contains (key)) conf.trigger_threshold [i] = s->value (key).toUInt ();
        key = QString ("dac_offset%1").arg (i);
        if (s->contains (key)) conf.dac_offset [i] = s->value (key).toUInt ();
        key = QString ("ch_enabled%1").arg (i);
        if (s->contains (key)) conf.ch_enabled [i] = s->value (key).toBool ();
    }

    s->endGroup ();

    getUI ()->applySettings ();
    std::cout << "done" << std::endl;
}

void Sis3302Module::saveSettings (QSettings *s) {
    if(s==0) return;

    std::cout << "Saving settings for " << getName ().toStdString () << "... ";
    s->beginGroup (getName ());
    ConfMap::save (s, &conf, confmap);

    QString key;
    for (int i = 0; i < 8; ++i) {
        key = QString ("trgMode%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.trgMode[i]));
        key = QString ("trigger_pulse_length%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.trigger_pulse_length[i]));
        key = QString ("trigger_gap_length%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.trigger_gap_length[i]));
        key = QString ("trigger_peak_length%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.trigger_peak_length[i]));
        key = QString ("trigger_threshold%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.trigger_threshold[i]));
        key = QString ("dac_offset%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.dac_offset[i]));
        key = QString ("ch_enabled%1").arg(i);
        s->setValue (key, static_cast<bool> (conf.ch_enabled[i]));
    }

    s->endGroup ();
    std::cout << "done" << std::endl;
}

/*!
\page sis3302mod SIS 3302 Flash ADC
<b>Module name:</b> \c sis3302

\section desc Module Description
The SIS 3302 is a fast multi-channel ADC.

\section Configuration Panel
DO DOCUMENTATION
*/
