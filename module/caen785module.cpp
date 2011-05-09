#include "caen785module.h"
#include "modulemanager.h"
#include "runmanager.h"

static ModuleRegistrar registrar ("caen785", Caen785Module::create);

Caen785Module::Caen785Module(int _id, QString _name)
        : BaseModule(_id, _name)
        , dmx (evslots, this)
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
    EventBuffer *evbuf = RunManager::ref ().getEventBuffer ();
    for (int i= 0; i < 32; ++i)
        evslots << evbuf->registerSlot (this, QString ("out %1").arg (i), PluginConnector::VectorUint32);
}

int Caen785Module::softReset()
{
    AbstractInterface *iface = getInterface ();
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
    AbstractInterface *iface = getInterface ();
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
    AbstractInterface *iface = getInterface ();
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
    AbstractInterface *iface = getInterface ();
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
    // TODO: transfer more flags
    ret = iface->writeA32D16(addr,data);
    if(ret != 0) printf("Error %d at CAEN785_BIT_SET2\n",ret);

    ret = readStatus();

    return ret;
}

int Caen785Module::readInfo()
{
    AbstractInterface *iface = getInterface ();
    int ret = 0;

    uint32_t addr;

    addr = conf.base_addr + CAEN785_FIRMWARE;
    ret = iface->readA32D16(addr,&firmware);
    if(ret != 0) printf("Error %d at CAEN785_FIRMWARE read\n",ret);

    return ret;
}

int Caen785Module::readStatus1()
{
    AbstractInterface *iface = getInterface ();
    int ret = 0;

    uint32_t addr;

    addr = conf.base_addr + CAEN785_STAT1;
    ret = iface->readA32D16(addr,&status1);
    if(ret != 0) printf("Error %d at CAEN785_STAT1 read\n",ret);

    return ret;
}

int Caen785Module::readStatus2()
{
    AbstractInterface *iface = getInterface ();
    int ret = 0;

    uint32_t addr;

    addr = conf.base_addr + CAEN785_STAT2;
    ret = iface->readA32D16(addr,&status2);
    if(ret != 0) printf("Error %d at CAEN785_STAT2 read\n",ret);

    return ret;
}

int Caen785Module::readStatus()
{
    AbstractInterface *iface = getInterface ();
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

int Caen785Module::acquire(Event *ev)
{
    int ret = 0;
    ret = acquireSingleEventMBLT();
    //ret =  acquireSingleEvent();
    //ret =  acquireSingleEventFIFO();
    if(ret > 0) writeToBuffer(ev, ret);
    return ret;
}

int Caen785Module::writeToBuffer(Event *ev, uint32_t nofWords)
{
    bool go_on = dmx.processData (ev, data, nofWords, RunManager::ref ().isSingleEventMode ());
    if (!go_on)
        dataReset ();
    return 0;
}

int Caen785Module::acquireSingleEvent()
{
    AbstractInterface *iface = getInterface ();
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
    AbstractInterface *iface = getInterface ();
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
    AbstractInterface *iface = getInterface ();
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
    set = "memTestModeEnabled";    if(settings->contains(set)) conf.memTestModeEnabled = settings->value(set).toBool();
    set = "offline";    if(settings->contains(set)) conf.offline = settings->value(set).toBool();
    set = "overRangeSuppressionEnabled";    if(settings->contains(set)) conf.overRangeSuppressionEnabled = settings->value(set).toBool();
    set = "program_reset";    if(settings->contains(set)) conf.program_reset = settings->value(set).toBool();
    set = "slideSubEnabled";    if(settings->contains(set)) conf.slideSubEnabled = settings->value(set).toBool();
    set = "slidingScaleEnabled";    if(settings->contains(set)) conf.slidingScaleEnabled = settings->value(set).toBool();
    set = "zeroSuppressionEnabled";    if(settings->contains(set)) conf.zeroSuppressionEnabled = settings->value(set).toBool();
    set = "zeroSuppressionThr";    if(settings->contains(set)) conf.zeroSuppressionThr = settings->value(set).toBool();

    set = "cratenumber";    if(settings->contains(set)) conf.cratenumber = settings->value(set).toInt(&ok);
    set = "irq_level";    if(settings->contains(set)) conf.irq_level = settings->value(set).toInt(&ok);
    set = "irq_vector";    if(settings->contains(set)) conf.irq_vector = settings->value(set).toInt(&ok);
    set = "nof_events";    if(settings->contains(set)) conf.nof_events = settings->value(set).toInt(&ok);
    set = "pollcount";    if(settings->contains(set)) conf.pollcount = settings->value(set).toInt(&ok);
    set = "slide_constant";    if(settings->contains(set)) conf.slide_constant = settings->value(set).toInt(&ok);

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
        settings->setValue("memTestModeEnabled",conf.memTestModeEnabled);
        settings->setValue("offline",conf.offline);
        settings->setValue("overRangeSuppressionEnabled",conf.overRangeSuppressionEnabled);
        settings->setValue("program_reset",conf.program_reset);
        settings->setValue("slideSubEnabled",conf.slideSubEnabled);
        settings->setValue("slidingScaleEnabled",conf.slidingScaleEnabled);
        settings->setValue("zeroSuppressionEnabled",conf.zeroSuppressionEnabled);
        settings->setValue("zeroSuppressionThr",conf.zeroSuppressionThr);

        settings->setValue("cratenumber",conf.cratenumber);
        settings->setValue("irq_level",conf.irq_level);
        settings->setValue("irq_vector",conf.irq_vector);
        settings->setValue("nof_events",conf.nof_events);
        settings->setValue("pollcount",conf.pollcount);
        settings->setValue("slide_constant",conf.slide_constant);

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

/*!
\page caen785mod Caen V785 ADC
<b>Module name:</b> \c caen785

\section desc Module Description
The Caen V785 is a 32-channel ADC. 

\section cpanel Configuration Panel
The Caen 785 configuration panel features some status displays at the top and configuration tabs at the bottom.
\subsection devctrl Dev Ctrl Tab
The device control panel provides several buttons for performing device tests. This panel will disappear some time in the future.

\subsection settings Settings Tab
This panel controls the operation mode of the device.
\li <b>Block End</b> and <b>Berr En</b> control the way data is read from the module. The readout depends on the specific settings of these switches, so <b>DO NOT CHANGE</b>!
These will go away in a future version.
\li <b>Prog Reset</b> controls how the module reacts to pushing the reset button on its front panel. See the Caen V785 manual for further details.
\li <b>Align 64</b> causes the length of an event to be a multiple of 64 bits.
Use this switch if the VME interface cuts off the last 32-bit word from a block transfer.

The next section controls data acquisition.
\li <b>Suppress Over Range</b> causes the module to not report any values that exceed the maximum output value of the ADCs.
\li <b>Suppress Low Thr</b> causes the module to honor the thresholds set for each of the individual channels and not report any value that is less than this threshold value.
Note that the threshold values can not be set from the UI yet.
\li <b>Offline</b> takes the ADC controller offline. No conversions will be performed.
\li <b>Auto increment</b> automatically discards elements from the module's event buffer after they were read. <b>DO NOT CHANGE!</b> This switch will go away in a future version.
\li <b>All triggers En</b> causes the event counter to be incremented on all triggers, not only accepted ones.
\li <b>Write empty events</b> allows the module to write events that contain no data to the event buffer.
\li <b>Use sliding scale</b> causes the module to use a sliding scale to reduce non-linearity at the cost of range.
See the Caen V785 manual for further details
\li <b>Slide subtract En</b> disables the subtraction of the sliding scale offset when checked. For testing purposes only.

The last group contains parameters for fine-grained control over the mode of operation.
\li The crate number is an arbitrary number that is attached to each data set sent back from the module. It is currently not available to post-processing as it is ignored by the demultiplexer.
\li The slide constant controls the offset that is added during data sampling if the sliding scale is disabled. For further reference, please consult the V785 manual.

\subsection thr Thresholds Tab
\note This functionality is not implemented yet.

The thresholds tab controls the threshold for each channel. If thresholds are enabled in the Settings tab, only channels exceeding their threshold will be recorded in the data set sent back from the VME module. It is also possible to completely remove any channel from all data sets.

\subsection irq IRQ Tab
This tab is for testing purposes only and will disappear in future versions. Use at your own risk.

\subsection info Info Tab
The Info tab displays a readout of the VME module ROM containing information about the firmware the VME module is currently executing.

\section outs Outputs
The module has one output connector for each channel. These contain single-element vectors for each event with the ADC output value in \c uint32 format.
*/

