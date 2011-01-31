#include "sis3350control.h"

Sis3350control::Sis3350control()
{
    c = new ConfigManager();
    c->readConfig("3350scope.conf");
    getConfigFromFile();
    //getConfigFromCode();
    this->m_device = NULL;
}

void Sis3350control::setDevice(HANDLE m_device)
{
    this->m_device = m_device;
}

//int Sis3350control::open()
//{
//    int status;
//    unsigned int found;
//
//    //! Find connected devices
//    status = FindAll_SIS3150USB_Devices(info, &found, 1);
//    fprintf(stdout,"Found %d devices with status %d \n",found,status);
//    if(status != 0) {
//        fprintf(stderr,"No device found!\n");
//        return 1;
//    }
//
//    //! Open the device
//    if(found >= 1) {
//        status = Sis3150usb_OpenDriver((PCHAR)info[0].cDName, &m_device);
//        if(status != 0) {
//            fprintf(stderr,"ERROR: %d\n",status);
//            return status;
//        }
//    }
//    else {
//        fprintf(stderr,"No device found\n");
//        return 1;
//    }
//    fprintf(stdout,"Device 0x%08lx opened.\n",(long int)m_device);
//
//    return 0;
//}

int Sis3350control::close()
{
//    int status;
//
    //! Finish up /////////////////////////////////////////////////////////
    //! ###################################################################

    // Switch off LED
    addr = conf.base_addr + SIS3350_CONTROL_STATUS;
    data = 0;
    data += SIS3350_CTRL_DISABLE_USER_LED;
    vme_A32D32_write(m_device,addr,data);

//    //! Close the device
//    status = Sis3150usb_CloseDriver(m_device);
//    fprintf(stdout,"Device closed\n");


    return 0;
}

int Sis3350control::reset()
{
    //cout << "Resetting sis3350...";
    int ret;

    // Reset
    addr = conf.base_addr + SIS3350_KEY_RESET;
    ret = vme_A32D32_write(m_device,addr,1);
    if(ret != 0) printf("Error %d at KEY RESET\n",ret);

    //cout << "done." << endl;

    return ret;
}

// This is the default configuration for the module
// This functions reads the configuration from a *.conf file
int Sis3350control::getConfigFromFile()
{
    bool ok = true;
    conf.base_addr = c->getItem("base_addr").toInt(&ok,16);
    cout << "Base address " << conf.base_addr << " " << ok << endl;
    conf.acq_mode = c->getItem("acq_mode").toInt(&ok,16);
    //conf.acq_mode = SIS3350_ACQ_OPERATION_DIRECT_MEMORY_START; // Use with caution!

    if(c->getItem("acq_enable_lemo") == QString("false"))
        conf.acq_enable_lemo = false;
    else
        conf.acq_enable_lemo = true;
    if(c->getItem("acq_multi_event") == QString("false"))
        conf.acq_multi_event = false;
    else
        conf.acq_multi_event = true;
    if(c->getItem("ctrl_lemo_invert") == QString("false"))
        conf.ctrl_lemo_invert = false;
    else
        conf.ctrl_lemo_invert = true;

    conf.multievent_max_nof_events = c->getItem("multievent_max_nof_events").toInt();
    conf.nof_reads = c->getItem("nof_reads").toInt();
    conf.pre_delay = c->getItem("pre_delay").toInt();
    conf.sample_length = c->getItem("sample_length").toInt(); // multiples of 8 !!!
    conf.direct_mem_wrap_length = c->getItem("direct_mem_wrap_length").toInt(&ok,16);  // Maximum is 0x07FFFFF8
    conf.direct_mem_sample_length = c->getItem("direct_mem_sample_length").toInt(&ok,16);    // Maximum is 0x07FFFFF8
    conf.direct_mem_pretrigger_length = c->getItem("direct_mem_pretrigger_length").toInt(&ok,16);
    conf.clock1 = c->getItem("clock1").toInt();
    conf.clock2 = c->getItem("clock2").toInt();

    for(unsigned int i = 0; i < 4; i++)
    {
        if(c->getItem(tr("trigger_enable_%1").arg(i,1,10,QChar())) == QString("false"))
            conf.trigger_enable[i] = false;
        else
            conf.trigger_enable[i] = true;
        if(c->getItem(tr("trigger_fir_%1").arg(i,1,10,QChar())) == QString("false"))
            conf.trigger_fir[i] = false;
        else
            conf.trigger_fir[i] = true;
        if(c->getItem(tr("trigger_gt_%1").arg(i,1,10,QChar())) == QString("false"))
            conf.trigger_gt[i] = false;
        else
            conf.trigger_gt[i] = true;
        conf.trigger_gap_length[i] = c->getItem(tr("trigger_gap_length_%1").arg(i,1,10,QChar())).toInt();
        conf.trigger_peak_length[i] = c->getItem(tr("trigger_peak_length_%1").arg(i,1,10,QChar())).toInt();
        conf.trigger_offset[i] = c->getItem(tr("trigger_offset_%1").arg(i,1,10,QChar())).toInt();
        conf.trigger_pulse_length[i] = c->getItem(tr("trigger_pulse_length_%1").arg(i,1,10,QChar())).toInt();
        conf.trigger_threshold[i] = c->getItem(tr("trigger_threshold_%1").arg(i,1,10,QChar())).toInt();

        conf.variable_gain[i] = c->getItem(tr("variable_gain_%1").arg(i,1,10,QChar())).toInt();
        conf.adc_offset[i] = c->getItem(tr("adc_offset_%1").arg(i,1,10,QChar())).toInt();

        if(c->getItem(tr("writeEnabled_%1").arg(i,1,10,QChar())) == QString("false"))
            conf.writeEnabled[i] = false;
        else
            conf.writeEnabled[i] = true;
    }

    conf.ext_clock_trigger_dac_control = c->getItem("ext_clock_trigger_dac_control").toInt();
    conf.ext_clock_trigger_daq_data = c->getItem("ext_clock_trigger_daq_data").toInt();

    strcpy(conf.filePath,c->getItem("filePath").toStdString().c_str());
    conf.filePointSkip = c->getItem("filePointSkip").toInt();

    conf.pollcount = c->getItem("pollcount").toInt();

    //cout << "Config read from file!" << endl;

    return 0;
}

// This is the default configuration for the module
int Sis3350control::getConfigFromCode()
{
    conf.base_addr = 0x30000000;
    conf.acq_mode = SIS3350_ACQ_OPERATION_RINGBUFFER_SYNCH;
    //conf.acq_mode = SIS3350_ACQ_OPERATION_DIRECT_MEMORY_START; // Use with caution!
    conf.acq_enable_lemo = false;
    conf.acq_multi_event = true;
    conf.ctrl_lemo_invert = false;

    conf.multievent_max_nof_events = 1;
    conf.nof_reads = 1;
    conf.pre_delay = 100;
    conf.sample_length = 512; // multiples of 8 !!!
    conf.direct_mem_wrap_length = 0x07FFFFF8;  // Maximum is 0x07FFFFF8
    conf.direct_mem_sample_length = 0x3FFFFF8;    // Maximum is 0x07FFFFF8
    conf.direct_mem_pretrigger_length = 0xFF;
    conf.clock1 = 20;
    conf.clock2 = 0;

    conf.trigger_enable[0] = true;
    conf.trigger_fir[0] = true;
    conf.trigger_gap_length[0] = 6;
    conf.trigger_gt[0] = true;
    conf.trigger_offset[0] = 10;
    conf.trigger_peak_length[0] = 6;
    conf.trigger_pulse_length[0] = 12;
    conf.trigger_threshold[0] = 0;

    conf.trigger_enable[1] = false;
    conf.trigger_fir[1] = true;
    conf.trigger_gap_length[1] = 6;
    conf.trigger_gt[1] = true;
    conf.trigger_offset[1] = 10;
    conf.trigger_peak_length[1] = 6;
    conf.trigger_pulse_length[1] = 12;
    conf.trigger_threshold[1] = 0;

    conf.trigger_enable[2] = false;
    conf.trigger_fir[2] = true;
    conf.trigger_gap_length[2] = 6;
    conf.trigger_gt[2] = true;
    conf.trigger_offset[2] = 10;
    conf.trigger_peak_length[2] = 6;
    conf.trigger_pulse_length[2] = 12;
    conf.trigger_threshold[2] = 0;

    conf.trigger_enable[3] = false;
    conf.trigger_fir[3] = true;
    conf.trigger_gap_length[3] = 6;
    conf.trigger_gt[3] = true;
    conf.trigger_offset[3] = 10;
    conf.trigger_peak_length[3] = 6;
    conf.trigger_pulse_length[3] = 12;
    conf.trigger_threshold[3] = 0;

    conf.ext_clock_trigger_dac_control = 1;
    conf.ext_clock_trigger_daq_data = 37500;

    conf.variable_gain[0] = 50;
    conf.variable_gain[1] = 50;
    conf.variable_gain[2] = 50;
    conf.variable_gain[3] = 50;

    conf.adc_offset[0] = 31000;
    conf.adc_offset[1] = 32000;
    conf.adc_offset[2] = 33000;
    conf.adc_offset[3] = 34000;

    strcpy(conf.filePath,"/tmp/data");
    conf.filePointSkip = 1;

    conf.writeEnabled[0] = true;
    conf.writeEnabled[1] = true;
    conf.writeEnabled[2] = true;
    conf.writeEnabled[3] = true;

    conf.pollcount = 10000;

    return 0;
}

unsigned int Sis3350control::configure()
{
    unsigned int ret = 0x0;

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

unsigned int Sis3350control::configureControlStatus()
{
    unsigned int ret;

    // Set Control Status register
    addr = conf.base_addr + SIS3350_CONTROL_STATUS;
    data = 0;
    if(conf.ctrl_lemo_invert) {
        data += SIS3350_CTRL_ENABLE_LEMO_IN_INVERT;
    }
    data += SIS3350_CTRL_ENABLE_USER_LED;
    ret = vme_A32D32_write(m_device,addr,data);
    if(ret != 0) printf("Error %d at SET STATUS\n",ret);

    return ret;
}

unsigned int Sis3350control::configureAcquisitionMode()
{
    unsigned int ret;

    // Setup acquisition mode
    addr = conf.base_addr + SIS3350_ACQUISITION_CONTROL;
    data = 0;
    data += conf.acq_mode;
    if(conf.acq_enable_lemo) {
        data += SIS3350_ACQ_ENABLE_LEMO_TRIGGER;
    }
    else {
        data += SIS3350_ACQ_ENABLE_INTERNAL_TRIGGER;
    }
    if(conf.acq_multi_event) {
        data += SIS3350_ACQ_ENABLE_MULTIEVENT;
    }
    ret = vme_A32D32_write(m_device,addr,data);
    if(ret != 0) printf("Error %d at SET ACQUISITION MODE\n",ret);

    return ret;
}

unsigned int Sis3350control::configureExternalDAC()
{
    unsigned int ret;

    // Ext DAC Trigger setup
    addr = conf.base_addr + SIS3350_EXT_CLOCK_TRIGGER_DAC_CONTROL_STATUS;
    data = (conf.ext_clock_trigger_dac_control << 4);
    ret = vme_A32D32_write(m_device,addr,data);
    if(ret != 0) printf("Error %d at EXT DAC TRG SETUP\n",ret);

    // Ext DAC Threshold
    addr = conf.base_addr + SIS3350_EXT_CLOCK_TRIGGER_DAC_DATA;
    data = (conf.ext_clock_trigger_daq_data & 0xffff);
    ret = vme_A32D32_write(m_device,addr,data);
    if(ret != 0) printf("Error %d at EXT DAC THR\n",ret);

    return ret;
}

unsigned int Sis3350control::configureDirectMemory()
{
    unsigned int ret;

    // Direct Memory Stop Mode Sample Wrap Length (only STOP mode)
    addr = conf.base_addr + SIS3350_DIRECT_MEMORY_SAMPLE_WRAP_LENGTH_ALL_ADC;
    data = (conf.direct_mem_wrap_length & 0x07FFFFF8);
    ret = vme_A32D32_write(m_device,addr,data);
    if(ret != 0) printf("Error %d at DIRECT MEM STOP MODE SAMPLE WRAP LENGTH SETUP\n",ret);

    // Direct Memory Sample Length (only START mode)
    addr = conf.base_addr + SIS3350_DIRECT_MEMORY_SAMPLE_LENGTH;
    data = (conf.direct_mem_sample_length & 0x07FFFFF8);
    ret = vme_A32D32_write(m_device,addr,data);
    if(ret != 0) printf("Error %d at DIRECT MEM SAMPLE LENGTH SETUP\n",ret);

    // Direct Memory Pretrigger Length
    addr = conf.base_addr + SIS3350_TRIGGER_DELAY;
    data = (conf.direct_mem_pretrigger_length & 0x03FFFFFE);
    ret = vme_A32D32_write(m_device,addr,data);
    if(ret != 0) printf("Error %d at DIRECT MEM PRETRIGGER LENGTH SETUP\n",ret);

    return ret;
}

unsigned int Sis3350control::configureInputGains()
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
        ret = vme_A32D32_write(m_device,addr,data);
        if(ret != 0) printf("Error %d at ADC GAIN %d\n",ret,i);

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

        sis3350_write_dac_offset(conf.base_addr + SIS3350_ADC12_DAC_CONTROL_STATUS, 0, conf.adc_offset[0] );
        sis3350_write_dac_offset(conf.base_addr + SIS3350_ADC12_DAC_CONTROL_STATUS, 1, conf.adc_offset[1] );
        sis3350_write_dac_offset(conf.base_addr + SIS3350_ADC34_DAC_CONTROL_STATUS, 0, conf.adc_offset[2] );
        sis3350_write_dac_offset(conf.base_addr + SIS3350_ADC34_DAC_CONTROL_STATUS, 1, conf.adc_offset[3] );
    }

    return ret;
}

unsigned int Sis3350control::configureMultiEventRegister()
{
    unsigned int ret;

    // Setup multi event register
    addr = conf.base_addr + SIS3350_MULTIEVENT_MAX_NOF_EVENTS;
    data = conf.multievent_max_nof_events;
    ret = vme_A32D32_write(m_device,addr,data);
    if(ret != 0) printf("Error %d at SETUP MULTIEVENT\n",ret);

    return ret;
}

unsigned int Sis3350control::configureRingBuffer()
{
    unsigned int ret;

    // Set ringbuffer length
    addr = conf.base_addr + SIS3350_RINGBUFFER_SAMPLE_LENGTH_ALL_ADC;
    ret = vme_A32D32_write(m_device,addr,conf.sample_length);
    if(ret != 0) printf("Error %d at SET RB LENGTH\n",ret);

    // Ringbuffer pre-delay
    addr = conf.base_addr + SIS3350_RINGBUFFER_PRE_DELAY_ALL_ADC;
    ret = vme_A32D32_write(m_device,addr,conf.pre_delay);
    if(ret != 0) printf("Error %d at SET RB PREDELAY\n",ret);

    return ret;
}

unsigned int Sis3350control::configureClock()
{
    unsigned int ret;

    // Set frequency synthesizer
    addr = conf.base_addr + SIS3350_FREQUENCE_SYNTHESIZER;
    data = (conf.clock2 & 3) << 9;
    data |= (conf.clock1 & 0x1FF);
    ret = vme_A32D32_write(m_device,addr,data);
    if(ret != 0) printf("Error %d at SET CLOCK\n",ret);

    return ret;
}

unsigned int Sis3350control::configureTriggers()
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
        ret = vme_A32D32_write(m_device,addr,data);
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
        ret = vme_A32D32_write(m_device,addr,data);
        if(ret != 0) printf("Error %d at TRIGGER THR %d\n",ret,i);

    }

    return ret;
}

int Sis3350control::acquisition()
{
    //! ACQUISITION LOOP //////////////////////////////////////////////////
    //! ###################################################################
    //! ###################################################################
    //  - Do this in separate thread
    //  - Write data to buffer using QSemaphore

    uint32_t i,j;
    uint32_t stop_next_sample_addr[4], read_data_block_length[4];
    uintptr_t read_data_block_ptr;
    uint32_t got_nof_lwords;
    int ret = 0;
    uint32_t pollcount = 0;
    bool triggered = false;

    uint32_t sample_length;

    if(conf.acq_mode == SIS3350_ACQ_OPERATION_RINGBUFFER_SYNCH) sample_length = conf.sample_length;
    else sample_length = conf.direct_mem_sample_length;

    for(i=0;i<conf.nof_reads;i++)
    {
        //printf("<%d>\n",i);fflush(stdout);

        //! POLL //////////////////////////////////////////////////////
        //! ###########################################################

        // Arm the device
        addr = conf.base_addr + SIS3350_KEY_ARM;
        ret = vme_A32D32_write(m_device,addr,0);
        if(ret != 0) printf("Error %d at KEY ARM\n",ret);


        // Poll until triggered
        pollcount = 0;
        triggered = true;
        addr = conf.base_addr + SIS3350_ACQUISITION_CONTROL;
        do
        {
            pollcount++;
            data = 0;
            if(pollcount == conf.pollcount*conf.multievent_max_nof_events)
            {
                triggered = false;
                break;
            }
            ret = vme_A32D32_read(m_device,addr,&data);
            if(ret != 0) printf("Error %d at VME READ POLL\n",ret);

        } while((data & SIS3350_ACQ_STATUS_ARMED_FLAG) == SIS3350_ACQ_STATUS_ARMED_FLAG);

        if(!triggered)
        {
            printf("No trigger in %d polls.\n",pollcount);
            continue;
        }

        // Read Multievent counter
        addr = conf.base_addr + SIS3350_MULTIEVENT_EVENT_COUNTER;
        ret = vme_A32D32_read(m_device,addr,&data);
        if(ret != 0) printf("Error %d at VME MULTIEVENT COUNTER\n",ret);



        // Read all four channels
        for(j=0;j<4;j++)
        {

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
            ret = vme_A32D32_read(m_device,addr,&data);
            if(ret != 0) printf("Error %d at VME READ ACTUAL SAMPLE ADDR 1\n",ret);
            stop_next_sample_addr[j] = data;
//          printf("Stop next sample address 0x%08x\n",stop_next_sample_addr[j]);



            // Check buffer length
            if(stop_next_sample_addr[j] > 2*MAX_NOF_LWORDS)
            {
                    printf("Buffer length too short!\n");
                    exit(1);
            }



            // Read data buffer
            read_data_block_ptr = (uintptr_t) rblt_data[j];

            if(stop_next_sample_addr[j] != 0)
            {
                    sis3350_DMA_Read_MBLT64_ADC_DataBuffer(conf.base_addr,
                            j,0x0,stop_next_sample_addr[j],&got_nof_lwords,
                            (uint32_t *)read_data_block_ptr);
                    read_data_block_length[j] = got_nof_lwords;
            }
            else
            {
                    printf("<%08d> ADC%d No data read!\n",i,j);
                    read_data_block_length[j] = 0;
                    continue;
            }

            if(read_data_block_length[j] != (sample_length/2+4)*conf.multievent_max_nof_events)
            {
                    printf("<%08d> ADC%d Data block length was too short for amount of events.\n",i,j);
                    printf("0x%08x, 0x%08x\n",read_data_block_length[j],
                                    (sample_length/2+4)*conf.multievent_max_nof_events);
            }



        }

        if(conf.writeEnabled[0] ||
           conf.writeEnabled[1] ||
           conf.writeEnabled[2] ||
           conf.writeEnabled[3])
        {
            fileWrite(i);
        }

    }

    return ret;
}

int Sis3350control::setupQDCspectrum()
{
    int ret = 0;

    qdcSpectrum = new QMap<double,unsigned int>;

    return ret;
}

int Sis3350control::fileWrite(int i)
{
    //! Write to disk /////////////////////////////////////////////
    //! ###########################################################
    //  - Write to buffer first, implemented with QSemaphore
    //  - Do file i/o in separate thread


    char fileName[300];
    sprintf(fileName,"%s/multi%08d.dat",conf.filePath,i+1);
    fHandle = fopen(fileName,"w");
    if(fHandle == NULL) {printf("File %s not opened.\n",fileName); return -1;}
    unsigned int ev=0,j=0,ch=0;
    int holdoff = conf.filePointSkip;

    // Write data buffer to file
    for(ev=0; ev < conf.multievent_max_nof_events; ev++)
    {
        unsigned int sample_length;
        if(conf.acq_mode == SIS3350_ACQ_OPERATION_RINGBUFFER_SYNCH) sample_length = conf.sample_length;
        else sample_length = conf.direct_mem_sample_length;

        unsigned int sample_start = (ev*(sample_length/2+4));

        for(ch=0;ch<4;ch++)
        {
            if(conf.writeEnabled[ch])
            {
                int hoCnt = 0;

                // Print time stamp
                if(ev % 1000 == 0) printf("<%05d,%05d> ADC %d Time: 0x%8.8x 0x%8.8x, Info: 0x%8.8x 0x%8.8x\n",i,ev,ch,
                    rblt_data[ch][sample_start+0],
                    rblt_data[ch][sample_start+1],
                    rblt_data[ch][sample_start+2],
                    rblt_data[ch][sample_start+3]);

                unsigned long int timestamp = ((rblt_data[ch][sample_start+0] & 0x0fff0000) << 20) |
                        ((rblt_data[ch][sample_start+0] & 0x00000fff)<< 24) |
                        ((rblt_data[ch][sample_start+1] & 0x0fff0000) >> 4) |
                        ((rblt_data[ch][sample_start+1] & 0x00000fff));
                fprintf(fHandle,"# Ch: %d S: %u T: %lu\n",ch,(i*conf.multievent_max_nof_events)+ev+1,timestamp);
                for(j=sample_start+4;j<((ev+1)*(sample_length/2+4));j++)
                {
                    unsigned int samplelow = (rblt_data[ch][j] & 0x0000ffff);
                    unsigned int samplehigh = (rblt_data[ch][j] & 0xffff0000) >> 16;

                    if(hoCnt == 0) fprintf(fHandle,"%d\n",samplelow);
                    hoCnt++;
                    if(hoCnt == holdoff) hoCnt = 0;

                    if(hoCnt == 0) fprintf(fHandle,"%d\n",samplehigh);
                    hoCnt++;
                    if(hoCnt == holdoff) hoCnt = 0;
                }
            }
        }
    }
    fclose(fHandle);

    return 0;
}


// ######## Post processing functions ##########################

vector<vector<vector<double> > > Sis3350control::byteArrayToVector()
{
    // Transform the bytearray into vectors
//    printf("Building vector from bytearray\n");
    vector<vector<vector<double> > > vdata;
    for(unsigned int ev=0; ev < conf.multievent_max_nof_events; ev++)
    {
        unsigned int sample_length = conf.sample_length;
        unsigned int sample_start = (ev*(sample_length/2+4));

        vector<vector<double> > sngEv(4,vector<double>(sample_length,0));

        for(int ch = 0; ch < 4; ch++)
        {
            int curSmp = 0;
            for(unsigned int j=sample_start+4;j<((ev+1)*(sample_length/2+4));j++)
            {
//                printf("<%d><%d><%d> %d/%d\n",ev,ch,j,curSmp,sngEv.at(0).size());
                unsigned int samplelow = (rblt_data[ch][j] & 0x0000ffff);
                sngEv[ch][curSmp] = samplelow;
                curSmp++;
                unsigned int samplehigh = (rblt_data[ch][j] & 0xffff0000) >> 16;
                sngEv[ch][curSmp] = samplehigh;
                curSmp++;
            }
        }
        vdata.push_back(sngEv);
        sngEv.clear();
    }

//    printf("done\n");


    return vdata;
}




// ######## Struck implemented functions #######################

int Sis3350control::sub_vme_A32D32_read (unsigned int vme_adr, unsigned int* vme_data)
{
        int error;
        error = vme_A32D32_read(m_device, vme_adr, vme_data)  ;
        return error ;
}

int Sis3350control::sub_vme_A32D32_write (unsigned int vme_adr, unsigned int vme_data)
{
        int error;
        error = vme_A32D32_write(m_device, vme_adr, vme_data)  ;
        return error ;
}

int Sis3350control::sub_vme_A32BLT32_read (unsigned int vme_adr, unsigned int* dma_buffer,
                                           unsigned int request_nof_words, unsigned int* got_nof_words)
{
        int error;
        error = sis3150Usb_Vme_Dma_Read(m_device, vme_adr, 0xf, 4, 0,
                                        (uint32_t *)dma_buffer, request_nof_words,
                                        (uint32_t *)got_nof_words);
        return error ;
}

int Sis3350control::sub_vme_A32MBLT64_read (unsigned int vme_adr, unsigned int* dma_buffer,
                                            unsigned int request_nof_words, unsigned int* got_nof_words)
{
        int error;
        error = sis3150Usb_Vme_Dma_Read(m_device, vme_adr, 0x8, 8, 0,
                                        (uint32_t *)dma_buffer, request_nof_words & 0xfffffffe,
                                        (uint32_t *)got_nof_words);
        return error ;
}

int Sis3350control::sis3350_write_dac_offset(unsigned int module_dac_control_status_addr,
                                             unsigned int dac_select_no, unsigned int dac_value )
{
        unsigned int error;
        unsigned int data, addr;
        unsigned int max_timeout, timeout_cnt;



                data =  dac_value ;
                addr = module_dac_control_status_addr + 4 ; // DAC_DATA
                if ((error = sub_vme_A32D32_write(addr,data )) != 0) {
                        //sisVME_ErrorHandling (error, addr, "sub_vme_A32D32_write");
                        return -1;
                }

                data =  1 + (dac_select_no << 4); // write to DAC Register
                addr = module_dac_control_status_addr ;
                if ((error = sub_vme_A32D32_write(addr,data )) != 0) {
                        //sisVME_ErrorHandling (error, addr, "sub_vme_A32D32_write");
                        return -1;
                }

                max_timeout = 5000 ;
                timeout_cnt = 0 ;
                addr = module_dac_control_status_addr  ;
                do {
                        if ((error = sub_vme_A32D32_read(addr,&data )) != 0) {
                                //sisVME_ErrorHandling (error, addr, "sub_vme_A32D32_read");
                                return -1;
                        }
                        timeout_cnt++;
                } while ( ((data & 0x8000) == 0x8000) && (timeout_cnt <  max_timeout) )    ;

                if (timeout_cnt >=  max_timeout) {
                        return -2 ;
                }



                data =  2 + (dac_select_no << 4); // Load DACs
                addr = module_dac_control_status_addr  ;
                if ((error = sub_vme_A32D32_write(addr,data )) != 0) {
                        //sisVME_ErrorHandling (error, addr, "sub_vme_A32D32_write");
                        return -1;
                }
                timeout_cnt = 0 ;
                addr = module_dac_control_status_addr  ;
                do {
                        if ((error = sub_vme_A32D32_read(addr,&data )) != 0) {
                                //sisVME_ErrorHandling (error, addr, "sub_vme_A32D32_read");
                                return -1;
                        }
                        timeout_cnt++;
                } while ( ((data & 0x8000) == 0x8000) && (timeout_cnt <  max_timeout) )    ;

                if (timeout_cnt >=  max_timeout) {
                        return -3 ;
                }



        return 0x0 ;

}

int Sis3350control::sis3350_DMA_Read_MBLT64_ADC_DataBuffer(unsigned int module_address,
        unsigned int adc_channel /* 0 to 3 */,
        unsigned int adc_buffer_sample_start_addr,
        unsigned int adc_buffer_sample_length,   // 16-bit words
        unsigned int* dma_got_no_of_words,
        unsigned int* uint_adc_buffer)
{
        int return_code ;
        unsigned int data ;
        unsigned int addr, req_nof_lwords, got_nof_lwords ;
        unsigned int dma_request_nof_lwords, dma_adc_addr_offset_bytes ;
        unsigned int max_page_sample_length, page_sample_length_mask ;
        unsigned int next_event_sample_start_addr ;
        unsigned int rest_event_sample_length ;
        unsigned int sub_event_sample_length ;
        unsigned int sub_event_sample_addr ;
        unsigned int sub_max_page_sample_length ;
        unsigned int sub_page_addr_offset ;
        unsigned int index_num_data ;

        max_page_sample_length  = SIS3350_MAX_PAGE_SAMPLE_LENGTH ; // 0x8000000
        page_sample_length_mask = max_page_sample_length - 1 ;

        next_event_sample_start_addr = (adc_buffer_sample_start_addr & 0x07fffffc)  ; // max 128 MSample  256MByte
        rest_event_sample_length     = (adc_buffer_sample_length & 0xfffffffc)      ; //

        if (rest_event_sample_length  >= SIS3350_MAX_SAMPLE_LENGTH)
        {
            rest_event_sample_length = SIS3350_MAX_SAMPLE_LENGTH ; // 0x8000000 max 128 MSample
        }



        return_code = 0 ;
        index_num_data = 0x0 ;
        do {
            sub_event_sample_addr = (next_event_sample_start_addr & page_sample_length_mask) ;
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
#endif

            dma_request_nof_lwords     =  (sub_event_sample_length) / 2  ; // Lwords
            dma_adc_addr_offset_bytes  =  (sub_event_sample_addr) * 2    ; // Bytes

            // set page
            addr = module_address + SIS3350_ADC_MEMORY_PAGE_REGISTER  ;
            data = sub_page_addr_offset ;
            if ((return_code = sub_vme_A32D32_write(addr,data )) != 0)
            {
                printf("Error at ADC MEM PAGE REG Setup\n");
                return -1;
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
            printf("\n");
#endif


            return_code = sub_vme_A32MBLT64_read(addr,
                                                 &uint_adc_buffer[index_num_data],
                                                 req_nof_lwords,
                                                 &got_nof_lwords);

            if(req_nof_lwords != got_nof_lwords)
            {
                    printf("Length Error sub_vme_A32MBLT64_read:   req_nof_lwords = 0x%08x  got_nof_lwords = 0x%08x\n", req_nof_lwords, got_nof_lwords );
                    return -1;
            }
            index_num_data = index_num_data + got_nof_lwords ;

            next_event_sample_start_addr =  next_event_sample_start_addr + sub_event_sample_length;
            rest_event_sample_length     =  rest_event_sample_length - sub_event_sample_length;

        } while ((return_code == 0) && (rest_event_sample_length>0)) ;

        *dma_got_no_of_words =   index_num_data ;
        return 0;
}
