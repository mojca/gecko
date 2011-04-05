#include "sis3302module.h"
#include "modulemanager.h"

static ModuleRegistrar registrar ("sis3302", Sis3302Module::create, AbstractModule::TypeDAq);

Sis3302Module::Sis3302Module(int _id, QString _name)
    : BaseDAqModule(_id, _name)
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

void Sis3302Module::createOutputPlugin()
{
    output = new DemuxSis3302Plugin (-1, getName () + " Dmx");
}

void Sis3302Module::setChannels()
{
    // Setup channels
    for (int ch=0; ch<8; ch++)
    {
        getChannels()->push_back(
                new ScopeChannel(this,tr("Sis3302 Raw %1").arg(ch),
                                 ScopeCommon::trace,1000,4));
    }

    getChannels ()->push_back(new ScopeChannel(this,"Sis3302 Meta info",
                                               ScopeCommon::trace,1000,4));
    getChannels ()->push_back(new ScopeChannel(this,"Sis3302 Poll Trigger"
                                               ,ScopeCommon::trigger,4,4));
}

int Sis3302Module::configure()
{
    int ret = 0x0;

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
    data = conf.start_delay & 0xfffff;
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
    data = (conf.event_length-4) & 0xfffffc;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_SAMPLE_LENGTH_ALL_ADC",ret);

    // Setup trigger registers
    for (int i=0; i<8; i++)
    {
        addr = conf.base_addr + SIS3302_TRIGGER_SETUP_ADC1;
        if(i%2 == 1) addr += 8;
        addr += (i/2) * SIS3302_NEXT_ADC_OFFSET;

        data = 0;
        data |= conf.trigger_peak_length[i] & 0x1f;
        data |= (conf.trigger_gap_length[i] & 0x1f) << 8;
        data |= (conf.trigger_pulse_length[i] & 0xff) << 16;

        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_TRIGGER_SETUP_ADC%d",ret,i);

        addr = conf.base_addr + SIS3302_TRIGGER_THRESHOLD_ADC1;
        if(i%2 == 1) addr += 8;
        addr += (i/2) * SIS3302_NEXT_ADC_OFFSET;

        data = 0;
        data |= conf.trigger_threshold[i] & 0xffff;
        if(conf.trgMode[i] == conf.ledFalling)      data |= (1<<24) | (1<<26);
        else if(conf.trgMode[i] == conf.ledRising)  data |= (1<<25) | (1<<26);
        else if(conf.trgMode[i] == conf.firFalling) data |= (1<<24);
        else if(conf.trgMode[i] == conf.firRising)  data |= (1<<25);

        ret = iface->writeA32D32(addr,data);
        if(ret != 0) printf("Error %d at SIS3302_TRIGGER_THRESHOLD_ADC%d",ret,i);
    }

    return ret;
}

int Sis3302Module::getModuleId(unsigned int* _modId)
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_MODID;
    data = 0;
    ret = iface->readA32D32(addr,&data);
    if(ret != 0) { printf("Error %d at VME READ ID\n",ret); (*_modId) = 0;}
    else (*_modId) = data;
    return ret;
}

int Sis3302Module::reset()
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_KEY_RESET; data = 0;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_KEY_RESET",ret);
    return ret;
}

int Sis3302Module::arm()
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_KEY_ARM; data = 0;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_KEY_ARM",ret);
    return ret;
}

int Sis3302Module::disarm()
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_KEY_DISARM; data = 0;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_KEY_DISARM",ret);
    return ret;
}

int Sis3302Module::start_sampling()
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_KEY_START; data = 0;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_KEY_START",ret);
    return ret;
}

int Sis3302Module::stop_sampling()
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_KEY_STOP; data = 0;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_KEY_STOP",ret);
    return ret;
}

int Sis3302Module::reset_DDR2_logic()
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_KEY_RESET_DDR2_LOGIC; data = 0;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_KEY_RESET_DDR2_LOGIC",ret);
    return ret;
}

int Sis3302Module::timestamp_clear()
{
    int ret = 0x0;
    addr = conf.base_addr + SIS3302_KEY_TIMESTAMP_CLEAR; data = 0;
    ret = iface->writeA32D32(addr,data);
    if(ret != 0) printf("Error %d at SIS3302_KEY_TIMESTAMP_CLEAR",ret);
    return ret;
}


// STRUCK legacy methods

/* --------------------------------------------------------------------------
   SIS3302 Offset
   offset Value_array		DAC offset value (16 bit)
-------------------------------------------------------------------------- */
int Sis3302Module::sis3302_write_dac_offset(unsigned int *offset_value_array)
{
        unsigned int i, error;
        unsigned int data, addr;
        unsigned int max_timeout, timeout_cnt;
        int ret = 0;

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
    confmap_t ("enable_external_trg", &Sis3302config::enable_external_trg)
};

void Sis3302Module::applySettings (QSettings *s) {
    if(s==0) return;

    std::cout << "Applying settings for " << getName ().toStdString () << "... ";
    s->beginGroup (getName ());
    ConfMap::apply (s, &conf, confmap);

    QString key;
    key = QString ("acMode");
    if (s->contains (key)) conf.acMode = (Sis3302config::AcMode)s->value (key).toUInt ();
    key = QString ("wrapSize");
    if (s->contains (key)) conf.wrapSize = (Sis3302config::WrapSize)s->value (key).toUInt ();
    key = QString ("avgMode");
    if (s->contains (key)) conf.avgMode = (Sis3302config::AvgMode)s->value (key).toUInt ();
    key = QString ("clockSource");
    if (s->contains (key)) conf.clockSource = (Sis3302config::ClockSource)s->value (key).toUInt ();
    key = QString ("irqSource");
    if (s->contains (key)) conf.irqSource =(Sis3302config::IrqSource) s->value (key).toUInt ();
    key = QString ("irqMode");
    if (s->contains (key)) conf.irqMode = (Sis3302config::IrqMode)s->value (key).toUInt ();

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
    key = QString ("acMode");
    s->setValue (key, static_cast<uint32_t> (conf.acMode));
    key = QString ("wrapSize");
    s->setValue (key, static_cast<uint32_t> (conf.wrapSize));
    key = QString ("avgMode");
    s->setValue (key, static_cast<uint32_t> (conf.avgMode));
    key = QString ("clockSource");
    s->setValue (key, static_cast<uint32_t> (conf.clockSource));
    key = QString ("irqSource");
    s->setValue (key, static_cast<uint32_t> (conf.irqSource));
    key = QString ("irqMode");
    s->setValue (key, static_cast<uint32_t> (conf.irqMode));

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
    }

    s->endGroup ();
    std::cout << "done" << std::endl;
}
