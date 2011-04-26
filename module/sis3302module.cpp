#include "sis3302module.h"
#include "modulemanager.h"
#include "eventbuffer.h"

static ModuleRegistrar registrar ("sis3302", Sis3302Module::create);

Sis3302Module::Sis3302Module(int _id, QString _name)
    : BaseModule(_id, _name)
    , dmx (evslots)
{
    setUI (new Sis3302UI(this));

    setChannels();
    createOutputPlugin();


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
        data |= (conf.trigger_threshold[i] & 0xffff);
        if(conf.trgMode[i] == conf.ledFalling)      data |= (1<<24) | (1<<26);
        else if(conf.trgMode[i] == conf.ledRising)  data |= (1<<25) | (1<<26);
        else if(conf.trgMode[i] == conf.firFalling) data |= (1<<24);
        else if(conf.trgMode[i] == conf.firRising)  data |= (1<<25);

        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_TRIGGER_THRESHOLD_ADC%d",ret,i);
    }

    return ret;
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
    uint32_t* ptr = timestampDir[0].data;
    for(int i=0; i<512*2; i++)
    {
        addr = conf.base_addr + SIS3302_TIMESTAMP_DIRECTORY + 4*i;
        data = 0;
        ret = getInterface()->readA32D32(addr,&data);
        if(ret != 0) { printf("Error %d at VME READ EVENT COUNTER\n",ret);}
        else
        {
            (*ptr) = data;
            ptr++;
        }
    }
    for(int i=0; i<512; i++)
    {
        if(timestampDir[i].low != 0)
            printf("TimestampDir %d: high: 0x%x, low: 0x%x\n",i,timestampDir[i].high,timestampDir[i].low);
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
    ret = acquisitionStart();
    if(ret == 0) writeToBuffer(ev);
    return ret;
}

int Sis3302Module::acquisitionStart()
{
    int ret = 0;
    uint32_t evCnt = 0;
    uint32_t curEvent = 0;
    uint32_t nextSampleAddrExp = 0; // Expected next sample address after sampling
    uint32_t startAddr = 0;         // Sampling start address

    for(int i=0; i<8; i++)
    {
        readLength[i] = 0;
    }

    ret = this->arm();

    if(!this->isArmedNotBusy()) printf("sis3302 is not armed or busy!\n");

    ret = this->getEventCounter(&evCnt);
    if (evCnt != 0) printf("sis3302 Event counter is not 0 after arm!\n");

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

    getTimeStampDir();

    for(int i=0; i<8; i++)
    {
        readAdcChannel(i);
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

int Sis3302Module::writeToBuffer(Event *ev)
{
    for(unsigned int i = 0; i < 8; i++)
    {
        if(conf.ch_enabled[i] == false) continue;
        dmx.process (ev, readBuffer[i], readLength[i]);
    }
    return 0;
}

bool Sis3302Module::dataReady()
{
    bool dready = isNotArmedNotBusy();
    return dready;
}

int Sis3302Module::readAdcChannel(int ch)
{
    const int vmeMode = 1;
    AbstractInterface *iface = getInterface ();

    const uint32_t pageLength = 0x400000;
    uint32_t pageLengthMask = pageLength - 1;

    uint32_t nextEventSampleStartAddr = (conf.event_sample_start_addr & 0x01fffffc)  ; // max 32 MSample
    uint32_t restEventSampleLength = (conf.event_length & 0x03fffffc); // max 32 MSample
    if (restEventSampleLength  >= 0x2000000) {restEventSampleLength =  0x2000000 ;}

    int ret = 0 ;

    do {
        uint32_t subEventSampleAddr = 0;
        uint32_t subMaxPageSampleLength = 0;
        uint32_t subEventSampleLength = 0;
        uint32_t subPageAddrOffset = 0;

        uint32_t dmaRequestNofLwords = 0;
        uint32_t dmaAdcAddrOffsetBytes = 0;
        uint32_t reqNofLwords = 0;
        uint32_t gotNofLwords = 0;

        uintptr_t readBufferPtr = 0;

        uint32_t startPos = 0x0;

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

        readBufferPtr = (uintptr_t) &(readBuffer[ch][0]);
        if (vmeMode == 0) { // Single Cycles
            for (uint32_t i=0; i<reqNofLwords; i++) {
                uint32_t* ptr = (uint32_t*)(readBufferPtr+startPos+4*i);
                ret = iface->readA32D32(addr,ptr);
                if (ret != 0) printf("sis3302 return_code = 0x%08x at addr = 0x%08x\n",ret,addr);
                //printf("sis3302: read from addr 0x%x value: 0x%x to buffer 0x%x\n",addr,(*ptr),ptr);
                addr = addr + 4 ;
            }
            startPos = startPos + reqNofLwords;
            readLength[ch] = reqNofLwords;
        }
        else { // DMA
            ret = iface->readA32MBLT64(addr,(uint32_t*)(readBufferPtr),reqNofLwords,&gotNofLwords);
            //ret = iface->readA32MBLT64(addr,&(readBuffer[ch][0]),reqNofLwords,&gotNofLwords);
            readLength[ch] = gotNofLwords;
            if(ret != 0) {
                printf("sis3302 return_code = 0x%08x at addr = 0x%08x\n", ret, addr );
                printf("reqNofLwords = 0x%08x  gotNofLwords = 0x%08x\n", reqNofLwords, gotNofLwords );
                return -1;
            }
            if(reqNofLwords != gotNofLwords) {
                printf("Length Error sis1100w_Vme_Dma_Read:   reqNofLwords = 0x%08x  gotNofLwords = 0x%08x\n", reqNofLwords, gotNofLwords );
                return -1;
            }
            startPos = startPos + gotNofLwords ;
        }

        /*
        printf("\nDump data ch %d\n",ch);
        for(uint32_t i=0; i<gotNofLwords; i++)
        {
            printf(" %d:0x%x  ",i,readBuffer[ch][i]);
        }*/

        nextEventSampleStartAddr =  nextEventSampleStartAddr + subEventSampleLength     ;
        restEventSampleLength     =  restEventSampleLength - subEventSampleLength     ;

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
