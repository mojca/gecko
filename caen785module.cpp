#include "caen785module.h"
#include "modulemanager.h"
#include "runmanager.h"

static ModuleRegistrar registrar ("caen785", Caen785Module::create, AbstractModule::TypeDAq);

Caen785Module::Caen785Module(int _id, QString _name)
        : BaseDAqModule(_id, _name)
{
    setChannels();
    createOutputPlugin();

    evcntr = 0;
    status1 = 0;
    status2 = 0;

    // Setup
    conf.base_addr = 0x40000000;

    // Create user interface
    setUI (new Caen785UI(this));

    std::cout << "Instantiated Caen785Module" << std::endl;
}

void Caen785Module::setChannels()
{
    // Setup channels
    for (int i= 0; i < 32; ++i)
        getChannels()->push_back(new ScopeChannel(this,QString ("spectrum %1").arg (i),ScopeCommon::eventBuffer,34,1));
    getChannels()->push_back(new ScopeChannel(this,"Poll Trigger",ScopeCommon::trigger,1,1));
}

void Caen785Module::createOutputPlugin()
{
    output = new DemuxCaenADCPlugin (-getId (), getName () + " dmx", AbstractPlugin::Attributes ());
}

int Caen785Module::softReset()
{
    int ret;

    uint32_t addr;
    uint16_t data;

    // Board reset
    addr = conf.base_addr + CAEN785_BIT_SET1;
    data = (1 << 7);
    ret = iface->writeA32D16(addr,data);
    if(ret != 0) printf("Error %d at CAEN785_BIT_SET1\n",ret);

    addr = conf.base_addr + CAEN785_BIT_CLR1;
    data = (1 << 7);
    ret = iface->writeA32D16(addr,data);
    if(ret != 0) printf("Error %d at CAEN785_BIT_CLR1\n",ret);

    return ret;
}

int Caen785Module::dataReset()
{
    int ret;

    uint32_t addr;
    uint16_t data;

    // Data reset
    addr = conf.base_addr + CAEN785_BIT_SET2;
    data = (1 << 2);
    ret = iface->writeA32D16(addr,data);
    if(ret != 0) printf("Error %d at CAEN785_BIT_SET2\n",ret);

    addr = conf.base_addr + CAEN785_BIT_CLR2;
    data = (1 << 2);
    ret = iface->writeA32D16(addr,data);
    if(ret != 0) printf("Error %d at CAEN785_BIT_SET2\n",ret);

    return ret;
}

int Caen785Module::counterReset()
{
    int ret;

    uint32_t addr;
    uint16_t data;

    // Counter reset
    addr = conf.base_addr + CAEN785_EVCNT_RST;
    data = 0x0;
    ret = iface->writeA32D16(addr,data);
    if(ret != 0) printf("Error %d at CAEN785_EVCNT_RST\n",ret);

    return ret;
}

int Caen785Module::configure()
{
    int ret = 0;

    uint32_t addr;
    uint16_t data;

    // Crate select
    addr = conf.base_addr + CAEN785_CRATE_SEL;
    data = 0x0 | conf.cratenumber;
    ret = iface->writeA32D16(addr,data);
    if(ret != 0) printf("Error %d at CAEN785_CRATE_SEL\n",ret);

    // Thresholds
    for(unsigned int i = 0; i < 32; i++)
    {
        addr = conf.base_addr + CAEN785_THRESHOLDS + 2*i;
        data = 0x0 | conf.thresholds[i];
        ret = iface->writeA32D16(addr,data);
        if(ret != 0) printf("Error %d at CAEN785_THRESHOLDS %d\n",ret,i);
    }

    ret = counterReset();

    // Sliding scale
    addr = conf.base_addr + CAEN785_SLIDE_CONST;
    data = 0x0 | conf.slide_constant;
    ret = iface->writeA32D16(addr,data);
    if(ret != 0) printf("Error %d at CAEN785_SLIDE_CONST\n",ret);

    // Board setup
    addr = conf.base_addr + CAEN785_CONTROL1;
    data = 0x0;
    if(conf.block_end){
        data |= (1 << 2); // default OFF
    }
    if(conf.program_reset){
        data |= (1 << 4); // default OFF
    }
    if(conf.berr_enable){
        data |= (1 << 5); // default OFF
    }
    if(conf.align64){
        data |= (1 << 6); // default OFF
    }
    ret = iface->writeA32D16(addr,data);
    if(ret != 0) printf("Error %d at CAEN785_CONTROL1",ret);

    addr = conf.base_addr + CAEN785_BIT_SET2;
    data = 0x0;
    if(conf.memTestModeEnabled) {
        data |= (1 << 0); // default OFF
    }
    if(conf.offline) {
        data |= (1 << 1); // default OFF
    }
    if(conf.overRangeSuppressionEnabled) {
        data |= (1 << 3); // default ON
    }
    if(conf.autoIncrementEnabled) {
        data |= (1 << 11); // default ON
    }
    if(conf.emptyEventWriteEnabled) {
        data |= (1 << 12); // default ON -> empty events are written
    }
    if(conf.alwaysIncrementEventCounter) {
        data |= (1 << 14); // default ON
    }
    ret = iface->writeA32D16(addr,data);
    if(ret != 0) printf("Error %d at CAEN785_BIT_SET2\n",ret);

    ret = readStatus();

    return ret;
}

int Caen785Module::readInfo()
{
    int ret = 0;

    uint32_t addr;

    addr = conf.base_addr + CAEN785_FIRMWARE;
    ret = iface->readA32D16(addr,&firmware);
    if(ret != 0) printf("Error %d at CAEN785_FIRMWARE read\n",ret);

    return ret;
}

int Caen785Module::readStatus1()
{
    int ret = 0;

    uint32_t addr;

    addr = conf.base_addr + CAEN785_STAT1;
    ret = iface->readA32D16(addr,&status1);
    if(ret != 0) printf("Error %d at CAEN785_STAT1 read\n",ret);

    return ret;
}

int Caen785Module::readStatus2()
{
    int ret = 0;

    uint32_t addr;

    addr = conf.base_addr + CAEN785_STAT2;
    ret = iface->readA32D16(addr,&status2);
    if(ret != 0) printf("Error %d at CAEN785_STAT2 read\n",ret);

    return ret;
}

int Caen785Module::readStatus()
{
    int ret = 0;

    uint32_t addr;
    uint16_t data;

    addr = conf.base_addr + CAEN785_BIT_SET1;
    ret = iface->readA32D16(addr,&bit1);
    if(ret != 0) printf("Error %d at CAEN785_BIT_SET1 read\n",ret);
    addr = conf.base_addr + CAEN785_BIT_SET2;
    ret = iface->readA32D16(addr,&bit2);
    if(ret != 0) printf("Error %d at CAEN785_BIT_SET2 read\n",ret);

    ret = readStatus1();
    ret = readStatus2();

    addr = conf.base_addr + CAEN785_EVCNT_H;
    ret = iface->readA32D16(addr,&data);
    if(ret != 0) printf("Error %d at CAEN785_EVCNT_H read\n",ret);
    evcntr = 0x0 | (data << 16);
    addr = conf.base_addr + CAEN785_EVCNT_L;
    ret = iface->readA32D16(addr,&data);
    if(ret != 0) printf("Error %d at CAEN785_EVCNT_L read\n",ret);
    evcntr |= data;

//    printf("bit1 %08x\n",bit1);
//    printf("bit2 %08x\n",bit2);
//    printf("status1 %08x\n",status1);
//    printf("status2 %08x\n",status2);
//    printf("CAEN785_EVCNT_H %08x\n",data);
//    printf("CAEN785_EVCNT_L %08x\n",data);

    return ret;
}

bool Caen785Module::dataReady()
{
    readStatus1();
    return (status1 & 0x1);
}

int Caen785Module::singleShot()
{
    softReset();
    configure();
    if(pollTrigger()) return acquireSingleEventMBLT();
    return -1;
}

bool Caen785Module::pollTrigger()
{
    printf("Polling.\n");fflush(stdout);
    bool triggered = true;
    uint32_t pollcount = 0;
    conf.pollcount = 100000;

    while(!dataReady())
    {
        pollcount++;
        if(pollcount == conf.pollcount)
        {
            triggered = false;
            break;
        }
    }

    if(!triggered)
    {
        printf("No trigger in %d polls.\n",pollcount);fflush(stdout);
        return false;
    }
    else
    {
        printf("Triggered after %d polls.\n",pollcount);fflush(stdout);
        return true;
    }
}

int Caen785Module::acquire()
{
    int ret = 0;
    ret = acquireSingleEventMBLT();
    //ret =  acquireSingleEvent();
    //ret =  acquireSingleEventFIFO();
    if(ret > 0) writeToBuffer(ret);
    return ret;
}

int Caen785Module::writeToBuffer(uint32_t nofWords)
{
    DemuxCaenADCPlugin *o = dynamic_cast<DemuxCaenADCPlugin*> (output);

    bool go_on = o->processData (data, nofWords, RunManager::ref ().isSingleEventMode ());
    if (!go_on)
        dataReset ();
    return 0;
}

int Caen785Module::acquireSingleEvent()
{
    int ret = 0;
    uint32_t addr;

    int i = 0;

    addr = conf.base_addr + CAEN785_MEB;
    while(i < 34)
    {
        ret = iface->readA32D32(addr,&data[i]);
        if(ret != 0) printf("Error %d at CAEN785_MEB read %d\n",ret,i);
        i++;
    }

    return 34;
}

int Caen785Module::acquireSingleEventMBLT()
{
    int ret = 0;

    uint32_t addr;
    uint32_t nofRead;

    addr = conf.base_addr + CAEN785_MEB;
    ret = iface->readA32MBLT64(addr,data,34,&nofRead);      // 455 ms
    if(ret != 0)
    {
        printf("Error %d at CAEN785_MEB readA32MBLT64\n",ret);fflush(stdout);
        return -1;
    }
    //printf("Read %d words\n",nofRead);fflush(stdout);

    return nofRead;
}

int Caen785Module::acquireSingleEventFIFO()
{
    int ret = 0;

    uint32_t addr;
    uint32_t nofRead;

    addr = conf.base_addr + CAEN785_MEB;
    ret = iface->readA32FIFO(addr,data,34,&nofRead);      // 527 ms
    if(ret != 0)
    {
        printf("Error %d at CAEN785_MEB readA32MBLT64\n",ret);
        return -1;
    }

    return nofRead;
}

void Caen785Module::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());

    bool ok = true;

    set = "base_addr";   if(settings->contains(set)) conf.base_addr = settings->value(set).toInt(&ok);

    set = "align64";    if(settings->contains(set)) conf.align64 = settings->value(set).toBool();
    set = "alwaysIncrementEventCounter";    if(settings->contains(set)) conf.alwaysIncrementEventCounter = settings->value(set).toBool();
    set = "autoIncrementEnabled";    if(settings->contains(set)) conf.autoIncrementEnabled = settings->value(set).toBool();
    set = "berr_enable";    if(settings->contains(set)) conf.berr_enable = settings->value(set).toBool();
    set = "block_end";    if(settings->contains(set)) conf.block_end = settings->value(set).toBool();
    set = "emptyEventWriteEnabled";    if(settings->contains(set)) conf.emptyEventWriteEnabled = settings->value(set).toBool();
    set = "highResSlidingScale";    if(settings->contains(set)) conf.highResSlidingScale = settings->value(set).toBool();
    set = "memTestModeEnabled";    if(settings->contains(set)) conf.memTestModeEnabled = settings->value(set).toBool();
    set = "offline";    if(settings->contains(set)) conf.offline = settings->value(set).toBool();
    set = "overRangeSuppressionEnabled";    if(settings->contains(set)) conf.overRangeSuppressionEnabled = settings->value(set).toBool();
    set = "program_reset";    if(settings->contains(set)) conf.program_reset = settings->value(set).toBool();
    set = "slideSubEnabled";    if(settings->contains(set)) conf.slideSubEnabled = settings->value(set).toBool();
    set = "slidingScaleEnabled";    if(settings->contains(set)) conf.slidingScaleEnabled = settings->value(set).toBool();
    set = "zeroSuppressionEnabled";    if(settings->contains(set)) conf.zeroSuppressionEnabled = settings->value(set).toBool();

    set = "cratenumber";    if(settings->contains(set)) conf.cratenumber = settings->value(set).toInt(&ok);
    set = "irq_level";    if(settings->contains(set)) conf.irq_level = settings->value(set).toInt(&ok);
    set = "irq_vector";    if(settings->contains(set)) conf.irq_vector = settings->value(set).toInt(&ok);
    set = "nof_events";    if(settings->contains(set)) conf.nof_events = settings->value(set).toInt(&ok);
    set = "pollcount";    if(settings->contains(set)) conf.pollcount = settings->value(set).toInt(&ok);
    set = "slide_constant";    if(settings->contains(set)) conf.slide_constant = settings->value(set).toInt(&ok);
    set = "zeroSuppressionThr";    if(settings->contains(set)) conf.zeroSuppressionThr = settings->value(set).toInt(&ok);

    for(unsigned int i = 0; i < 32; i++)
    {
        set = tr("killChannel%1").arg(i,1,10,QChar());
        if(settings->contains(set)) conf.killChannel[i] = settings->value(set).toBool();
        set = tr("thresholds%1").arg(i,1,10,QChar());
        if(settings->contains(set)) conf.thresholds[i] = settings->value(set).toInt(&ok);
    }

    settings->endGroup();

    getUI()->applySettings();
}

void Caen785Module::saveSettings(QSettings* settings)
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

        settings->setValue("align64",conf.align64);
        settings->setValue("alwaysIncrementEventCounter",conf.alwaysIncrementEventCounter);
        settings->setValue("autoIncrementEnabled",conf.autoIncrementEnabled);
        settings->setValue("berr_enable",conf.berr_enable);
        settings->setValue("block_end",conf.block_end);
        settings->setValue("emptyEventWriteEnabled",conf.emptyEventWriteEnabled);
        settings->setValue("highResSlidingScale",conf.highResSlidingScale);
        settings->setValue("memTestModeEnabled",conf.memTestModeEnabled);
        settings->setValue("offline",conf.offline);
        settings->setValue("overRangeSuppressionEnabled",conf.overRangeSuppressionEnabled);
        settings->setValue("program_reset",conf.program_reset);
        settings->setValue("slideSubEnabled",conf.slideSubEnabled);
        settings->setValue("slidingScaleEnabled",conf.slidingScaleEnabled);
        settings->setValue("zeroSuppressionEnabled",conf.zeroSuppressionEnabled);

        settings->setValue("cratenumber",conf.cratenumber);
        settings->setValue("irq_level",conf.irq_level);
        settings->setValue("irq_vector",conf.irq_vector);
        settings->setValue("nof_events",conf.nof_events);
        settings->setValue("pollcount",conf.pollcount);
        settings->setValue("slide_constant",conf.slide_constant);
        settings->setValue("zeroSuppressionThr",conf.zeroSuppressionThr);

        for(unsigned int i = 0; i < 32; i++)
        {
            settings->setValue(tr("killChannel%1").arg(i,1,10,QChar()),conf.killChannel[i]);
            settings->setValue(tr("thresholds%1").arg(i,1,10,QChar()),conf.thresholds[i]);
        }

        settings->endGroup();
        std::cout << " done" << std::endl;
    }

}

void Caen785Module::setBaseAddress (uint32_t baddr) {
    conf.base_addr = baddr;
    getUI ()->applySettings ();
}

uint32_t Caen785Module::getBaseAddress () const {
    return conf.base_addr;
}
