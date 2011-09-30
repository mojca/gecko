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

#include "sis3302module_gamma_v1410.h"
#include "modulemanager.h"
#include "eventbuffer.h"

/******************************** Struck SIS3302 8 ch FADC *****************************
 ********************************   GAMMA FIRMWARE 1410    *****************************
 *
 *
 * Some notes about configuration:
 *
 * - Three modes are generally supported:
 *   - Single event
 *   - Multi event
 *   - MCA mode (histogramming)
 *
 *
 * Note on the DAC settings:
 *
 * - The power on default setting of 0 is way too low
 * - To have a 0 V signal about in the middle of the acceptance range set DAC to 37000
 *
 *
 ***************************************************************************************/

static ModuleRegistrar registrar (Sis3302V1410Module::MODULE_NAME, Sis3302V1410Module::create);

Sis3302V1410Module::Sis3302V1410Module(int _id, QString _name)
    : BaseModule(_id, _name)
    , addr(0)
    , data(0)
    , dmx (evslots)
{   
    init();

    setUI (new Sis3302V1410UI(this));

    setChannels();
    createOutputPlugin();
    getUI()->applySettings();

    std::cout << "Instantiated " << MODULE_NAME << " Module" << std::endl;
}

const char* Sis3302V1410Module::MODULE_NAME = "sis3302_gamma_v1410";

Sis3302V1410Module::~Sis3302V1410Module()
{
    //delete buffer;
}

void Sis3302V1410Module::init()
{
    for (int ch=0; ch<NOF_CHANNELS; ch++)
    {
        currentTimestamp[ch] = 0ULL;
        currentHeader[ch] = 0;
        currentTriggerCounter[ch] = 0;
        currentEnergyMaxValue[ch] = 0;
        currentEnergyFirstValue[ch] = 0;
        currentPileupFlag[ch] = false;
        currentRetriggerFlag[ch] = false;
    }
}

void Sis3302V1410Module::setChannels()
{
    EventBuffer *evb = RunManager::ref ().getEventBuffer ();
    // Setup channels
    for (int ch=0; ch<NOF_CHANNELS; ch++)
    {
        evslots << evb->registerSlot (this, tr("Raw trace %1").arg(ch),
                                      PluginConnector::VectorUint32);
    }
    for (int ch=0; ch<NOF_CHANNELS; ch++)
    {
        evslots << evb->registerSlot (this, tr("Energy trace %1").arg(ch),
                                      PluginConnector::VectorUint32);
    }
    for (int ch=0; ch<NOF_CHANNELS; ch++)
    {
        evslots << evb->registerSlot (this, tr("Energy value %1").arg(ch),
                                      PluginConnector::VectorDouble);
    }

    evslots << evb->registerSlot (this, tr("Meta"),
                                  PluginConnector::VectorUint32);
}

int Sis3302V1410Module::configure()
{   
    printf("%s::configure\n",MODULE_NAME);
    int ret = 0x0;
    AbstractInterface *iface = getInterface ();

    ret = checkConfig();

    updateEndAddrThresholds();

    // Set Control Status register
    addr = conf.base_addr + SIS3302_V1410_CONTROL_STATUS;
    data = 0;
    if(conf.enable_user_led) data |= SIS3302_V1410_FCT_CTRL_LED_ON;
    else data |= SIS3302_V1410_FCT_CTRL_LED_OFF;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_V1410_CONTROL_STATUS\n",ret);

    // Set IRQ config register
    addr = conf.base_addr + SIS3302_V1410_IRQ_CONFIG;
    data = 0;
    if(conf.irqMode == conf.modeRoak)
        data |= (conf.modeRoak << SIS3302_V1410_OFF_IRQ_CFG_MODE);
    if(conf.enable_irq == true)
        data |= (1 << SIS3302_V1410_OFF_IRQ_CFG_ENAB);
    data |= (conf.irq_level << SIS3302_V1410_OFF_IRQ_CFG_LVL) & SIS3302_V1410_MSK_IRQ_CFG_LVL;
    data |= (conf.irq_vector) & SIS3302_V1410_MSK_IRQ_CFG_VECT;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_V1410_OFF_IRQ_CFG_MODE\n",ret);

    // Set IRQ control register
    addr = conf.base_addr + SIS3302_V1410_IRQ_CONTROL;
    data = 0;
    data |= SIS3302_V1410_FCT_IRQ_CTRL_ENABLE_SRC(conf.irqSource); // Enable the correct irq source
    data |= SIS3302_V1410_FCT_IRQ_CTRL_DISABLE_OTHER_SRC(conf.irqSource); // Disable all the others
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_V1410_IRQ_CONTROL\n",ret);

    // Set acquisition control register
    addr = conf.base_addr + SIS3302_V1410_ACQUISITION_CONTROL;
    data = 0;
    data |= (conf.lemo_in_mode << SIS3302_V1410_OFF_ACQ_CTRL_LEMO_IN_MODE);
    data |= (conf.lemo_out_mode << SIS3302_V1410_OFF_ACQ_CTRL_LEMO_OUT_MODE);
    for(int i=0; i<3; i++) {
        if(conf.enable_lemo_in[i])
            data |= (1 << (SIS3302_V1410_OFF_ACQ_CTRL_LEMO_IN_ENABLE + (2-i)));
        else
            data |= (1 << (SIS3302_V1410_OFF_ACQ_CTRL_LEMO_IN_DISABLE + (2-i)));
    }
    if(conf.send_int_trg_to_ext_as_or) data |= SIS3302_V1410_MSK_ACQ_CTRL_TRG_OR_STAT;
    else data &= (~SIS3302_V1410_MSK_ACQ_CTRL_TRG_OR_STAT);
    data |= (conf.clockSource << SIS3302_V1410_OFF_ACQ_CTRL_CLOCK_SOURCE) & SIS3302_V1410_MSK_ACQ_CTRL_CLOCK_SOURCE;
#if defined(SIS3302_V1410_MCA_MODE)
    data |= SIS3302_V1410_MSK_ACQ_CTRL_MCA_ENAB;
#endif
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_V1410_ACQUISITION_CONTROL\n",ret);

    // Setup DACs
    write_dac_offset(&(conf.dac_offset[0]));

    // Setup event config for single channels
    for(int i=0; i<NOF_ADC_GROUPS; ++i) {
        addr = conf.base_addr + SIS3302_V1410_EVENT_CONFIG(i);
        data = 0;
        int even_i = i*2+1;
        int odd_i = i*2;
        // Odd channels
        if(conf.enable_input_invert[odd_i])     data |= SIS3302_V1410_MSK_EVENT_CFG_ODD_INVERT;
        if(conf.enable_int_trg[odd_i])          data |= SIS3302_V1410_MSK_EVENT_CFG_ODD_INT_TRG_ENAB;
        if(conf.enable_ext_trg[odd_i])          data |= SIS3302_V1410_MSK_EVENT_CFG_ODD_EXT_TRG_ENAB;
        if(conf.enable_int_gate[odd_i])         data |= SIS3302_V1410_MSK_EVENT_CFG_ODD_INT_GATE_ENAB;
        if(conf.enable_ext_gate[odd_i])         data |= SIS3302_V1410_MSK_EVENT_CFG_ODD_EXT_GATE_ENAB;
        if(conf.enable_prev_adc_gate[odd_i])    data |= SIS3302_V1410_MSK_EVENT_CFG_ODD_NMINUS_GATE_ENAB;
        if(conf.enable_next_adc_gate[odd_i])    data |= SIS3302_V1410_MSK_EVENT_CFG_ODD_NPLUS_GATE_ENAB;
        // Even channels
        if(conf.enable_input_invert[even_i])    data |= SIS3302_V1410_MSK_EVENT_CFG_EVEN_INVERT;
        if(conf.enable_int_trg[even_i])         data |= SIS3302_V1410_MSK_EVENT_CFG_EVEN_INT_TRG_ENAB;
        if(conf.enable_ext_trg[even_i])         data |= SIS3302_V1410_MSK_EVENT_CFG_EVEN_EXT_TRG_ENAB;
        if(conf.enable_int_gate[even_i])        data |= SIS3302_V1410_MSK_EVENT_CFG_EVEN_INT_GATE_ENAB;
        if(conf.enable_ext_gate[even_i])        data |= SIS3302_V1410_MSK_EVENT_CFG_EVEN_EXT_GATE_ENAB;
        if(conf.enable_prev_adc_gate[even_i])   data |= SIS3302_V1410_MSK_EVENT_CFG_EVEN_NMINUS_GATE_ENAB;
        if(conf.enable_next_adc_gate[even_i])   data |= SIS3302_V1410_MSK_EVENT_CFG_EVEN_NPLUS_GATE_ENAB;
        // Header ID
        data |= (conf.header_id[i] << SIS3302_V1410_OFF_EVENT_CFG_HEADER_ID);
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_V1410_EVENT_CONFIG(%d)\n",ret,i);
    }

    // Setup extended event config for single channels
    for(int i=0; i<NOF_ADC_GROUPS; ++i) {
        addr = conf.base_addr + SIS3302_V1410_EVENT_CFG_EXTENDED(i);
        data = 0;
        int even_i = i*2+1;
        int odd_i = i*2;
        // Odd channels
#if defined(SIS3302_V1410_MCA_MODE)
        if(conf.enable_50kHz_trigger[odd_i]) data |= SIS3302_V1410_MSK_EVENT_CFG_EXT_ODD_TRG_50KHZ;
#endif
        if(conf.enable_prev_adc_trg[odd_i])  data |= SIS3302_V1410_MSK_EVENT_CFG_EXT_ODD_NMINUS_TRG_EN;
        if(conf.enable_next_adc_trg[odd_i])  data |= SIS3302_V1410_MSK_EVENT_CFG_EXT_ODD_NPLUS_TRG_EN;
        // Even channels
#if defined(SIS3302_V1410_MCA_MODE)
        if(conf.enable_50kHz_trigger[even_i]) data |= SIS3302_V1410_MSK_EVENT_CFG_EXT_EVEN_TRG_50KHZ;
#endif
        if(conf.enable_prev_adc_trg[even_i])  data |= SIS3302_V1410_MSK_EVENT_CFG_EXT_EVEN_NMINUS_TRG_EN;
        if(conf.enable_next_adc_trg[even_i])  data |= SIS3302_V1410_MSK_EVENT_CFG_EXT_EVEN_NPLUS_TRG_EN;
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_V1410_EVENT_CFG_EXTENDED(%d)\n",ret,i);
    }

    // Setup end address threshold
    for(int i=0; i<NOF_ADC_GROUPS; ++i) {
        addr = conf.base_addr + SIS3302_V1410_END_ADDRESS_THR(i);
        data = (conf.end_addr_thr_in_samples[i] & SIS3302_V1410_MSK_END_ADDR_THR);
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_V1410_END_ADDRESS_THR(%d)\n",ret,i);
    }

    // Setup pretrigger and gate
    for(int i=0; i<NOF_ADC_GROUPS; ++i) {
        int tgl = conf.trigger_gate_length[i]-1;
        int tpd = conf.trigger_pretrigger_delay[i];
        if(tpd > 1021) tpd = tpd-1022;
        else tpd = tpd+2;
        addr = conf.base_addr + SIS3302_V1410_PRETRIGGER_AND_GATE(i);
        data = 0;
        data |= (((tgl)) & SIS3302_V1410_MSK_TRG_GATE_LEN);
        data |= (((tpd) << SIS3302_V1410_OFF_PRETRIGGER_DELAY) & SIS3302_V1410_MSK_PRETRIGGER_DELAY);
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_V1410_PRETRIGGER_AND_GATE(%d)\n",ret,i);
    }

    // Setup raw buffer
    for(int i=0; i<NOF_ADC_GROUPS; ++i) {
        addr = conf.base_addr + SIS3302_V1410_RAW_BUFFER_CONFIG(i);
        data = 0;
        data |= ((conf.raw_data_sample_start_idx[i]) & SIS3302_V1410_MSK_RAW_BUF_START_IDX);
        data |= ((conf.raw_sample_length[i] << SIS3302_V1410_OFF_RAW_BUF_SAMPLE_LEN) & SIS3302_V1410_MSK_RAW_BUF_SAMPLE_LEN);
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_V1410_RAW_BUFFER_CONFIG(%d)\n",ret,i);
    }

    // Setup trigger and energy registers
    for (int i=0; i<NOF_CHANNELS; ++i) {
        uint32_t thr;

        // Trigger setup
        addr = conf.base_addr + SIS3302_V1410_TRG_SETUP(i);
        data = 0;
        data |= ((conf.trigger_peak_length[i]) & SIS3302_V1410_MSK_TRG_SETUP_PEAK_TIME);
        data |= ((conf.trigger_sumg_length[i] << SIS3302_V1410_OFF_TRG_SETUP_SUMG_TIME) & SIS3302_V1410_MSK_TRG_SETUP_SUMG_TIME);
        data |= ((conf.trigger_pulse_length[i] << SIS3302_V1410_OFF_TRG_SETUP_PULSE_LEN) & SIS3302_V1410_MSK_TRG_SETUP_PULSE_LEN);
        data |= ((conf.trigger_int_gate_length[i] << SIS3302_V1410_OFF_TRG_SETUP_GATE_LEN) & SIS3302_V1410_MSK_TRG_SETUP_GATE_LEN);
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_V1410_TRG_SETUP(%d)",ret,i);

        // Trigger extended setup
        addr = conf.base_addr + SIS3302_V1410_TRG_EXTENDED(i);
        data = 0;
        data |= ( conf.trigger_peak_length[i] >> 8) & SIS3302_V1410_MSK_TRG_EXT_SETUP_PEAK_89;
        data |= (((conf.trigger_sumg_length[i] >> 8) << SIS3302_V1410_OFF_TRG_EXT_SETUP_SUMG_89) & SIS3302_V1410_MSK_TRG_EXT_SETUP_SUMG_89);
        data |= ((conf.trigger_decim_mode[i] << SIS3302_V1410_OFF_TRG_EXT_SETUP_TRG_DECIM) & SIS3302_V1410_MSK_TRG_EXT_SETUP_TRG_DECIM);
        data |= ((conf.trigger_int_trg_delay[i] << SIS3302_V1410_OFF_TRG_EXT_SETUP_TRG_DELAY) & SIS3302_V1410_MSK_TRG_EXT_SETUP_TRG_DELAY);
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_V1410_TRG_EXTENDED(%d)",ret,i);

        // Trigger threshold
        thr = conf.trigger_threshold[i] + SIS3302_V1410_TRG_THR_ADDED_CONST;
        addr = conf.base_addr + SIS3302_V1410_TRG_THR(i);
        data = 0;
        data |= ((thr) & SIS3302_V1410_MSK_TRG_THR_VALUE);
        if(conf.enable_ch[i]) data |= (1 << SIS3302_V1410_OFF_TRG_ENABLE);
        if(conf.disable_trg_out[i]) data |= (1 << SIS3302_V1410_OFF_TRG_THR_DISABLE_OUT);
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_V1410_TRG_THR(%d)",ret,i);

        // Energy setup
        addr = conf.base_addr + SIS3302_V1410_ENERGY_SETUP_GP(i);
        data = 0;
        data |= ((conf.energy_peak_length[i]) & SIS3302_V1410_MSK_ENERGY_SETUP_PEAK);
        data |= ((conf.energy_sumg_length[i] << SIS3302_V1410_OFF_ENERGY_SETUP_SUMG) & SIS3302_V1410_MSK_ENERGY_SETUP_SUMG);
        data |= (((conf.energy_peak_length[i] >> 8) << SIS3302_V1410_OFF_ENERGY_SETUP_PEAK_89) & SIS3302_V1410_MSK_ENERGY_SETUP_PEAK);
        data |= ((conf.energy_decim_mode[i] << SIS3302_V1410_OFF_ENERGY_SETUP_DECIM) & SIS3302_V1410_MSK_ENERGY_SETUP_DECIM);
        if(conf.enable_energy_extra_filter[i]) data |= ((1 << SIS3302_V1410_OFF_ENERGY_SETUP_EXTRA_FILTER) & SIS3302_V1410_MSK_ENERGY_SETUP_EXTRA_FILTER);
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_V1410_ENERGY_SETUP_GP(%d)",ret,i);

        // Energy gate length setup
        addr = conf.base_addr + SIS3302_V1410_ENERGY_GATE_LEN(i);
        data = 0;
        data |= ((conf.energy_gate_length[i]) & SIS3302_V1410_MSK_ENERGY_GATE_LEN);
        data |= ((conf.energy_test_mode[i] << SIS3302_V1410_OFF_ENERGY_TEST_MODE) & SIS3302_V1410_MSK_ENERGY_TEST_MODE);
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_V1410_ENERGY_GATE_LEN(%d)",ret,i);

        // Energy sample length setup
        addr = conf.base_addr + SIS3302_V1410_ENERGY_SAMPLE_LEN(i);
        data = 0;
        data |= ((conf.energy_sample_length[i]) & SIS3302_V1410_MSK_ENERGY_SAMPLE_LEN);
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_V1410_ENERGY_SAMPLE_LEN(%d)",ret,i);

        // Energy sample start idx1 setup
        addr = conf.base_addr + SIS3302_V1410_ENERGY_START_IDX1(i);
        data = 0;
        data |= ((conf.energy_sample_start_idx[i][0]) & SIS3302_V1410_MSK_ENERGY_SAMPLE_START_IDX);
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_V1410_ENERGY_START_IDX1(%d)",ret,i);

        // Energy sample start idx2 setup
        addr = conf.base_addr + SIS3302_V1410_ENERGY_START_IDX2(i);
        data = 0;
        data |= ((conf.energy_sample_start_idx[i][1]) & SIS3302_V1410_MSK_ENERGY_SAMPLE_START_IDX);
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_V1410_ENERGY_START_IDX2(%d)",ret,i);

        // Energy sample start idx3 setup
        addr = conf.base_addr + SIS3302_V1410_ENERGY_START_IDX3(i);
        data = 0;
        data |= ((conf.energy_sample_start_idx[i][2]) & SIS3302_V1410_MSK_ENERGY_SAMPLE_START_IDX);
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_V1410_ENERGY_START_IDX3(%d)",ret,i);

        // Energy tau setup
        addr = conf.base_addr + SIS3302_V1410_ENERGY_TAU(i);
        data = 0;
        data |= ((conf.energy_tau[i]) & SIS3302_V1410_MSK_ENERGY_TAU);
        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_V1410_ENERGY_TAU(%d)",ret,i);
    }

    resetSampleLogic();

    return ret;
}

//! Check, if configuration is sane
int Sis3302V1410Module::checkConfig()
{
    // Check, if DACs are greater than zero
    for(int i=0; i<NOF_CHANNELS; i++)
    {
        if(conf.dac_offset[i] == 0) printf("%s: Config warning: DAC %d is 0.\n",MODULE_NAME,i);
    }

    return 0;
}

int Sis3302V1410Module::updateModuleInfo()
{
    int ret = 0;

    uint32_t modId;
    ret = getModuleId(&modId);

    conf.module_id = (modId >> SIS3302_V1410_OFF_FIRMWARE_MODID);
    conf.firmware_major_rev = ((modId & SIS3302_V1410_MSK_FIRMWARE_REV_MAJ) >> SIS3302_V1410_OFF_FIRMWARE_REV_MAJ);
    conf.firmware_minor_rev = ((modId & SIS3302_V1410_MSK_FIRMWARE_REV_MIN) >> SIS3302_V1410_OFF_FIRMWARE_REV_MIN);
    conf.module_id = (modId >> SIS3302_V1410_OFF_FIRMWARE_MODID);

    return ret;
}

int Sis3302V1410Module::getModuleId(uint32_t* _modId)
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_V1410_MODID;
    data = 0;
    ret = getInterface()->readA32D32(addr,&data);
    if(ret != 0) { printf("Error %d at VME READ ID\n",ret); (*_modId) = 0;}
    else (*_modId) = data;
    return ret;
}

int Sis3302V1410Module::getMcaTrgStartCounter(uint8_t ch, uint32_t* _evCnt)
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_V1410_MCA_TRG_START_COUNTER(ch);
    data = 0;
    ret = getInterface()->readA32D32(addr,&data);
    if(ret != 0) {
        printf("Error %d at VME READ SIS3302_V1410_MCA_TRG_START_COUNTER(%d)\n",ret,ch);
        (*_evCnt) = -1;
    }
    else (*_evCnt) = data;
    return ret;
}

// This function returns the next sampling address from each adc
// adc: Range 0..7 denotes the adc channel
// _addr: Placeholder to store the value
int Sis3302V1410Module::getNextSampleAddr(int adc, uint32_t* _addr)
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_V1410_NEXT_SAMPLE_ADDR(adc);
    data = 0;
    ret = getInterface ()->readA32D32(addr,&data);
    //printf("sis3302: ch %d, nextsampleaddr: 0x%x from addr 0x%x\n",adc,data,addr);
    if(ret != 0) {
        printf("Error %d at VME READ SIS3302_V1410_NEXT_SAMPLE_ADDR(%d)",ret,adc);
        (*_addr) = 0;
    }
    else (*_addr) = data;
    return ret;
}

int Sis3302V1410Module::reset()
{
    printf("sis3302::reset\n");
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_V1410_RESET; data = 0;
    ret = getInterface()->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_V1410_RESET",ret);
    return ret;
}

int Sis3302V1410Module::resetSampleLogic()
{
    printf("sis3302::reset\n");
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_V1410_SAMPLE_LOGIC_RESET; data = 0;
    ret = getInterface()->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_V1410_SAMPLE_LOGIC_RESET",ret);
    return ret;
}

int Sis3302V1410Module::arm(uint8_t bank)
{
    int ret = 0x0;
    if(bank == 1) {
        addr = conf.base_addr + SIS3302_V1410_DISARM_AND_ARM_BANK1;
    } else {
        addr = conf.base_addr + SIS3302_V1410_DISARM_AND_ARM_BANK2;
    }
    data = 0;
    ret = getInterface()->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_V1410_DISARM_AND_ARM_BANK %d",ret,bank);
    return ret;
}

int Sis3302V1410Module::disarm()
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_V1410_DISARM; data = 0;
    ret = getInterface()->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_V1410_DISARM",ret);
    return ret;
}

int Sis3302V1410Module::trigger()
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_V1410_TRIGGER; data = 0;
    ret = getInterface()->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_V1410_TRIGGER",ret);
    return ret;
}

int Sis3302V1410Module::reset_DDR2_logic()
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_V1410_RESET_DDR2; data = 0;
    ret = getInterface()->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_V1410_RESET_DDR2",ret);
    return ret;
}

int Sis3302V1410Module::timestamp_clear()
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_V1410_TIMESTAMP_CLEAR; data = 0;
    ret = getInterface()->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_V1410_TIMESTAMP_CLEAR",ret);
    return ret;
}

bool Sis3302V1410Module::isArmedNotBusy(uint8_t bank)
{
    bool ret = false;
    int test = 0;
    uint32_t msk = SIS3302_V1410_MSK_ACQ_CTRL_BUSY;
    uint32_t msk2 = 0;
    if(bank == 1) {
        msk2 = SIS3302_V1410_MSK_ACQ_CTRL_ARMED_BNK_1;
    } else {
        msk2 = SIS3302_V1410_MSK_ACQ_CTRL_ARMED_BNK_2;
    }
    msk |= msk2;
    addr = conf.base_addr + SIS3302_V1410_ACQUISITION_CONTROL;
    data = 0;
    test = getInterface()->readA32D32(addr,&data);
    if(test != 0) {
        printf("Error %d at VME READ SIS3302_V1410_ACQUISITION_CONTROL\n",ret);
    } else {
        ret = ((data & msk) == msk2);
    }
    return ret;
}

bool Sis3302V1410Module::isArmed(uint8_t bank)
{
    bool ret = false;
    int test = 0;
    uint32_t msk = 0;
    if(bank == 1) {
        msk = SIS3302_V1410_MSK_ACQ_CTRL_ARMED_BNK_1;
    } else {
        msk = SIS3302_V1410_MSK_ACQ_CTRL_ARMED_BNK_2;
    }
    addr = conf.base_addr + SIS3302_V1410_ACQUISITION_CONTROL;
    data = 0;
    test = getInterface()->readA32D32(addr,&data);
    if(test != 0) {
        printf("Error %d at VME READ SIS3302_V1410_ACQUISITION_CONTROL\n",ret);
    } else {
        //INFO("acquisitionControlRegister",data);
        ret = ((data & msk) == msk);
    }
    return ret;
}

bool Sis3302V1410Module::isArmedOrBusy()
{
    bool ret = false;
    int test = 0;
    uint32_t msk = SIS3302_V1410_MSK_ACQ_CTRL_BUSY |
                   SIS3302_V1410_MSK_ACQ_CTRL_ARMED_BNK_1 |
                   SIS3302_V1410_MSK_ACQ_CTRL_ARMED_BNK_2;
    addr = conf.base_addr + SIS3302_V1410_ACQUISITION_CONTROL;
    data = 0;
    test = getInterface()->readA32D32(addr,&data);
    if(test != 0) {
        printf("Error %d at VME READ SIS3302_V1410_ACQUISITION_CONTROL\n",ret);
    } else {
        //INFO("acquisitionControlRegister",data);
        ret = ((data & msk) != 0x0);
    }
    return ret;
}

bool Sis3302V1410Module::isNotArmedNotBusy()
{
    bool ret = false;
    int test = 0;
    uint32_t msk = SIS3302_V1410_MSK_ACQ_CTRL_BUSY |
                   SIS3302_V1410_MSK_ACQ_CTRL_ARMED_BNK_1 |
                   SIS3302_V1410_MSK_ACQ_CTRL_ARMED_BNK_2;
    addr = conf.base_addr + SIS3302_V1410_ACQUISITION_CONTROL;
    data = 0;
    test = getInterface()->readA32D32(addr,&data);
    if(test != 0) {
        printf("Error %d at VME READ SIS3302_V1410_ACQUISITION_CONTROL\n",ret);
    } else {
        ret = ((data & msk) == 0x0);
    }
    return ret;
}

int Sis3302V1410Module::waitForNotBusy()
{
    //printf("sis3302: waitForNotBusy \n");
    bool ret = false;
    AbstractInterface *iface = getInterface ();
    addr = conf.base_addr + SIS3302_V1410_ACQUISITION_CONTROL; data = 0;
    unsigned int cnt = 0;
    do {
        if(iface->readA32D32(addr,&data) != 0) {
            printf("Error %d at VME READ SIS3302_V1410_ACQUISITION_CONTROL\n",ret);
        }
        cnt++;
    } while(((data & SIS3302_V1410_MSK_ACQ_CTRL_BUSY) == SIS3302_V1410_MSK_ACQ_CTRL_BUSY)
          && (cnt < conf.poll_count));
    //printf("Waited for %d poll loops.\n",cnt);
    return (cnt < conf.poll_count);
}

int Sis3302V1410Module::waitForAddrThreshold()
{
    //printf("sis3302: waitForAddrThreshold \n");
    bool ret = false;
    AbstractInterface *iface = getInterface ();
    addr = conf.base_addr + SIS3302_V1410_ACQUISITION_CONTROL; data = 0;
    unsigned int cnt = 0;
    do {
        if(iface->readA32D32(addr,&data) != 0) {
            printf("Error %d at VME READ SIS3302_V1410_ACQUISITION_CONTROL\n",ret);
        }
        cnt++;
    } while(((data & SIS3302_V1410_MSK_ACQ_CTRL_END_ADDR_THR) != SIS3302_V1410_MSK_ACQ_CTRL_END_ADDR_THR)
          && (cnt < conf.poll_count));
    //printf("Waited for %d poll loops.\n",cnt);
    return (cnt < conf.poll_count);
}

int Sis3302V1410Module::singleShot()
{
    int ret = 0;

    ret = reset();
    if(ret != 0) {
        ERROR("singleShot: reset failed.",ret);
    }
    //ret = timestamp_clear();
    if(ret != 0) {
        ERROR("singleShot: timestamp clear failed.",ret);
    }
    ret = configure();
    if(ret != 0) {
        ERROR("singleShot: configure failed.",ret);
    }

    int nof_events = 1;
    if(conf.acMode == Sis3302V1410config::singleEvent) {
        ret = acquisitionStartSingle();
        nof_events = 1;
    } else if(conf.acMode == Sis3302V1410config::multiEvent) {
        ret = acquisitionStartMulti();
        nof_events = conf.nof_events;
    }
    if(ret != 0) {
        ERROR("singleShot: acquisition failed.",ret);
    }

    for(int n = 0; n < nof_events; ++n) {
        // Store the data in local values
        for(int ch = 0; ch < NOF_CHANNELS; ++ch) {
            if(conf.enable_ch[ch]) {
                uint32_t raw_sample_length_samples = conf.raw_sample_length[ch/2];
                uint32_t energy_sample_length_words = conf.energy_sample_length[ch/2];
                uint32_t event_length_words = 6 + conf.raw_sample_length[ch/2]/2
                                                + conf.energy_sample_length[ch/2];
                uint32_t bufIdx = n * event_length_words;

                //INFO("raw_sample_length_words",raw_sample_length_samples);
                //INFO("energy_sample_length_words",energy_sample_length_words);

                currentTimestamp[ch] =
                        (((uint64_t)(readBuffer[ch][0] & SIS3302_V1410_MSK_EVENT_BUF_TS_HI)) << 16) + (readBuffer[ch][1]);
                currentHeader[ch] = (readBuffer[ch][0] & SIS3302_V1410_MSK_EVENT_BUF_HEADER);
                currentRawLengthFromHeader[ch] = ((currentHeader[ch]-ch)/2);
                bufIdx = 2;
                for(uint32_t i = 0; i < raw_sample_length_samples;) {
                    currentRawBuffer[ch][i++] = (readBuffer[ch][bufIdx] & SIS3302_V1410_MSK_EVENT_BUF_RAW_LOW);
                    currentRawBuffer[ch][i++] = ((readBuffer[ch][bufIdx] & SIS3302_V1410_MSK_EVENT_BUF_RAW_HIGH) >> SIS3302_V1410_OFF_EVENT_BUF_RAW_HIGH);
                    //INFO_i("raw data",i-2, currentRawBuffer[ch][i-2]);
                    //INFO_i("raw data",i-1, currentRawBuffer[ch][i-1]);
                    bufIdx++;
                }
                for(uint32_t i = 0; i < energy_sample_length_words; ++i) {
                    currentEnergyBuffer[ch][i] = (int32_t)(readBuffer[ch][bufIdx++]);
                    //INFO_i("energy data",i, currentEnergyBuffer[ch][i]);
                    //printf("<%d,%d> Energy data before: 0x%08x = %u, after 0x%08x = %d\n",
                    //       ch,i,readBuffer[ch][bufIdx-1],readBuffer[ch][bufIdx-1],
                    //       currentEnergyBuffer[ch][i],currentEnergyBuffer[ch][i]);
                }
                currentEnergyMaxValue[ch] =  (int32_t)(readBuffer[ch][bufIdx++]);
                currentEnergyFirstValue[ch] =  (int32_t)(readBuffer[ch][bufIdx++]);
                uint32_t flags = (readBuffer[ch][bufIdx++]);
                currentTriggerCounter[ch] = ((flags & SIS3302_V1410_MSK_EVENT_BUF_FAST_TRG_CNT) >> SIS3302_V1410_OFF_EVENT_BUF_FAST_TRG_CNT);
                currentPileupFlag[ch] = ((flags & SIS3302_V1410_MSK_EVENT_BUF_PILEUP_FLAG) >> SIS3302_V1410_OFF_EVENT_BUF_PILEUP_FLAG);
                currentRetriggerFlag[ch] = ((flags & SIS3302_V1410_MSK_EVENT_BUF_RETRIG_FLAG) >> SIS3302_V1410_OFF_EVENT_BUF_RETRIG_FLAG);

                /*if(!currentPileupFlag[ch]) {
                    DUMP("currentEnergyBuffer[ch]",currentEnergyBuffer[ch],energy_sample_length_words);
                }*/
            }
        }

        // Print event dump
        /*
        for(int ch = 0; ch < NOF_CHANNELS; ++ch) {
            if(conf.enable_ch[ch]) {
                printf("\n\n<%s><%d,%d> Start Event Dump #################\n",MODULE_NAME,n,ch);
                printf("<%s><%d,%d> Timestamp:\t %llu \n",MODULE_NAME,n,ch,(long long unsigned int)(currentTimestamp[ch]));
                printf("<%s><%d,%d> Header:\t %d \n",MODULE_NAME,n,ch,currentHeader[ch]);
                printf("<%s><%d,%d> Raw length:\t %d \n",MODULE_NAME,n,ch,currentRawLengthFromHeader[ch]);
                printf("<%s><%d,%d> Energy min:\t %d \n",MODULE_NAME,n,ch,currentEnergyFirstValue[ch]);
                printf("<%s><%d,%d> Energy max:\t %d \n",MODULE_NAME,n,ch,currentEnergyMaxValue[ch]);
                if(currentTriggerCounter[ch]) {
                    printf("<%s><%d,%d> Fast trigger counter: \t %d \n",MODULE_NAME,n,ch,currentTriggerCounter[ch]);
                }
                if(currentPileupFlag[ch]) {
                    printf("<%s><%d,%d> Pileup Flag detected.\n",MODULE_NAME,n,ch);
                }
                if(currentRetriggerFlag[ch]) {
                    printf("<%s><%d,%d> Retrigger Flag detected.\n",MODULE_NAME,n,ch);
                }
            }
        }*/
    }

    // Flush output buffer
    fflush(stdout);

    return ret;
}

int Sis3302V1410Module::acquire(Event *ev)
{
    int ret = 0;

    //printf("sis3302: Acquiring event %p\n", ev);

    if(conf.acMode == Sis3302V1410config::singleEvent) {
        ret = acquisitionStartSingle(); }
    if(conf.acMode == Sis3302V1410config::multiEvent) {
        ret = acquisitionStartMulti(); }
    if(ret == 0) writeToBuffer(ev);
    else printf("sis3302: Failed acquiring event %p, ret = 0x%x\n", ev,ret);

    return ret;
}

int Sis3302V1410Module::acquisitionStartSingle()
{
    int ret = 0;
    int waitCounter = 0;

    //INFO("Arming bank 1");
    ret = this->arm(1);
    //INFO("Waiting for Addr Threshold");
    ret = this->waitForAddrThreshold();
    if(ret == false) ERROR("timeout while waiting for address threshold flag\n",ret);

    //INFO("Disarming bank 1");
    ret = this->disarm();

    // Wait until sampling complete
    while(isArmedOrBusy()) {
        ++waitCounter;
    }
    //INFO("waitCounter",waitCounter);

    //INFO("Reading channel data");
    for(int i=0; i<NOF_CHANNELS; ++i) {
        readLength[i] = 0;
        currentTriggerCounter[i] = 0;

        if(conf.enable_ch[i]) {
            uint32_t expectedNextSamplingAddr_words = 2
                    + conf.raw_sample_length[i/2] / 2
                    + conf.energy_sample_length[i/2]
                    + 4;

            //INFO_i("Start reading on channel.",i);
            uint32_t nofSamplesRead = 0;
            this->getNextSampleAddr(i,&nofSamplesRead);
            //INFO_i("nofSamplesRead",i,nofSamplesRead);
            endSampleAddr_words[i] = nofSamplesRead/2;

            if(expectedNextSamplingAddr_words != endSampleAddr_words[i]) {
                ERROR_i("Expected next sample addr does not match",i,
                        expectedNextSamplingAddr_words,endSampleAddr_words[i]);
            }

            uint32_t reqNofWords = endSampleAddr_words[i];
            //uint32_t reqNofWords = expectedNextSamplingAddr_words;
            //INFO_i("reqNofWords",i,reqNofWords);
            this->readAdcChannelSinglePage(i,reqNofWords);
            //INFO_i("readLength[i]",i,readLength[i]);

            // Check event trailer
            if(readLength[i] > 1 && readBuffer[i][readLength[i]-1] != SIS3302_V1410_MSK_EVENT_BUF_TRAILER) {
                ERROR_i("Event trailer does not match",i,readBuffer[i][readLength[i]-1]);
                ERROR_i("reqNofWords, readLength[ch]",i,reqNofWords,readLength[i]);
                //DUMP("readBuffer[i]",readBuffer[i],readLength[i]);
            }
            // Store channel information in highest 3 bits of readLength[i];
            readLength[i] |= (i << 29);
        }
    }

    return ret;
}

int Sis3302V1410Module::acquisitionStartMulti()
{
    int ret = 0;
    int waitCounter = 0;

    //INFO("Arming bank 1");
    ret = this->arm(1);
    //INFO("Waiting for Addr Threshold");
    ret = this->waitForAddrThreshold();
    if(ret == false) ERROR("timeout while waiting for address threshold flag\n",ret);

    //INFO("Disarming bank 1");
    ret = this->disarm();

    // Wait until sampling complete
    while(isArmedOrBusy()) {
        ++waitCounter;
    }
    //INFO("waitCounter",waitCounter);

    //INFO("Reading channel data");
    for(int i=0; i<NOF_CHANNELS; ++i) {
        readLength[i] = 0;
        currentTriggerCounter[i] = 0;

        if(conf.enable_ch[i]) {
            uint32_t expectedNextSamplingAddr_words =
                    (6 + conf.raw_sample_length[i/2]/2
                    + conf.energy_sample_length[i/2]) * conf.nof_events;

            //INFO_i("Start reading on channel.",i);
            uint32_t nofSamplesRead = 0;
            this->getNextSampleAddr(i,&nofSamplesRead);
            //INFO_i("nofSamplesRead",i,nofSamplesRead);
            endSampleAddr_words[i] = nofSamplesRead/2;

            if(expectedNextSamplingAddr_words > endSampleAddr_words[i]) {
                ERROR_i("Expected next sample addr does not match",i,
                        expectedNextSamplingAddr_words,endSampleAddr_words[i]);
            }

            uint32_t reqNofWords = endSampleAddr_words[i];
            //uint32_t reqNofWords = expectedNextSamplingAddr_words;
            //INFO_i("reqNofWords",i,reqNofWords);
            this->readAdcChannelSinglePage(i,reqNofWords);
            //INFO_i("readLength[i]",i,readLength[i]);

            // Check event trailer
            if(readLength[i] > 1 && readBuffer[i][readLength[i]-1] != SIS3302_V1410_MSK_EVENT_BUF_TRAILER) {
                ERROR_i("Event trailer does not match",i,readBuffer[i][readLength[i]-1]);
                ERROR_i("reqNofWords, readLength[ch]",i,reqNofWords,readLength[i]);
                DUMP("readBuffer[i]",readBuffer[i],readLength[i]);
            }
            // Store channel information in highest 3 bits of readLength[i];
            readLength[i] |= (i << 29);
        }
    }

    return ret;
}

int Sis3302V1410Module::writeToBuffer(Event *ev)
{
    if(conf.acMode == Sis3302V1410config::multiEvent) {
        dmx.setMultiEvent(true);
        dmx.setNofEvents(conf.nof_events);
    } else {
        dmx.setMultiEvent(false);
        dmx.setNofEvents(1);
    }
    for(unsigned int i = 0; i < NOF_CHANNELS; i++)
    {
        //printf("sis3302: ch %d: Trying to write to buffer (ev = 0x%x)\n",i,ev);
        if(conf.enable_ch[i] == false) continue;
        dmx.process (ev, readBuffer[i], readLength[i]);
        //printf("sis3302: ch %d: Success!\n",i);
    }
    return 0;
}

bool Sis3302V1410Module::dataReady()
{
    bool dready = isNotArmedNotBusy();
    return dready;
}

/*! This function does the readout of the ADC memory
 *  up to the length of ONE page.
 *  ch: The channel to be read out. From 0 to NOF_CHANNELS
 *  _reqNofWords: The number of words to be read out from the channel.
 *
 *  Returns: 0:  OK
 *           !0: else
 *
 *  Read out mode can be controlled using conf.vmeMode
 *
 *  Side effects: Data from the channel i is stored in readBuffer[ch]
 *                starting at index 0.
 *                Number of words read is stored in readLength[ch]
 */
int Sis3302V1410Module::readAdcChannelSinglePage(int ch, uint32_t _reqNofWords)
{
    int ret = 0;

    AbstractInterface *iface = getInterface();

    if(_reqNofWords > SIS3302_V1410_MEM_PAGE_LENGTH_WORDS) {
        ERROR("Too many words required for single page!",_reqNofWords);
    }

    uint32_t addr = conf.base_addr + SIS3302_V1410_ADC_MEM(ch);

    switch(conf.vmeMode) {

    case Sis3302V1410config::vmeSingle: {
        //INFO("vmeMode = vmeSingle");
        uint32_t words  = _reqNofWords;
        uint32_t bufIdx = 0;
        //INFO("ch",ch);
        //INFO("words",words);
        while(words--) {
            //INFO_i("address",words,addr);
            ret = iface->readA32D32(addr,&readBuffer[ch][bufIdx++]);
            if(ret != 0) {
                ERROR_i("readAdcChannelSinglePage with vmeSingle"
                        "read error in ch = a, ret = b",words,ch,ret);
            } /*else {
                INFO_i("data",words,readBuffer[ch][bufIdx-1]);
            }*/
            addr+=4;
        }
        readLength[ch] = _reqNofWords;
        break;}

    case Sis3302V1410config::vme2E: {
        uint32_t words = 0;
        ret = iface->readA322E(addr,&readBuffer[ch][0],_reqNofWords,&words);
        if(ret != 0) {
            ERROR("readAdcChannelSinglePage with vme2E"
                   "read error in ch = a, ret = b",ch,ret);
        }
        if(_reqNofWords != words) {
            ERROR("readAdcChannelSinglePage with vme2E"
                  "mismatch of a:_reqNofWords and b:words",_reqNofWords,words);
        }
        readLength[ch] = words;
        break;}

    case Sis3302V1410config::vmeBLT32: {
        uint32_t words = 0;
        ret = iface->readA32BLT32(addr,&readBuffer[ch][0],_reqNofWords,&words);
        if(ret != 0) {
            ERROR("readAdcChannelSinglePage with vmeBLT32"
                   "read error in ch = a, ret = b",ch,ret);
        }
        if(_reqNofWords != words) {
            ERROR("readAdcChannelSinglePage with vmeBLT32"
                  "mismatch of a:_reqNofWords and b:words",_reqNofWords,words);
        }
        readLength[ch] = words;
        break;}

    default:
        ret = false;
        break;
    }

    return ret;
}

int Sis3302V1410Module::readAdcChannel(int ch, uint32_t _reqNofWords)
{
    int ret = 0 ;
#if 0
    //printf("sis3302 Starting ADC ch %d read of %d lwords\n",ch,_reqNofWords);

    AbstractInterface *iface = getInterface ();

    uint32_t nextEventSampleStartAddr = (conf.event_sample_start_addr & 0x01fffffc)  ; // max 32 MSample == 64 MByte
    uint32_t restEventSampleLength = ((_reqNofWords*2) & 0x01fffffc); // max 32 MSample == 64 MByte
    if (restEventSampleLength  >= 0x2000000) {restEventSampleLength =  0x2000000 ;}



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
#endif
    return ret;
}

void Sis3302V1410Module::INFO_i(const char *msg, int i) {
    printf("<%s> INFO in loop at (%d): %s\n",MODULE_NAME,i,msg);
}
void Sis3302V1410Module::INFO_i(const char *msg, int i, uint32_t v) {
    printf("<%s> INFO in loop at (%d): %s (0x%08x)\n",MODULE_NAME,i,msg,v);
}
void Sis3302V1410Module::INFO_i(const char *msg, int i, uint32_t a, uint32_t b) {
    printf("<%s> INFO in loop at (%d): %s (a = 0x%08x, b = 0x%08x)\n",MODULE_NAME,i,msg,a,b);
}
void Sis3302V1410Module::INFO(const char *msg) {
    printf("<%s> INFO: %s\n",MODULE_NAME,msg);
}
void Sis3302V1410Module::INFO(const char *msg, uint32_t v) {
    printf("<%s> INFO: %s (0x%08x)\n",MODULE_NAME,msg,v);
}
void Sis3302V1410Module::INFO(const char *msg, uint32_t a, uint32_t b) {
    printf("<%s> INFO: %s (a = 0x%08x, b= 0x%08x)\n",MODULE_NAME,msg,a,b);
}

void Sis3302V1410Module::ERROR_i(const char *e, int i, uint32_t v) {
    printf("<%s> ERROR in loop at (%d): %s (0x%08x)\n",MODULE_NAME,i,e,v);
}
void Sis3302V1410Module::ERROR_i(const char *e, int i, uint32_t a, uint32_t b) {
    printf("<%s> ERROR in loop at (%d): %s (a = 0x%08x, b = 0x%08x)\n",MODULE_NAME,i,e,a,b);
}
void Sis3302V1410Module::ERROR(const char *e, uint32_t v) {
    printf("<%s> ERROR: %s (0x%08x)\n",MODULE_NAME,e,v);
}
void Sis3302V1410Module::ERROR(const char *e, uint32_t a, uint32_t b) {
    printf("<%s> ERROR: %s (a = 0x%08x, b= 0x%08x)\n",MODULE_NAME,e,a,b);
}

void Sis3302V1410Module::DUMP(const char* name, uint32_t* buf, uint32_t len) {
    uint8_t cnt = 1;
    printf("<%s> Dumping %s\n",MODULE_NAME,name);
    printf("0x%04x:  ",0);
    for(uint32_t i = 0; i < len; ++i) {
        printf("\t 0x%08x",buf[i]);
        if(cnt == 4) {
            cnt = 0;
            printf("\n");
            printf("0x%04x:  ",i);
        }
        ++cnt;
    }
    printf("\n");
    fflush(stdout);
}
void Sis3302V1410Module::DUMP(const char* name, int32_t* buf, uint32_t len) {
    uint8_t cnt = 1;
    printf("<%s> Dumping %s\n",MODULE_NAME,name);
    printf("0x%04x:  ",0);
    for(uint32_t i = 0; i < len; ++i) {
        printf("\t 0x%08x",buf[i]);
        if(cnt == 4) {
            cnt = 0;
            printf("\n");
            printf("0x%04x:  ",i);
        }
        ++cnt;
    }
    printf("\n");
    fflush(stdout);
}

// STRUCK legacy methods

/* --------------------------------------------------------------------------
   SIS3302 Offset
   offset Value_array		DAC offset value (16 bit)
-------------------------------------------------------------------------- */
int Sis3302V1410Module::write_dac_offset(uint32_t *offset_value_array)
{
        uint32_t i, error;
        uint32_t data, addr;
        uint32_t max_timeout, timeout_cnt;
        int ret = 0;
        AbstractInterface *iface = getInterface ();

        for (i=0;i<NOF_CHANNELS;i++) {

                data = offset_value_array[i] ;
                addr = conf.base_addr + SIS3302_V1410_DAC_DATA ;
                if ((error = iface->writeA32D32(addr,data )) != 0) {
                        ERROR_i("write_dac_offset: dac data write",i,error);
                        ret++;
                }

                // write to DAC Register
                data =  1 + (i << 4);
                addr = conf.base_addr + SIS3302_V1410_DAC_CTRL_STATUS ;
                if ((error = iface->writeA32D32(addr,data )) != 0) {
                         ERROR_i("write_dac_offset: dac ctrl write",i,error);
                         ret++;
                }

                max_timeout = 5000 ;
                timeout_cnt = 0 ;
                addr = conf.base_addr + SIS3302_V1410_DAC_CTRL_STATUS ;
                do {
                        if ((error = iface->readA32D32(addr,&data )) != 0) {
                                ERROR_i("write_dac_offset: dac ctrl read",i,error);
                                ret++;
                        }
                        timeout_cnt++;
                } while ( ((data & SIS3302_V1410_MSK_DAC_CTRL_BUSY) == SIS3302_V1410_MSK_DAC_CTRL_BUSY)
                          && (timeout_cnt <  max_timeout) )    ;

                if (timeout_cnt >=  max_timeout) ret++;

                // Load DACs
                data =  2 + (i << 4);
                addr = conf.base_addr + SIS3302_V1410_DAC_CTRL_STATUS  ;
                if ((error = iface->writeA32D32(addr,data )) != 0) {
                        ERROR_i("write_dac_offset: dac ctrl write",i,error);
                        ret++;
                }
                timeout_cnt = 0 ;
                addr = conf.base_addr + SIS3302_V1410_DAC_CTRL_STATUS  ;
                do {
                        if ((error = iface->readA32D32(addr,&data )) != 0) {
                                ERROR_i("write_dac_offset: dac ctrl read",i,error);
                                ret++;
                        }
                        timeout_cnt++;
                } while ( ((data & SIS3302_V1410_MSK_DAC_CTRL_BUSY) == SIS3302_V1410_MSK_DAC_CTRL_BUSY)
                         && (timeout_cnt <  max_timeout) )    ;

                if (timeout_cnt >=  max_timeout) ret++;
        }

        return ret;
}




// Configuration handling

typedef ConfMap::confmap_t<Sis3302V1410config> confmap_t;
static const confmap_t confmap [] = {
    confmap_t ("base_addr", &Sis3302V1410config::base_addr),
    confmap_t ("poll_count", &Sis3302V1410config::poll_count),
    confmap_t ("irq_level", &Sis3302V1410config::irq_level),
    confmap_t ("irq_vector", &Sis3302V1410config::irq_vector),
    confmap_t ("nof_events", &Sis3302V1410config::nof_events),
    confmap_t ("module_id", &Sis3302V1410config::module_id),
    confmap_t ("firmware_major_rev", &Sis3302V1410config::firmware_major_rev),
    confmap_t ("firmware_minor_rev", &Sis3302V1410config::firmware_minor_rev),
    confmap_t ("enable_irq", &Sis3302V1410config::enable_irq),
    confmap_t ("update_irq", &Sis3302V1410config::update_irq),
    confmap_t ("enable_user_led", &Sis3302V1410config::enable_user_led),
    confmap_t ("enable_vipa", &Sis3302V1410config::enable_vipa),
    confmap_t ("enable_geo_addressing", &Sis3302V1410config::enable_geo_addressing),
    confmap_t ("enable_reduced_addressing", &Sis3302V1410config::enable_reduced_addressing),
    confmap_t ("enable_a32_addressing", &Sis3302V1410config::enable_a32_addressing),
    confmap_t ("send_int_trg_to_ext_as_or", &Sis3302V1410config::send_int_trg_to_ext_as_or),
    confmap_t ("acMode", (uint32_t Sis3302V1410config::*) &Sis3302V1410config::acMode),
    confmap_t ("vmeMode", (uint32_t Sis3302V1410config::*) &Sis3302V1410config::vmeMode),
    confmap_t ("clockSource", (uint32_t Sis3302V1410config::*) &Sis3302V1410config::clockSource),
    confmap_t ("irqSource", (uint32_t Sis3302V1410config::*) &Sis3302V1410config::irqSource),
    confmap_t ("irqMode", (uint32_t Sis3302V1410config::*) &Sis3302V1410config::irqMode),
    confmap_t ("lemo_in_mode", (uint32_t Sis3302V1410config::*) &Sis3302V1410config::lemo_in_mode),
    confmap_t ("lemo_out_mode", (uint32_t Sis3302V1410config::*) &Sis3302V1410config::lemo_out_mode),
};

void Sis3302V1410Module::applySettings (QSettings *s) {
    if(s==0) return;

    std::cout << "Applying settings for " << getName ().toStdString () << "... ";
    s->beginGroup (getName ());
    ConfMap::apply (s, &conf, confmap);

    QString key;
    for (int i = 0; i < 3; ++i) {
        key = QString ("enable_lemo_in%1").arg (i);
        if (s->contains (key)) conf.enable_lemo_in[i] = s->value (key).toBool ();
    }

    for (int i = 0; i < NOF_ADC_GROUPS; ++i) {
        key = QString ("header_id%1").arg (i);
        if (s->contains (key)) conf.header_id [i] = s->value (key).toUInt ();
        key = QString ("raw_sample_length%1").arg (i);
        if (s->contains (key)) conf.raw_sample_length [i] = s->value (key).toUInt ();
        key = QString ("raw_data_sample_start_idx%1").arg (i);
        if (s->contains (key)) conf.raw_data_sample_start_idx [i] = s->value (key).toUInt ();
        key = QString ("end_addr_thr_in_samples%1").arg (i);
        if (s->contains (key)) conf.end_addr_thr_in_samples [i] = s->value (key).toUInt ();
        key = QString ("trigger_gate_length%1").arg (i);
        if (s->contains (key)) conf.trigger_gate_length [i] = s->value (key).toUInt ();
        key = QString ("trigger_pretrigger_delay%1").arg (i);
        if (s->contains (key)) conf.trigger_pretrigger_delay [i] = s->value (key).toUInt ();
        key = QString ("energy_peak_length%1").arg (i);
        if (s->contains (key)) conf.energy_peak_length [i] = s->value (key).toUInt ();
        key = QString ("energy_sumg_length%1").arg (i);
        if (s->contains (key)) conf.energy_sumg_length [i] = s->value (key).toUInt ();
        key = QString ("energy_gate_length%1").arg (i);
        if (s->contains (key)) conf.energy_gate_length [i] = s->value (key).toUInt ();
        key = QString ("energy_sample_length%1").arg (i);
        if (s->contains (key)) conf.energy_sample_length [i] = s->value (key).toUInt ();
        key = QString ("energy_sample_start_idx_1_%1").arg (i);
        if (s->contains (key)) conf.energy_sample_start_idx[i][0] = s->value (key).toUInt ();
        key = QString ("energy_sample_start_idx_2_%1").arg (i);
        if (s->contains (key)) conf.energy_sample_start_idx[i][1] = s->value (key).toUInt ();
        key = QString ("energy_sample_start_idx_3_%1").arg (i);
        if (s->contains (key)) conf.energy_sample_start_idx[i][2] = s->value (key).toUInt ();
        key = QString ("energy_tau%1").arg (i);
        if (s->contains (key)) conf.energy_tau [i] = s->value (key).toUInt ();
        key = QString ("enable_energy_extra_filter%1").arg (i);
        if (s->contains (key)) conf.enable_energy_extra_filter [i] = s->value (key).toBool ();
        key = QString ("energy_decim_mode%1").arg (i);
        if (s->contains (key)) conf.energy_decim_mode[i] = (Sis3302V1410config::EnDecimMode)s->value (key).toUInt ();
        key = QString ("energy_test_mode%1").arg (i);
        if (s->contains (key)) conf.energy_test_mode[i] = (Sis3302V1410config::EnTestMode)s->value (key).toUInt ();
    }

    for (int i = 0; i < NOF_CHANNELS; ++i) {
        key = QString ("trigger_pulse_length%1").arg (i);
        if (s->contains (key)) conf.trigger_pulse_length [i] = s->value (key).toUInt ();
        key = QString ("trigger_sumg_length%1").arg (i);
        if (s->contains (key)) conf.trigger_sumg_length [i] = s->value (key).toUInt ();
        key = QString ("trigger_peak_length%1").arg (i);
        if (s->contains (key)) conf.trigger_peak_length [i] = s->value (key).toUInt ();
        key = QString ("trigger_threshold%1").arg (i);
        if (s->contains (key)) conf.trigger_threshold [i] = s->value (key).toInt ();
        key = QString ("trigger_int_gate_length%1").arg (i);
        if (s->contains (key)) conf.trigger_int_gate_length [i] = s->value (key).toUInt ();
        key = QString ("trigger_int_trg_delay%1").arg (i);
        if (s->contains (key)) conf.trigger_int_trg_delay [i] = s->value (key).toUInt ();
        key = QString ("dac_offset%1").arg (i);
        if (s->contains (key)) conf.dac_offset [i] = s->value (key).toUInt ();
        key = QString ("enable_input_invert%1").arg (i);
        if (s->contains (key)) conf.enable_input_invert [i] = s->value (key).toBool();
        key = QString ("enable_ch%1").arg (i);
        if (s->contains (key)) conf.enable_ch [i] = s->value (key).toBool ();
        key = QString ("enable_next_adc_gate%1").arg (i);
        if (s->contains (key)) conf.enable_next_adc_gate [i] = s->value (key).toBool ();
        key = QString ("enable_prev_adc_gate%1").arg (i);
        if (s->contains (key)) conf.enable_prev_adc_gate [i] = s->value (key).toBool ();
        key = QString ("enable_next_adc_trg%1").arg (i);
        if (s->contains (key)) conf.enable_next_adc_trg [i] = s->value (key).toBool ();
        key = QString ("enable_prev_adc_trg%1").arg (i);
        if (s->contains (key)) conf.enable_prev_adc_trg [i] = s->value (key).toBool ();
        key = QString ("enable_ext_gate%1").arg (i);
        if (s->contains (key)) conf.enable_ext_gate [i] = s->value (key).toBool ();
        key = QString ("enable_ext_trg%1").arg (i);
        if (s->contains (key)) conf.enable_ext_trg [i] = s->value (key).toBool ();
        key = QString ("enable_int_gate%1").arg (i);
        if (s->contains (key)) conf.enable_int_gate [i] = s->value (key).toBool ();
        key = QString ("enable_int_trg%1").arg (i);
        if (s->contains (key)) conf.enable_int_trg [i] = s->value (key).toBool ();
        key = QString ("disable_trg_out%1").arg (i);
        if (s->contains (key)) conf.disable_trg_out [i] = s->value (key).toBool ();
        key = QString ("trigger_decim_mode%1").arg (i);
        if (s->contains (key)) conf.trigger_decim_mode[i] = (Sis3302V1410config::TrgDecimMode)s->value (key).toUInt ();

    }

    updateEndAddrThresholds();

    s->endGroup ();

    getUI ()->applySettings ();
    std::cout << "done" << std::endl;
}

void Sis3302V1410Module::saveSettings (QSettings *s) {
    if(s==0) return;

    std::cout << "Saving settings for " << getName ().toStdString () << "... ";
    s->beginGroup (getName ());
    ConfMap::save (s, &conf, confmap);

    QString key;
    for (int i = 0; i < 3; ++i) {
        key = QString ("enable_lemo_in%1").arg (i);
        s->setValue (key, static_cast<bool> (conf.enable_lemo_in[i]));
    }

    for (int i = 0; i < NOF_ADC_GROUPS; ++i) {
        key = QString ("header_id%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.header_id[i]));
        key = QString ("raw_sample_length%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.raw_sample_length[i]));
        key = QString ("raw_data_sample_start_idx%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.raw_data_sample_start_idx[i]));
        key = QString ("end_addr_thr_in_samples%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.end_addr_thr_in_samples[i]));
        key = QString ("trigger_gate_length%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.trigger_gate_length[i]));
        key = QString ("trigger_pretrigger_delay%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.trigger_pretrigger_delay[i]));
        key = QString ("header_id%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.header_id[i]));
        key = QString ("energy_peak_length%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.energy_peak_length[i]));
        key = QString ("energy_sumg_length%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.energy_sumg_length[i]));
        key = QString ("energy_gate_length%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.energy_gate_length[i]));
        key = QString ("energy_sample_length%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.energy_sample_length[i]));
        key = QString ("energy_sample_start_idx_1_%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.energy_sample_start_idx[i][0]));
        key = QString ("energy_sample_start_idx_2_%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.energy_sample_start_idx[i][1]));
        key = QString ("energy_sample_start_idx_3_%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.energy_sample_start_idx[i][2]));
        key = QString ("energy_tau%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.energy_tau[i]));
        key = QString ("enable_energy_extra_filter%1").arg(i);
        s->setValue (key, static_cast<bool> (conf.enable_energy_extra_filter[i]));
        key = QString ("energy_decim_mode%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.energy_decim_mode[i]));
        key = QString ("energy_test_mode%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.energy_test_mode[i]));

    }

    for (int i = 0; i < NOF_CHANNELS; ++i) {
        key = QString ("trigger_pulse_length%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.trigger_pulse_length[i]));
        key = QString ("trigger_sumg_length%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.trigger_sumg_length[i]));
        key = QString ("trigger_peak_length%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.trigger_peak_length[i]));
        key = QString ("trigger_threshold%1").arg(i);
        s->setValue (key, static_cast<int32_t> (conf.trigger_threshold[i]));
        key = QString ("dac_offset%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.dac_offset[i]));
        key = QString ("enable_ch%1").arg(i);
        s->setValue (key, static_cast<bool> (conf.enable_ch[i]));
        key = QString ("trigger_int_gate_length%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.trigger_int_gate_length[i]));
        key = QString ("trigger_int_trg_delay%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.trigger_int_trg_delay[i]));
        key = QString ("enable_input_invert%1").arg(i);
        s->setValue (key, static_cast<bool> (conf.enable_input_invert[i]));
        key = QString ("enable_next_adc_gate%1").arg(i);
        s->setValue (key, static_cast<bool> (conf.enable_next_adc_gate[i]));
        key = QString ("enable_prev_adc_gate%1").arg(i);
        s->setValue (key, static_cast<bool> (conf.enable_prev_adc_gate[i]));
        key = QString ("enable_next_adc_trg%1").arg(i);
        s->setValue (key, static_cast<bool> (conf.enable_next_adc_trg[i]));
        key = QString ("enable_prev_adc_trg%1").arg(i);
        s->setValue (key, static_cast<bool> (conf.enable_prev_adc_trg[i]));
        key = QString ("enable_ext_gate%1").arg(i);
        s->setValue (key, static_cast<bool> (conf.enable_ext_gate[i]));
        key = QString ("enable_ext_trg%1").arg(i);
        s->setValue (key, static_cast<bool> (conf.enable_ext_trg[i]));
        key = QString ("enable_int_gate%1").arg(i);
        s->setValue (key, static_cast<bool> (conf.enable_int_gate[i]));
        key = QString ("enable_int_trg%1").arg(i);
        s->setValue (key, static_cast<bool> (conf.enable_int_trg[i]));
        key = QString ("disable_trg_out%1").arg(i);
        s->setValue (key, static_cast<bool> (conf.disable_trg_out[i]));
        key = QString ("trigger_decim_mode%1").arg(i);
        s->setValue (key, static_cast<uint32_t> (conf.trigger_decim_mode[i]));
    }

    s->endGroup ();
    std::cout << "done" << std::endl;
}

void Sis3302V1410Module::updateEndAddrThresholds(){
    for (int i = 0; i < NOF_ADC_GROUPS; ++i) {
        if(conf.acMode == Sis3302V1410config::singleEvent) {
            conf.end_addr_thr_in_samples[i] = 6*2
                    + conf.raw_sample_length[i]
                    + conf.energy_sample_length[i]*2;
        } else if (conf.acMode == Sis3302V1410config::multiEvent){
            conf.end_addr_thr_in_samples[i] = (6*2
                    + conf.raw_sample_length[i]
                    + conf.energy_sample_length[i]*2) * conf.nof_events;
        }

        //INFO_i("Setting end_addr_thr_in_samples",i,conf.end_addr_thr_in_samples[i]);
    }
}


/*!
\page sis3302mod SIS 3302 Flash ADC (gamma)
<b>Module name:</b> \c sis3302

\section desc Module Description
The SIS 3302 is a fast multi-channel ADC.
The gamma firmware enables fast energy estimation
for detector signals.

\section Configuration Panel
DO DOCUMENTATION
*/
