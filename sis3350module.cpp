#include "sis3350module.h"
#include "modulemanager.h"

static ModuleRegistrar registrar ("sis3350", Sis3350Module::create, AbstractModule::TypeDAq);

Sis3350Module::Sis3350Module(int _id, QString _name)
        : BaseDAqModule(_id, _name)
{
    setDefaultConfig();
    setUI (new Sis3350UI(this));

    setChannels();
    createBuffer();
    createOutputPlugin();

    std::cout << "Instantiated Sis3350 Module" << std::endl;
}

Sis3350Module::~Sis3350Module()
{
    delete buffer;
}

void Sis3350Module::setDefaultConfig()
{
    conf.acMode = Sis3350config::ringBufferSync;
    conf.acq_enable_lemo = false;
    conf.acq_multi_event = false;
    conf.clock1 = 20;
    conf.clock2 = 0;
    conf.clockSource = Sis3350config::intVar;
    conf.ctrl_lemo_invert = false;
    conf.direct_mem_pretrigger_length = 0;
    conf.direct_mem_sample_length = 1000;
    conf.direct_mem_wrap_length = 134217720;
    conf.ext_clock_trigger_dac_control = 1;
    conf.ext_clock_trigger_daq_data = 37500;
    conf.irqSource = Sis3350config::endOfMultiEvent;
    conf.multievent_max_nof_events = 1;
    conf.nof_reads = 1;
    conf.pollcount = 100000;
    conf.pre_delay = 100;
    conf.sample_length = 1000;

    for(int i = 0; i < 4; i++)
    {
        conf.adc_offset[i] = 32000;
        conf.trigger_enable[i] = false;
        conf.trigger_fir[i] = true;
        conf.trigger_gap_length[i] = 5;
        conf.trigger_gt[i] = true;
        conf.trigger_offset[i] = 10;
        conf.trigger_peak_length[i] = 5;
        conf.trigger_pulse_length[i] = 10;
        conf.trigger_threshold[i] = 50;
        conf.variable_gain[i] = 10;
    }
}

void Sis3350Module::createBuffer()
{
    //buffer = ModuleManager::ptr()->createBuffer(100*10000, 10000, getId ());
}

void Sis3350Module::createOutputPlugin()
{
//    output = PluginManager::ref().create ("demuxsis3350", "Dmx sis3350");
    output = new DemuxSis3350Plugin (-1, getName () + " Dmx");
}

void Sis3350Module::setChannels()
{
    // Setup channels
    getChannels ()->push_back(new ScopeChannel(this,"Sis3350 Raw 0",ScopeCommon::trace,1000,4));
    getChannels ()->push_back(new ScopeChannel(this,"Sis3350 Raw 1",ScopeCommon::trace,1000,4));
    getChannels ()->push_back(new ScopeChannel(this,"Sis3350 Raw 2",ScopeCommon::trace,1000,4));
    getChannels ()->push_back(new ScopeChannel(this,"Sis3350 Raw 3",ScopeCommon::trace,1000,4));
    getChannels ()->push_back(new ScopeChannel(this,"Sis3350 Meta info",ScopeCommon::trace,1000,4));
    getChannels ()->push_back(new ScopeChannel(this,"Sis3350 Poll Trigger",ScopeCommon::trigger,4,4));
}

int Sis3350Module::configure()
{
    int ret = 0x0;

    ret |= configureControlStatus();
    ret |= configureAcquisitionMode();
    ret |= configureMultiEventRegister();
    ret |= configureTriggers();
    ret |= configureDirectMemory();
    ret |= configureExternalDAC();
    ret |= configureInputGains();
    ret |= configureRingBuffer();
    ret |= configureClock();

    if(ret == 0)
    {
        //fprintf(stdout,"Device sis3350 configured!\n");
    }
    else
    {
        fprintf(stderr,"There were errors during config\n");
    }

    return ret;
}

unsigned int Sis3350Module::configureControlStatus()
{
    unsigned int ret;

    // Set Control Status register
    addr = conf.base_addr + SIS3350_CONTROL_STATUS;
    data = 0;
    if(conf.ctrl_lemo_invert) {
        data += SIS3350_CTRL_ENABLE_LEMO_IN_INVERT;
    }
    data += SIS3350_CTRL_ENABLE_USER_LED;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SET STATUS\n",ret);

    return ret;
}

unsigned int Sis3350Module::configureEndAddressThresholds()
{
    unsigned int ret = 0;

    if(conf.acMode == Sis3350config::ringBufferAsync)
    {
        // End address threshold
        addr = conf.base_addr + SIS3350_END_ADDRESS_THRESHOLD_ADC12;
        data = conf.sample_length;
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at END_ADDRESS_THRESHOLD_ADC12\n",ret);

        addr = conf.base_addr + SIS3350_END_ADDRESS_THRESHOLD_ADC34;
        data = conf.sample_length;
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at END_ADDRESS_THRESHOLD_ADC34\n",ret);
    }

    return ret;
}

unsigned int Sis3350Module::configureAcquisitionMode()
{
    unsigned int ret;

    // Setup acquisition mode
    addr = conf.base_addr + SIS3350_ACQUISITION_CONTROL;
    data = 0;
    data += conf.acMode;
    if(conf.acq_enable_lemo) {
        data += SIS3350_ACQ_ENABLE_LEMO_TRIGGER;
    }
    else {
        data += SIS3350_ACQ_ENABLE_INTERNAL_TRIGGER;
    }
    if(conf.acq_multi_event) {
        data += SIS3350_ACQ_ENABLE_MULTIEVENT;
    }
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SET ACQUISITION MODE\n",ret);

    return ret;
}

unsigned int Sis3350Module::configureExternalDAC()
{
    unsigned int ret;

    // Ext DAC Trigger setup
    addr = conf.base_addr + SIS3350_EXT_CLOCK_TRIGGER_DAC_CONTROL_STATUS;
    data = (conf.ext_clock_trigger_dac_control << 4);
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at EXT DAC TRG SETUP\n",ret);

    // Ext DAC Threshold
    addr = conf.base_addr + SIS3350_EXT_CLOCK_TRIGGER_DAC_DATA;
    data = (conf.ext_clock_trigger_daq_data & 0xffff);
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at EXT DAC THR\n",ret);

    return ret;
}

unsigned int Sis3350Module::configureDirectMemory()
{
    unsigned int ret;

    // Direct Memory Stop Mode Sample Wrap Length (only STOP mode)
    addr = conf.base_addr + SIS3350_DIRECT_MEMORY_SAMPLE_WRAP_LENGTH_ALL_ADC;
    data = (conf.direct_mem_wrap_length & 0x07FFFFF8);
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at DIRECT MEM STOP MODE SAMPLE WRAP LENGTH SETUP\n",ret);

    // Direct Memory Sample Length (only START mode)
    addr = conf.base_addr + SIS3350_DIRECT_MEMORY_SAMPLE_LENGTH;
    data = (conf.direct_mem_sample_length & 0x07FFFFF8);
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at DIRECT MEM SAMPLE LENGTH SETUP\n",ret);

    // Direct Memory Pretrigger Length
    addr = conf.base_addr + SIS3350_TRIGGER_DELAY;
    data = (conf.direct_mem_pretrigger_length & 0x03FFFFFE);
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at DIRECT MEM PRETRIGGER LENGTH SETUP\n",ret);

    return ret;
}

unsigned int Sis3350Module::configureInputGains()
{
    unsigned int ret;
    int i=0;

    // ADC input gain value table (coarse)
    // ##############################
    // VGA setting | Input range in V
    // ##############################
    //  10         | 7.992
    //  22         | 4.000
    //  47         | 1.940
    //  95         | 0.950
    // 162         | 0.376
    // 194         | 0.200
    // 242         | 0.110
    //
    // Maximum input range is 8 V


    // ADC input gains
    for(i=0;i<4;i++)
    {
        switch(i)
        {
            case 0:
                addr = conf.base_addr + SIS3350_ADC_VGA_ADC1;
                break;
            case 1:
                addr = conf.base_addr + SIS3350_ADC_VGA_ADC2;
                break;
            case 2:
                addr = conf.base_addr + SIS3350_ADC_VGA_ADC3;
                break;
            case 3:
                addr = conf.base_addr + SIS3350_ADC_VGA_ADC4;
                break;
            default:
                break;
        }

        data = (conf.variable_gain[i] & 0xfff);
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at ADC GAIN %d\n",ret,i);
    }

    // DAC offset value table
    // ########################
    // Value    | Threshold (V)
    // ########################
    // 23700    | - 2.75
    // 30000    | - 0.836
    // 31500    | - 0.370 (NIM)
    // 37500    | + 1.45  (TTL)
    // 40000    | + 2.2
    // 46500    | + 4.00

    writeDacOffset(conf.base_addr + SIS3350_ADC12_DAC_CONTROL_STATUS, 0, conf.adc_offset[0] );
    writeDacOffset(conf.base_addr + SIS3350_ADC12_DAC_CONTROL_STATUS, 1, conf.adc_offset[1] );
    writeDacOffset(conf.base_addr + SIS3350_ADC34_DAC_CONTROL_STATUS, 0, conf.adc_offset[2] );
    writeDacOffset(conf.base_addr + SIS3350_ADC34_DAC_CONTROL_STATUS, 1, conf.adc_offset[3] );

    return ret;
}

unsigned int Sis3350Module::configureMultiEventRegister()
{
    unsigned int ret;

    // Setup multi event register
    addr = conf.base_addr + SIS3350_MULTIEVENT_MAX_NOF_EVENTS;
    data = conf.multievent_max_nof_events;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SETUP MULTIEVENT\n",ret);

    return ret;
}

unsigned int Sis3350Module::configureRingBuffer()
{
    unsigned int ret;

    // Set ringbuffer length
    addr = conf.base_addr + SIS3350_RINGBUFFER_SAMPLE_LENGTH_ALL_ADC;
    ret = iface->writeA32D32(addr,conf.sample_length);
    if(ret != 0) printf("Error %d at SET RB LENGTH\n",ret);

    // Ringbuffer pre-delay
    addr = conf.base_addr + SIS3350_RINGBUFFER_PRE_DELAY_ALL_ADC;
    ret = iface->writeA32D32(addr,conf.pre_delay);
    if(ret != 0) printf("Error %d at SET RB PREDELAY\n",ret);

    return ret;
}

unsigned int Sis3350Module::configureClock()
{
    unsigned int ret;

    // Set frequency synthesizer
    addr = conf.base_addr + SIS3350_FREQUENCE_SYNTHESIZER;
    data = (conf.clock2 & 3) << 9;
    data |= (conf.clock1 & 0x1FF);
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SET CLOCK\n",ret);

    return ret;
}

unsigned int Sis3350Module::configureTriggers()
{
    unsigned int ret=0;
    int i;

    for(i=0;i<4;i++)
    {
        ret=0;

        // Setup triggers
        switch(i)
        {
            case 0:
                addr = conf.base_addr + SIS3350_TRIGGER_SETUP_ADC1;
                break;
            case 1:
                addr = conf.base_addr + SIS3350_TRIGGER_SETUP_ADC2;
                break;
            case 2:
                addr = conf.base_addr + SIS3350_TRIGGER_SETUP_ADC3;
                break;
            case 3:
                addr = conf.base_addr + SIS3350_TRIGGER_SETUP_ADC4;
                break;
            default:
                break;
        }

        data = 0;
        if(conf.trigger_enable[i]) {
                data += SIS3350_TRIGGER_SETUP_TRIGGER_ENABLE;
        } // Otherwise disabled
        if(conf.trigger_fir[i]) {
                data += SIS3350_TRIGGER_SETUP_FIR_TRIGGER_MODE;
        } // Otherwise use normal trigger
        if(conf.trigger_gt[i]) {
                data += SIS3350_TRIGGER_SETUP_TRIGGER_GT_MODE;
        } // Otherwise use LT mode
        data += ((conf.trigger_pulse_length[i] & 0xff) << 16);
        data += ((conf.trigger_gap_length[i]   & 0x1f) <<  8);
        data += ( conf.trigger_peak_length[i]  & 0x1f);
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at TRIGGER SETUP %d\n",ret,i);


        // Trigger thresholds
        ret=0;
        switch(i)
        {
            case 0:
                addr = conf.base_addr + SIS3350_TRIGGER_THRESHOLD_ADC1;
                break;
            case 1:
                addr = conf.base_addr + SIS3350_TRIGGER_THRESHOLD_ADC2;
                break;
            case 2:
                addr = conf.base_addr + SIS3350_TRIGGER_THRESHOLD_ADC3;
                break;
            case 3:
                addr = conf.base_addr + SIS3350_TRIGGER_THRESHOLD_ADC4;
                break;
            default:
                break;
        }

        data = 0;
        data += ( conf.trigger_threshold[i]     & 0xfff);
        data += ((conf.trigger_offset[i] & 0xfff) << 16);
        ret = iface->writeA32D32(addr,data);
        if(ret != 0)
        {
            printf("Error %d at TRIGGER THR %d\n",ret,i);
            return ret;
        }

    }

    return ret;
}

int Sis3350Module::ledOff()
{
    int status;

    addr = conf.base_addr + SIS3350_CONTROL_STATUS;
    data = 0;
    data += SIS3350_CTRL_DISABLE_USER_LED;
    status = iface->writeA32D32(addr,data);
    if(status != 0)
    {
        printf("Error %d at VME READ POLL\n",status);
    }
    return status;
}

int Sis3350Module::ledOn()
{
    int status;

    addr = conf.base_addr + SIS3350_CONTROL_STATUS;
    data = 0;
    data += SIS3350_CTRL_ENABLE_USER_LED;
    status = iface->writeA32D32(addr,data);
    if(status != 0)
    {
        printf("Error %d at VME READ POLL\n",status);
    }
    return status;
}

int Sis3350Module::reset()
{
    int ret;

    addr = conf.base_addr + SIS3350_KEY_RESET;
    ret = iface->writeA32D32(addr,0x1);
    if(ret != 0) printf("Error %d at KEY RESET\n",ret);

    return ret;
}

int Sis3350Module::timestampClear()
{
    int ret = 0;
    addr = conf.base_addr + SIS3350_KEY_TIMESTAMP_CLR;
    ret = iface->writeA32D32(addr,0);
    if(ret != 0) printf("Error %d at KEY TIMESTAMP_CLR\n",ret);
    return ret;
}

int Sis3350Module::arm()
{
    int ret = 0;
    addr = conf.base_addr + SIS3350_KEY_ARM;
    ret = iface->writeA32D32(addr,0);
    if(ret != 0) printf("Error %d at KEY ARM\n",ret);
    return ret;
}

int Sis3350Module::disarm()
{
    int ret = 0;
    addr = conf.base_addr + SIS3350_KEY_DISARM;
    ret = iface->writeA32D32(addr,0);
    if(ret != 0) printf("Error %d at KEY DISARM\n",ret);
    return ret;
}

int Sis3350Module::trigger()
{
    int ret = 0;
    addr = conf.base_addr + SIS3350_KEY_TRIGGER;
    ret = iface->writeA32D32(addr,0);
    if(ret != 0) printf("Error %d at KEY TRIGGER",ret);
    return ret;
}

bool Sis3350Module::pollTrigger()
{
    bool triggered = true;
    uint32_t pollcount = 0;
    conf.pollcount = 100000;

    while(!dataReady())
    {
        pollcount++;
        if(pollcount == conf.pollcount*conf.multievent_max_nof_events)
        {
            triggered = false;
            break;
        }
    }

    if(!triggered)
    {
        printf("No trigger in %d polls.\n",pollcount);
        return false;
    }
    else
    {
        return true;
    }
}

bool Sis3350Module::dataReady()
{
    int ret = 0;
    addr = conf.base_addr + SIS3350_ACQUISITION_CONTROL;
    data = 0;
    ret = iface->readA32D32(addr,&data);
    if(ret != 0)
    {
        printf("Error %d at VME READ POLL\n",ret);
        return false;
    }

    if(conf.acMode == Sis3350config::ringBufferAsync)
    {
        if((data & SIS3350_ACQ_STATUS_END_ADDRESS_FLAG) == SIS3350_ACQ_STATUS_END_ADDRESS_FLAG)
        {
            printf("Disarming\n");
            disarm();
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        if((data & SIS3350_ACQ_STATUS_ARMED_FLAG) == SIS3350_ACQ_STATUS_ARMED_FLAG)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    return true;
}

int Sis3350Module::singleShot()
{
    reset();
    configure();
    arm();
    if(pollTrigger()) return acquisitionStart();
    return -1;
}

int Sis3350Module::acquire()
{
    int ret = 0;
    ret = acquisitionStart();
    if(ret == 0) writeToBuffer();
    //sleep(1);
    arm();
    return ret;
}

int Sis3350Module::writeToBuffer()
{
    DemuxSis3350Plugin* o = dynamic_cast<DemuxSis3350Plugin*>(output);

    for(unsigned int i = 0; i < 4; i++)
    {
        if(!o->getOutputs()->at(i)->hasOtherSide()) continue;
        o->setData(rblt_data[i],read_data_block_length[i]);
        o->process();
    }
    return 0;
}

int Sis3350Module::acquisitionStart()
{
//    QTime t;
//    t.start();
//
//    printf("Acquiring...\n");

    int ret = 0;

    switch(conf.acMode)
    {
        case Sis3350config::ringBufferAsync:
            ret = acquireRingBufferASync();
            break;
        case Sis3350config::ringBufferSync:
            ret = acquireRingBufferSync();
            break;
        case Sis3350config::directMemGateAsync:
            break;
        case Sis3350config::directMemGateSync:
            break;
        case Sis3350config::directMemStop:
            break;
        case Sis3350config::directMemStart:
            ret = acquireDirectMemStart();
            break;
        default:
            return -1;
            break;
    }

//    std::cout << "time " << t.elapsed() << " msec" << std::endl;

    return ret;
}

int Sis3350Module::acquireDirectMemStart()
{
    return acquireRingBufferSync();
}

int Sis3350Module::acquireRingBufferASync()
{
    std::cout << "acquireRingBufferASync "<< std::endl;

    uint32_t i = 0,j = 0;
    uint32_t stop_next_sample_addr[4];
    uintptr_t read_data_block_ptr = NULL;
    uint32_t got_nof_lwords = 0;
    uint32_t sample_length = 0;
    uint32_t event_length = 0;
    int ret = 0;

    sample_length = conf.sample_length;

    event_length = (sample_length/2)+4;

    // Read all four channels
    for(j=0;j<4;j++)
    {
        ret = 0;

        // Read stop sample counter
        switch(j)
        {
            case 0:
                addr = conf.base_addr + SIS3350_ACTUAL_SAMPLE_ADDRESS_ADC1;
                break;
            case 1:
                addr = conf.base_addr + SIS3350_ACTUAL_SAMPLE_ADDRESS_ADC2;
                break;
            case 2:
                addr = conf.base_addr + SIS3350_ACTUAL_SAMPLE_ADDRESS_ADC3;
                break;
            case 3:
                addr = conf.base_addr + SIS3350_ACTUAL_SAMPLE_ADDRESS_ADC4;
                break;
            default:
                break;
        }
        ret = iface->readA32D32(addr,&data);
        if(ret != 0) printf("Error %d at VME READ ACTUAL SAMPLE ADDR %d\n",ret,j);
        stop_next_sample_addr[j] = data;
        printf("Stop next sample address 0x%08x\n",stop_next_sample_addr[j]);

        // Check buffer length
        if(stop_next_sample_addr[j] > 2*MAX_NOF_LWORDS)
        {
                printf("Buffer length too short!\n");
                return -1;
        }

        // Read data buffer
        read_data_block_ptr = (uintptr_t) rblt_data[j];
        if(stop_next_sample_addr[j] != 0)
        {
                ret = readDmaMblt64AdcDataBuffer(conf.base_addr,
                                           j,0x0,stop_next_sample_addr[j],&got_nof_lwords,
                                           (uint32_t *)read_data_block_ptr);
                read_data_block_length[j] = got_nof_lwords;
                if(ret != 0) printf("Error 0x%08x at readDmaMblt64AdcDataBuffer\n",ret);
        }
        else
        {
                printf("<%08d> ADC%d No data read!\n",i,j);
                read_data_block_length[j] = 0;
                ret++;
                continue;
        }

        if(event_length*conf.multievent_max_nof_events - read_data_block_length[j] > 8) // Optimization results in offsets
        {
                printf("<%08d> ADC%d Data block length was too short for amount of events.\n",i,j);
                printf("0x%08x, 0x%08x\n",read_data_block_length[j],
                                event_length*conf.multievent_max_nof_events);
                ret++;
        }

        // Add channel information to first byte of data
        for(unsigned int i = 0; i < conf.multievent_max_nof_events; i++)
        {
            rblt_data[j][i*event_length] |= (j << 28);
        }
    }

    return ret;
}

int Sis3350Module::acquireRingBufferSync()
{
    uint32_t i = 0,j = 0;
    uint32_t stop_next_sample_addr[4];
    uintptr_t read_data_block_ptr = NULL;
    uint32_t got_nof_lwords = 0;
    uint32_t sample_length = 0;
    uint32_t event_length = 0;
    int ret = 0;

    if(conf.acMode==Sis3350config::ringBufferSync)
    {
        sample_length = conf.sample_length;
    }
    else if(conf.acMode==Sis3350config::directMemStart)
    {
        sample_length = conf.direct_mem_sample_length;
    }

    else sample_length = conf.sample_length;

    event_length = (sample_length/2)+4;

    // Read Multievent counter
//    if(conf.multievent_max_nof_events > 1)
//    {
//        addr = conf.base_addr + SIS3350_MULTIEVENT_EVENT_COUNTER;
//        ret = iface->readA32D32(addr,&data);
//        if(ret != 0) printf("Error %d at VME MULTIEVENT COUNTER\n",ret);
//    }

    // Read all four channels
    for(j=0;j<4;j++)
    {
        if(!output->getOutputs()->at(j)->hasOtherSide()) continue;

        ret = 0;

        // Read stop sample counter
        switch(j)
        {
            case 0:
                addr = conf.base_addr + SIS3350_ACTUAL_SAMPLE_ADDRESS_ADC1;
                break;
            case 1:
                addr = conf.base_addr + SIS3350_ACTUAL_SAMPLE_ADDRESS_ADC2;
                break;
            case 2:
                addr = conf.base_addr + SIS3350_ACTUAL_SAMPLE_ADDRESS_ADC3;
                break;
            case 3:
                addr = conf.base_addr + SIS3350_ACTUAL_SAMPLE_ADDRESS_ADC4;
                break;
            default:
                break;
        }
        ret = iface->readA32D32(addr,&data);
        if(ret != 0) printf("Error %d at VME READ ACTUAL SAMPLE ADDR %d\n",ret,j);
        stop_next_sample_addr[j] = data;
        //printf("Stop next sample address 0x%08x\n",stop_next_sample_addr[j]);

        // Check buffer length
        if(stop_next_sample_addr[j] > 2*MAX_NOF_LWORDS)
        {
                printf("Buffer length too short!\n");
                return -1;
        }

        // Read data buffer
        read_data_block_ptr = (uintptr_t) rblt_data[j];
        if(stop_next_sample_addr[j] != 0)
        {
                ret = readDmaMblt64AdcDataBuffer(conf.base_addr,
                                           j,0x0,stop_next_sample_addr[j],&got_nof_lwords,
                                           (uint32_t *)read_data_block_ptr);
                read_data_block_length[j] = got_nof_lwords;
                if(ret != 0) printf("Error 0x%08x at readDmaMblt64AdcDataBuffer\n",ret);
        }
        else
        {
                printf("<%08d> ADC%d No data read!\n",i,j);
                read_data_block_length[j] = 0;
                ret++;
                continue;
        }

        if(event_length*conf.multievent_max_nof_events - read_data_block_length[j] > 8) // Optimization results in offsets
        {
                printf("<%08d> ADC%d Data block length was too short for amount of events.\n",i,j);
                printf("0x%08x, 0x%08x\n",read_data_block_length[j],
                                event_length*conf.multievent_max_nof_events);
                ret++;
        }

        // Add channel information to first byte of data
        for(unsigned int i = 0; i < conf.multievent_max_nof_events; i++)
        {
            rblt_data[j][i*event_length] |= (j << 28);
        }
    }

    return ret;
}



// ######## Struck implemented functions #######################

// Aliases
int Sis3350Module::write_dac_offset(uint32_t module_dac_control_status_addr,
                                    uint32_t dac_select_no, uint32_t dac_value)
{
    return writeDacOffset(module_dac_control_status_addr,
                          dac_select_no, dac_value);
}

int Sis3350Module::sis3350_DMA_Read_MBLT64_ADC_DataBuffer(uint32_t module_address, uint32_t adc_channel /* 0 to 3 */,
                                               uint32_t adc_buffer_sample_start_addr, uint32_t adc_buffer_sample_length,   // 16-bit words
                                               uint32_t*  dma_got_no_of_words, uint32_t* uint_adc_buffer)
{
    return readDmaMblt64AdcDataBuffer(module_address, adc_channel,
                                      adc_buffer_sample_start_addr, adc_buffer_sample_length,
                                      dma_got_no_of_words, uint_adc_buffer);
}

// VME access
int Sis3350Module::writeDacOffset(uint32_t module_dac_control_status_addr,
                                  uint32_t dac_select_no, uint32_t dac_value)
{
        uint32_t error;
        uint32_t data, addr;
        uint32_t max_timeout, timeout_cnt;

        data =  dac_value ;
        addr = module_dac_control_status_addr + 4 ;         // DAC_DATA
        if ((error = iface->writeA32D32(addr,data)) != 0)
        {
                //sisVME_ErrorHandling (error, addr, "sub_vme_A32D32_write");
                return -1;
        }

        data =  1 + (dac_select_no << 4);                   // write to DAC Register
        addr = module_dac_control_status_addr ;
        if ((error = iface->writeA32D32(addr,data)) != 0)
        {
                //sisVME_ErrorHandling (error, addr, "sub_vme_A32D32_write");
                return -1;
        }

        max_timeout = 5000 ;
        timeout_cnt = 0 ;
        addr = module_dac_control_status_addr  ;
        do
        {
            if ((error = iface->readA32D32(addr,&data)) != 0)
            {
                    //sisVME_ErrorHandling (error, addr, "sub_vme_A32D32_read");
                    return -1;
            }
            timeout_cnt++;
        }
        while ( ((data & 0x8000) == 0x8000) && (timeout_cnt <  max_timeout) );

        if (timeout_cnt >=  max_timeout)
        {
                return -2 ;
        }

        data =  2 + (dac_select_no << 4);                   // Load DACs
        addr = module_dac_control_status_addr;
        if ((error = iface->writeA32D32(addr,data)) != 0) {
                printf("Error %d at Load DACs\n",error);
                return -1;
        }

        timeout_cnt = 0 ;
        addr = module_dac_control_status_addr;
        do
        {
            if ((error = iface->readA32D32(addr,&data)) != 0)
            {
                printf("Error %d at Load DACs 2\n",error);
                return -1;
            }
            timeout_cnt++;
        }
        while ( ((data & 0x8000) == 0x8000) && (timeout_cnt <  max_timeout) );

        if (timeout_cnt >=  max_timeout)
        {
                return -3 ;
        }

        return 0x0 ;

}

int Sis3350Module::readDmaMblt64AdcDataBuffer(uint32_t module_address, uint32_t adc_channel /* 0 to 3 */,
                                              uint32_t adc_buffer_sample_start_addr, uint32_t adc_buffer_sample_length,   // 16-bit words
                                              uint32_t*  dma_got_no_of_words, uint32_t* uint_adc_buffer)
{
//#define TEST_PRINT 1
        int return_code ;
        uint32_t data ;
        uint32_t addr, req_nof_lwords, got_nof_lwords ;
        uint32_t dma_request_nof_lwords, dma_adc_addr_offset_bytes ;
        uint32_t max_page_sample_length, page_sample_length_mask ;
        uint32_t next_event_sample_start_addr ;
        uint32_t rest_event_sample_length ;
        uint32_t sub_event_sample_length ;
        uint32_t sub_event_sample_addr ;
        uint32_t sub_max_page_sample_length ;
        uint32_t sub_page_addr_offset ;
        uint32_t index_num_data ;

        max_page_sample_length  = SIS3350_MAX_PAGE_SAMPLE_LENGTH ; // 0x800000  max. 8 MSample / 16 MByte
        page_sample_length_mask = max_page_sample_length - 1 ;

        next_event_sample_start_addr = (adc_buffer_sample_start_addr & 0x07fffffc)  ; // max 128 MSample  256MByte
        rest_event_sample_length     = (adc_buffer_sample_length     & 0xfffffffc)  ; //

        if (rest_event_sample_length  >= SIS3350_MAX_SAMPLE_LENGTH)
        {
            rest_event_sample_length = SIS3350_MAX_SAMPLE_LENGTH ; // 0x8000000 max 128 MSample
        }



        return_code = 0 ;
        index_num_data = 0x0 ;
        int nofTries = 0;

        do
        {
            nofTries++;
            sub_event_sample_addr      = (next_event_sample_start_addr & page_sample_length_mask) ;
            sub_max_page_sample_length =  max_page_sample_length - sub_event_sample_addr ;

            if (rest_event_sample_length >= sub_max_page_sample_length)
            {
                    sub_event_sample_length = sub_max_page_sample_length  ;
            }
            else
            {
                    sub_event_sample_length = rest_event_sample_length  ; // - sub_event_sample_addr
            }

            sub_page_addr_offset = (next_event_sample_start_addr >> 23) & 0xf ;

#ifdef TEST_PRINT
            printf("sub_event_sample_addr  = 0x%08x  sub_event_sample_length  = 0x%08x \n",sub_event_sample_addr, sub_event_sample_length);
            printf("sub_page_addr_offset   = 0x%08x   \n",sub_page_addr_offset);
            printf("rest_event_sample_length   = 0x%08x   \n",rest_event_sample_length);
#endif

            dma_request_nof_lwords     =  (sub_event_sample_length) / 2  ; // Lwords
            dma_adc_addr_offset_bytes  =  (sub_event_sample_addr) * 2    ; // Bytes

            // set page
            addr = module_address + SIS3350_ADC_MEMORY_PAGE_REGISTER  ;
            data = sub_page_addr_offset ;
            if ((return_code = iface->writeA32D32(addr,data)) != 0)
            {
                printf("Error at ADC MEM PAGE REG Setup\n");
                return return_code;
            }


            // read
            addr = module_address
                    + SIS3350_ADC1_OFFSET
                    + (SIS3350_NEXT_ADC_OFFSET * adc_channel)
                    + (dma_adc_addr_offset_bytes);
            req_nof_lwords = dma_request_nof_lwords ; //


#ifdef TEST_PRINT
            printf("addr                 = 0x%08x   \n",addr);
            printf("req_nof_lwords       = 0x%08x   \n",req_nof_lwords);
            printf("sub_page_addr_offset = 0x%08x   \n",sub_page_addr_offset);
            printf("index_num_data       = 0x%08x   \n",index_num_data);
            printf("\n");
#endif

//            return_code = iface->readA32MBLT64(addr,
//                                           &uint_adc_buffer[index_num_data],
//                                           req_nof_lwords,
//                                           &got_nof_lwords);

            return_code = iface->readA322E(addr,
                                           &uint_adc_buffer[index_num_data],
                                           req_nof_lwords,
                                           &got_nof_lwords);

            // Error checks
            if(req_nof_lwords != got_nof_lwords || return_code != 0)
            {
                printf("Ch: %u, Length Error (%d) readDmaMblt64AdcDataBuffer:   req_nof_lwords = 0x%08x  got_nof_lwords = 0x%08x  return_code = 0x%08x\n", adc_channel, nofTries, req_nof_lwords, got_nof_lwords, return_code );
            }


            index_num_data = index_num_data + got_nof_lwords;
            next_event_sample_start_addr =  next_event_sample_start_addr + sub_event_sample_length;
            rest_event_sample_length     =  rest_event_sample_length - sub_event_sample_length;

        } while ((return_code == 0) && (rest_event_sample_length>0)) ;

        *dma_got_no_of_words = index_num_data;
        return return_code;
}


void Sis3350Module::saveSettings(QSettings* settings)
{
    if(settings == NULL)
    {
        std::cout << getName().toStdString() << ": no settings file" << std::endl;
        return;
    }
    else
    {
        std::cout << getName().toStdString() << " saving settings...";
        settings->beginGroup(getName());
        settings->setValue("base_addr",conf.base_addr);
        settings->setValue("acMode",conf.acMode);
        settings->setValue("clockSource",conf.clockSource);
        settings->setValue("irqSource",conf.irqSource);
        settings->setValue("acq_enable_lemo",conf.acq_enable_lemo);
        settings->setValue("acq_multi_event",conf.acq_multi_event);
        settings->setValue("ctrl_lemo_invert",conf.ctrl_lemo_invert);
        settings->setValue("multievent_max_nof_events",conf.multievent_max_nof_events);
        settings->setValue("nof_reads",conf.nof_reads);
        settings->setValue("pre_delay",conf.pre_delay);
        settings->setValue("sample_length",conf.sample_length);

        settings->setValue("direct_mem_wrap_length",conf.direct_mem_wrap_length);
        settings->setValue("direct_mem_sample_length",conf.direct_mem_sample_length);
        settings->setValue("direct_mem_pretrigger_length",conf.direct_mem_pretrigger_length);

        settings->setValue("clock1",conf.clock1);
        settings->setValue("clock2",conf.clock2);
        settings->setValue("ext_clock_trigger_dac_control",conf.ext_clock_trigger_dac_control);
        settings->setValue("ext_clock_trigger_daq_data",conf.ext_clock_trigger_daq_data);
        settings->setValue("pollcount",conf.pollcount);

        for(unsigned int i = 0; i < 4; i++)
        {
            settings->setValue(tr("trigger_enable_%1").arg(i,1,10,QChar()),conf.trigger_enable[i]);
            settings->setValue(tr("trigger_fir_%1").arg(i,1,10,QChar()),conf.trigger_fir[i]);
            settings->setValue(tr("trigger_gt_%1").arg(i,1,10,QChar()),conf.trigger_gt[i]);

            settings->setValue(tr("trigger_gap_length_%1").arg(i,1,10,QChar()),conf.trigger_gap_length[i]);
            settings->setValue(tr("trigger_peak_length_%1").arg(i,1,10,QChar()),conf.trigger_peak_length[i]);
            settings->setValue(tr("trigger_offset_%1").arg(i,1,10,QChar()),conf.trigger_offset[i]);
            settings->setValue(tr("trigger_pulse_length_%1").arg(i,1,10,QChar()),conf.trigger_pulse_length[i]);
            settings->setValue(tr("trigger_threshold_%1").arg(i,1,10,QChar()),conf.trigger_threshold[i]);
            settings->setValue(tr("variable_gain_%1").arg(i,1,10,QChar()),conf.variable_gain[i]);
            settings->setValue(tr("adc_offset_%1").arg(i,1,10,QChar()),conf.adc_offset[i]);
        }

        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void Sis3350Module::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName ());

    bool ok = true;

    set = "base_addr";   if(settings->contains(set)) conf.base_addr = settings->value(set).toInt(&ok);
    cout << getName().toStdString() << " Base address " << std::hex << conf.base_addr << endl;
    set = "acMode";    if(settings->contains(set)) conf.acMode = (Sis3350config::AcMode)settings->value(set).toInt(&ok);
    set = "clockSource";    if(settings->contains(set)) conf.clockSource = (Sis3350config::ClockSource)settings->value(set).toInt(&ok);
    set = "irqSource";    if(settings->contains(set)) conf.irqSource = (Sis3350config::IrqSource)settings->value(set).toInt(&ok);

    set = "acq_enable_lemo";    if(settings->contains(set)) conf.acq_enable_lemo = settings->value(set).toBool();
    set = "acq_multi_event";    if(settings->contains(set)) conf.acq_multi_event = settings->value(set).toBool();
    set = "ctrl_lemo_invert";    if(settings->contains(set)) conf.ctrl_lemo_invert = settings->value(set).toBool();

    set = "multievent_max_nof_events";    if(settings->contains(set)) conf.multievent_max_nof_events = settings->value(set).toInt(&ok);
    set = "nof_reads";    if(settings->contains(set)) conf.nof_reads = settings->value(set).toInt(&ok);
    set = "pre_delay";    if(settings->contains(set)) conf.pre_delay = settings->value(set).toInt(&ok);
    set = "sample_length";    if(settings->contains(set)) conf.sample_length = settings->value(set).toInt(&ok);

    set = "direct_mem_wrap_length";    if(settings->contains(set)) conf.direct_mem_wrap_length = settings->value(set).toInt(&ok);
    set = "direct_mem_sample_length";    if(settings->contains(set)) conf.direct_mem_sample_length = settings->value(set).toInt(&ok);
    set = "direct_mem_pretrigger_length";    if(settings->contains(set)) conf.direct_mem_pretrigger_length = settings->value(set).toInt(&ok);

    set = "clock1";    if(settings->contains(set)) conf.clock1 = settings->value(set).toInt(&ok);
    set = "clock2";    if(settings->contains(set)) conf.clock2 = settings->value(set).toInt(&ok);
    set = "ext_clock_trigger_dac_control";    if(settings->contains(set)) conf.ext_clock_trigger_dac_control = settings->value(set).toInt(&ok);
    set = "ext_clock_trigger_daq_data";    if(settings->contains(set)) conf.ext_clock_trigger_daq_data = settings->value(set).toInt(&ok);
    set = "pollcount";    if(settings->contains(set)) conf.pollcount = settings->value(set).toInt(&ok);

    for(unsigned int i = 0; i < 4; i++)
    {
        set = tr("trigger_enable_%1").arg(i,1,10,QChar());
        if(settings->contains(set)) conf.trigger_enable[i] = settings->value(set).toBool();
        set = tr("trigger_fir_%1").arg(i,1,10,QChar());
        if(settings->contains(set)) conf.trigger_fir[i] = settings->value(set).toBool();
        set = tr("trigger_gt_%1").arg(i,1,10,QChar());
        if(settings->contains(set)) conf.trigger_gt[i] = settings->value(set).toBool();

        set = tr("trigger_gap_length_%1").arg(i,1,10,QChar());
        if(settings->contains(set)) conf.trigger_gap_length[i] = settings->value(set).toInt(&ok);
        set = tr("trigger_peak_length_%1").arg(i,1,10,QChar());
        if(settings->contains(set)) conf.trigger_peak_length[i] = settings->value(set).toInt(&ok);
        set = tr("trigger_offset_%1").arg(i,1,10,QChar());
        if(settings->contains(set)) conf.trigger_offset[i] = settings->value(set).toInt(&ok);
        set = tr("trigger_pulse_length_%1").arg(i,1,10,QChar());
        if(settings->contains(set)) conf.trigger_pulse_length[i] = settings->value(set).toInt(&ok);
        set = tr("trigger_threshold_%1").arg(i,1,10,QChar());
        if(settings->contains(set)) conf.trigger_threshold[i] = settings->value(set).toInt(&ok);

        set = tr("variable_gain_%1").arg(i,1,10,QChar());
        if(settings->contains(set)) conf.variable_gain[i] = settings->value(set).toInt(&ok);
        set = tr("adc_offset_%1").arg(i,1,10,QChar());
        if(settings->contains(set)) conf.adc_offset[i] = settings->value(set).toInt(&ok);
    }

    settings->endGroup();

    getUI()->applySettings();
}

void Sis3350Module::prepareForNextAcquisition()
{
    Sis3350UI* theui = dynamic_cast<Sis3350UI*>(getUI());
    theui->armTimer();
}

void Sis3350Module::setBaseAddress (uint32_t baddr) {
    conf.base_addr = baddr;
    getUI()->applySettings();
}

uint32_t Sis3350Module::getBaseAddress () const {
    return conf.base_addr;
}

/*!
\page sis3350mod SIS 3350 Flash ADC
<b>Module name:</b> \c sis3350

\section desc Module Description
The SIS 3350 is a fast multi-channel ADC.

\section Configuration Panel
DO DOCUMENTATION
*/

