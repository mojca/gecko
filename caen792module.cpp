#include "caen792module.h"
#include "caen_v792.h"
#include "caen792ui.h"
#include "modulemanager.h"
#include "runmanager.h"
#include "confmap.h"

#include <cstdio>
#include <cstring>

using namespace std;
static ModuleRegistrar reg1 ("caen792", Caen792Module::createQdc, AbstractModule::TypeDAq);
static ModuleRegistrar reg2 ("caen775", Caen792Module::createTdc, AbstractModule::TypeDAq);

Caen792Module::Caen792Module (int i, const QString &n, bool _isqdc)
    : BaseDAqModule (i, n)
    , isqdc   (_isqdc)
    , bitset1 (0)
	, bitset2 (0)
	, status1 (0)
	, status2 (0)
	, evcnt   (0)
{
    conf_.pollcount = 100000;
    setChannels ();
    createBuffer ();
    createOutputPlugin();

    setUI (new Caen792UI (this, isqdc));
	std::cout << "Instantiated Caen792 module" << std::endl;
}


void Caen792Module::setChannels () {
    for(int i = 0; i < 32; i++)
        getChannels()->push_back (new ScopeChannel (this, tr("out %1").arg(i,1,10), ScopeCommon::eventBuffer, 34, 1));
    getChannels()->push_back (new ScopeChannel (this, "Poll Trigger", ScopeCommon::trigger, 1, 1));
}

void Caen792Module::createBuffer () {
    //buffer_ = ModuleManager::ptr ()->createBuffer (100*34, 34, getId ());
}

void Caen792Module::createOutputPlugin()
{
    output = new DemuxCaenADCPlugin (-getId (), getName () + " dmx", AbstractPlugin::Attributes ());
}

int Caen792Module::configure () {
	uint32_t baddr = conf_.base_addr;
	uint16_t data;
	int ret = 0;

	// set crate number
        ret = iface->writeA32D16 (baddr + CAEN792_CRATE_SEL, conf_.cratenumber);
	if (ret) printf ("Error %d at CAEN792_CRATE_SEL\n", ret);

	// set channel thresholds and kill bits
	for (int i = 0; i < 32; ++i) {
            ret = iface->writeA32D16 (baddr + CAEN792_THRESHOLDS + 2*i, conf_.thresholds [i] | (conf_.killChannel [i] ? (1 << CAEN792_THRESH_KILL) : 0));
            if (ret) printf ("Error %d at CAEN792_THRESHOLDS[%d]\n", ret, i);
	}

        ret = iface->writeA32D16 (baddr + CAEN792_CONTROL1,
                (conf_.block_end ?      (1 << CAEN792_C1_BLKEND)    : 0 ) |
                (conf_.berr_enable ?    (1 << CAEN792_C1_BERREN)    : 0 ) |
                (conf_.program_reset ?  (1 << CAEN792_C1_PROGRST)   : 0 ) |
                (conf_.align64 ?        (1 << CAEN792_C1_ALIGN64)   : 0 ));
	if (ret) printf ("Error %d at CAEN792_CONTROL1\n", ret);

        ret = iface->writeA32D16 (baddr + CAEN792_IRQ_LVL, conf_.irq_level);
	if (ret) printf ("Error %d at CAEN792_IRQ_LVL\n", ret);

        ret = iface->writeA32D16 (baddr + CAEN792_IRQ_VEC, conf_.irq_vector);
	if (ret) printf ("Error %d at CAEN792_IRQ_VEC\n", ret);

        ret = iface->writeA32D16 (baddr + CAEN792_EV_TRG, conf_.ev_trg);
        if (ret) printf ("Error %d at CAEN792_EV_TRG\n", ret);

       data =
        (conf_.memTestModeEnabled?          (1 << CAEN792_B2_TESTMEM)   : 0) |
        (conf_.offline?                     (1 << CAEN792_B2_OFFLINE)   : 0) |
        (!conf_.overRangeSuppressionEnabled?(1 << CAEN792_B2_OVDIS)     : 0) |
        (!conf_.zeroSuppressionEnabled?     (1 << CAEN792_B2_UNDIS)     : 0) |
        (conf_.slidingScaleEnabled?         (1 << CAEN792_B2_SLIDEN)    : 0) |
        (conf_.zeroSuppressionThr?          (1 << CAEN792_B2_STEPTH)    : 0) |
        (conf_.autoIncrementEnabled?        (1 << CAEN792_B2_AUTOINC)   : 0) |
        (conf_.emptyEventWriteEnabled?      (1 << CAEN792_B2_EMPTYEN)   : 0) |
        (conf_.slideSubEnabled?             (1 << CAEN792_B2_SLSUBEN)   : 0) |
        (conf_.alwaysIncrementEventCounter? (1 << CAEN792_B2_ALLTRG)    : 0) |
        (conf_.stop_mode?                   (1 << CAEN775_B2_STOPMODE)  : 0);
        ret = iface->writeA32D16 (baddr + CAEN792_BIT_SET2, data & (isqdc ? CAEN792_B2_MASK : CAEN775_B2_MASK));
	if (ret) printf ("Error %d at CAEN792_BIT_SET2\n", ret);

        ret = iface->writeA32D16 (baddr + CAEN792_BIT_CLR2, (~data) & (isqdc ? CAEN792_B2_MASK : CAEN775_B2_MASK));
	if (ret) printf ("Error %d at CAEN792_BIT_CLR2\n", ret);

        ret = iface->writeA32D16 (baddr + CAEN792_FCLR_WND, conf_.fastclear);
	if (ret) printf ("Error %d at CAEN792_FCLR_WND\n", ret);

        ret = iface->writeA32D16 (baddr + CAEN792_SLD_CONSTANT, conf_.slideconst);
        if (ret) printf ("Error %d at CAEN792_SLD_CONSTANT", ret);

    if (isqdc) {
        ret = iface->writeA32D16 (baddr + CAEN792_IPED, conf_.i_ped);
        if (ret) printf ("Error %d at CAEN792_IPED\n", ret);
    } else {
        ret = iface->writeA32D16 (baddr + CAEN775_FSR, conf_.fsr);
        if (ret) printf ("Error %d at CAEN792_FSR\n", ret);
    }

	ret = counterReset ();
	return ret;
}

int Caen792Module::counterReset () {
    return iface->writeA32D16 (conf_.base_addr + CAEN792_EVCNT_RST, 0x00);
}

int Caen792Module::dataReset () {
	int ret = 0;

    ret = iface->writeA32D16(conf_.base_addr + CAEN792_BIT_SET2, (1 << CAEN792_B2_CLRDATA));
	if (ret) return ret;

    ret = iface->writeA32D16(conf_.base_addr + CAEN792_BIT_CLR2, (1 << CAEN792_B2_CLRDATA));
	return ret;
}

int Caen792Module::softReset () {
    int ret = 0;

    ret = iface->writeA32D16(conf_.base_addr + CAEN792_BIT_SET1, (1 << CAEN792_B1_SOFTRST));
    if (ret) {
        std::cout << "Error " << ret << " at Caen792 Soft Reset!" << std::endl;
        return ret;
    }

    ret = iface->writeA32D16(conf_.base_addr + CAEN792_BIT_CLR1, (1 << CAEN792_B1_SOFTRST));
    if (ret)
        std::cout << "Error " << ret << " at Caen792 Soft Reset!" << std::endl;
    return ret;
}

int Caen792Module::reset () {
	return softReset ();
}

uint16_t Caen792Module::getInfo () const {
	if (!info_)
        iface->readA32D16 (conf_.base_addr + CAEN792_FIRMWARE, &info_);
	return info_;
}

int Caen792Module::readStatus () {
	int ret;
	uint16_t data;

    ret = iface->readA32D16 (conf_.base_addr + CAEN792_BIT_SET1, &bitset1);
	if (ret)
		printf ("Error %d at CAEN792_BIT_SET1\n", ret);
    ret = iface->readA32D16 (conf_.base_addr + CAEN792_BIT_SET2, &bitset2);
	if (ret)
		printf ("Error %d at CAEN792_BIT_SET2\n", ret);
    ret = iface->readA32D16 (conf_.base_addr + CAEN792_STAT1, &status1);
	if (ret)
		printf ("Error %d at CAEN792_STAT1\n", ret);
    ret = iface->readA32D16 (conf_.base_addr + CAEN792_STAT2, &status2);
	if (ret)
		printf ("Error %d at CAEN792_STAT2\n", ret);
    ret = iface->readA32D16 (conf_.base_addr + CAEN792_EVCNT_L, &data);
	if (ret)
		printf ("Error %d at CAEN792_EVCNT_L\n", ret);
	evcnt = data;
    ret = iface->readA32D16 (conf_.base_addr + CAEN792_EVCNT_H, &data);
	if (ret)
		printf ("Error %d at CAEN792_EVCNT_H\n", ret);
	evcnt |= (data << 16);
	return 0;
}

bool Caen792Module::dataReady () {
    int ret;
    ret = iface->readA32D16 (conf_.base_addr + CAEN792_STAT1, &status1);
    if (ret)
        printf ("Error %d at CAEN792_STAT1\n", ret);

    return (status1 & (1 << CAEN792_S1_DREADY)) != 0;
}

int Caen792Module::acquire () {
	int ret;

	ret = acquireSingle (data, &rd);
        if (ret == 0) writeToBuffer();

        //buffer_->write (data, rd);
        return rd;
}

void Caen792Module::writeToBuffer()
{
    DemuxCaenADCPlugin* o = dynamic_cast<DemuxCaenADCPlugin*>(output);

    bool go_on = o->processData (data,rd,RunManager::ref ().isSingleEventMode ());
    if (!go_on)
        dataReset ();
}

int Caen792Module::acquireSingle (uint32_t *data, uint32_t *rd) {
    *rd = 0;
    int ret = iface->readA32MBLT64 (conf_.base_addr + CAEN792_MEB, data, 34, rd);
    if (!*rd && ret && !iface->isBusError (ret)) {
        printf ("Error %d at CAEN792_MEB\n", ret);
        return ret;
    }
    return 0;
}

void Caen792Module::singleShot (uint32_t *data, uint32_t *rd) {
	bool triggered = false;
	for (unsigned int i = 0; i < conf_.pollcount; ++i) {
		if (dataReady ()) {
			triggered = true;
		}
	}

	if (!triggered)
		std::cout << "No data after " << conf_.pollcount << " trigger loops" << std::endl;
	else
		acquireSingle (data, rd);
}

typedef ConfMap::confmap_t<Caen792ModuleConfig> confmap_t;
static const confmap_t confmap [] = {
	confmap_t ("align64", &Caen792ModuleConfig::align64),
	confmap_t ("alwaysIncrementEventCounter", &Caen792ModuleConfig::alwaysIncrementEventCounter),
	confmap_t ("autoIncrementEnabled", &Caen792ModuleConfig::autoIncrementEnabled),
	confmap_t ("base_addr", &Caen792ModuleConfig::base_addr),
	confmap_t ("berr_enable", &Caen792ModuleConfig::berr_enable),
	confmap_t ("block_end", &Caen792ModuleConfig::block_end),
	confmap_t ("cratenumber", &Caen792ModuleConfig::cratenumber),
	confmap_t ("emptyEventWriteEnabled", &Caen792ModuleConfig::emptyEventWriteEnabled),
	confmap_t ("ev_trg", &Caen792ModuleConfig::ev_trg),
	confmap_t ("fastclear", &Caen792ModuleConfig::fastclear),
        confmap_t ("slideconst", &Caen792ModuleConfig::slideconst),
	confmap_t ("i_ped", &Caen792ModuleConfig::i_ped),
	confmap_t ("irq_level", &Caen792ModuleConfig::irq_level),
	confmap_t ("irq_vector", &Caen792ModuleConfig::irq_vector),
	confmap_t ("memTestModeEnabled", &Caen792ModuleConfig::memTestModeEnabled),
	confmap_t ("offline", &Caen792ModuleConfig::offline),
	confmap_t ("overRangeSuppressionEnabled", &Caen792ModuleConfig::overRangeSuppressionEnabled),
	confmap_t ("pollcount", &Caen792ModuleConfig::pollcount),
	confmap_t ("program_reset", &Caen792ModuleConfig::program_reset),
	confmap_t ("slideSubEnabled", &Caen792ModuleConfig::slideSubEnabled),
	confmap_t ("slidingScaleEnabled", &Caen792ModuleConfig::slidingScaleEnabled),
	confmap_t ("zeroSuppressionEnabled", &Caen792ModuleConfig::zeroSuppressionEnabled),
    confmap_t ("zeroSuppressionThr", &Caen792ModuleConfig::zeroSuppressionThr),
    confmap_t ("tdc_stop_mode", &Caen792ModuleConfig::stop_mode),
    confmap_t ("tdc_fsr", &Caen792ModuleConfig::fsr)
};

void Caen792Module::applySettings (QSettings *settings) {
    std::cout << "Applying settings for " << getName ().toStdString () << "... ";
    settings->beginGroup (getName ());
    ConfMap::apply (settings, &conf_, confmap);
	for (int i = 0; i < 32; ++i) {
		QString key = QString ("thresholds%1").arg (i);
		if (settings->contains (key))
			conf_.thresholds [i] = settings->value (key).toInt ();
		key = QString ("killChannel%1").arg (i);
		if (settings->contains (key))
			conf_.killChannel [i] = settings->value (key).toBool ();
	}
	settings->endGroup ();
	std::cout << "done" << std::endl;

    getUI ()->applySettings ();
}

void Caen792Module::saveSettings (QSettings *settings) {
    std::cout << "Saving settings for " << getName ().toStdString () << "... ";
    settings->beginGroup (getName ());
    ConfMap::save (settings, &conf_, confmap);
    for (int i = 0; i < 32; ++i) {
		QString key = QString ("thresholds%1").arg (i);
		settings->setValue (key, conf_.thresholds [i]);
		key = QString ("killChannel%1").arg (i);
		settings->setValue (key, conf_.killChannel [i]);
	}
	settings->endGroup ();
	std::cout << "done" << std::endl;
}

void Caen792Module::setBaseAddress (uint32_t baddr) {
    conf_.base_addr = baddr;
    getUI ()->applySettings ();
}

uint32_t Caen792Module::getBaseAddress () const {
    return conf_.base_addr;
}
