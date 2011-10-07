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

#include "caen965module.h"
#include "caen_v965.h"
#include "caen965ui.h"
#include "modulemanager.h"
#include "runmanager.h"
#include "confmap.h"

#include <cstdio>
#include <cstring>

using namespace std;
static ModuleRegistrar reg1 ("caen965", Caen965Module::create);

Caen965Module::Caen965Module (int i, const QString &n)
    : BaseModule (i, n)
    , info_(0)
    , bitset1 (0)
    , bitset2 (0)
    , status1 (0)
    , status2 (0)
    , evcnt   (0)
    , dmx_    (evslots_, this)
{
    conf_.pollcount = 100000;
    setChannels ();
    createOutputPlugin();

    setUI (new Caen965UI (this));
        std::cout << "Instantiated Caen965 module" << std::endl;
}


void Caen965Module::setChannels () {
    EventBuffer *evbuf = RunManager::ref ().getEventBuffer ();
    for(int i = 0; i < CAEN_V965_NOF_CHANNELS*2; i++)
        evslots_ << evbuf->registerSlot (this, tr("out %1").arg(i,1,10), PluginConnector::VectorUint32);
}

int Caen965Module::configure () {
    AbstractInterface *iface = getInterface ();
    uint32_t baddr = conf_.base_addr;
    uint16_t data;
    int ret = 0;

    // set crate number
    ret = iface->writeA32D16 (baddr + CAEN965_CRATE_SEL, conf_.cratenumber);
    if (ret) printf ("Error %d at CAEN965_CRATE_SEL\n", ret);

    // set geo address
    ret = iface->writeA32D16(baddr + CAEN965_GEO_ADDR, 0x00);
    if (ret) printf ("Error %d at CAEN965_GEO_ADDR", ret);

    // set channel thresholds and kill bits
    for (int i = 0; i < 32; ++i) {
        ret = iface->writeA32D16 (baddr + CAEN965_THRESHOLDS + 2*i,
                                  conf_.thresholds [i]
                                  | (conf_.killChannel [i] ? (1 << CAEN965_THRESH_KILL) : 0));
        if (ret) printf ("Error %d at CAEN965_THRESHOLDS[%d]\n", ret, i);
    }

    ret = iface->writeA32D16 (baddr + CAEN965_CONTROL1,
            (conf_.block_end ?      (1 << CAEN965_C1_BLKEND)    : 0 ) |
            (conf_.berr_enable ?    (1 << CAEN965_C1_BERREN)    : 0 ) |
            (conf_.program_reset ?  (1 << CAEN965_C1_PROGRST)   : 0 ) |
            (conf_.align64 ?        (1 << CAEN965_C1_ALIGN64)   : 0 ));
    if (ret) printf ("Error %d at CAEN965_CONTROL1\n", ret);

    ret = iface->writeA32D16 (baddr + CAEN965_IRQ_LVL, conf_.irq_level);
    if (ret) printf ("Error %d at CAEN965_IRQ_LVL\n", ret);

    ret = iface->writeA32D16 (baddr + CAEN965_IRQ_VEC, conf_.irq_vector);
    if (ret) printf ("Error %d at CAEN965_IRQ_VEC\n", ret);

    ret = iface->writeA32D16 (baddr + CAEN965_EV_TRG, conf_.ev_trg);
    if (ret) printf ("Error %d at CAEN965_EV_TRG\n", ret);

    data =
        (conf_.memTestModeEnabled?          (1 << CAEN965_B2_TESTMEM)   : 0) |
        (conf_.offline?                     (1 << CAEN965_B2_OFFLINE)   : 0) |
        (!conf_.overRangeSuppressionEnabled?(1 << CAEN965_B2_OVDIS)     : 0) |
        (!conf_.zeroSuppressionEnabled?     (1 << CAEN965_B2_UNDIS)     : 0) |
        (conf_.slidingScaleEnabled?         (1 << CAEN965_B2_SLIDEN)    : 0) |
        (conf_.zeroSuppressionThr?          (1 << CAEN965_B2_STEPTH)    : 0) |
        (conf_.autoIncrementEnabled?        (1 << CAEN965_B2_AUTOINC)   : 0) |
        (conf_.emptyEventWriteEnabled?      (1 << CAEN965_B2_EMPTYEN)   : 0) |
        (!conf_.slideSubEnabled?             (1 << CAEN965_B2_SLSUBEN)   : 0) |
        (conf_.alwaysIncrementEventCounter? (1 << CAEN965_B2_ALLTRG)    : 0);
    ret = iface->writeA32D16 (baddr + CAEN965_BIT_SET2, data & CAEN965_B2_MASK);
    if (ret) printf ("Error %d at CAEN965_BIT_SET2\n", ret);

    ret = iface->writeA32D16 (baddr + CAEN965_BIT_CLR2, (~data) & CAEN965_B2_MASK);
    if (ret) printf ("Error %d at CAEN965_BIT_CLR2\n", ret);

    ret = iface->writeA32D16 (baddr + CAEN965_FCLR_WND, conf_.fastclear);
    if (ret) printf ("Error %d at CAEN965_FCLR_WND\n", ret);

    ret = iface->writeA32D16 (baddr + CAEN965_SLD_CONSTANT, conf_.slideconst);
    if (ret) printf ("Error %d at CAEN965_SLD_CONSTANT", ret);

    ret = iface->writeA32D16 (baddr + CAEN965_IPED, conf_.i_ped);
    if (ret) printf ("Error %d at CAEN965_IPED\n", ret);

    ret = counterReset ();
    return ret;
}

int Caen965Module::counterReset () {
    return getInterface ()->writeA32D16 (conf_.base_addr + CAEN965_EVCNT_RST, 0x00);
}

int Caen965Module::dataReset () {
	int ret = 0;

    ret = getInterface ()->writeA32D16(conf_.base_addr + CAEN965_BIT_SET2, (1 << CAEN965_B2_CLRDATA));
	if (ret) return ret;

    ret = getInterface ()->writeA32D16(conf_.base_addr + CAEN965_BIT_CLR2, (1 << CAEN965_B2_CLRDATA));
	return ret;
}

int Caen965Module::softReset () {
    int ret = 0;

    ret = getInterface ()->writeA32D16(conf_.base_addr + CAEN965_BIT_SET1, (1 << CAEN965_B1_SOFTRST));
    if (ret) {
        std::cout << "Error " << ret << " at Caen965 Soft Reset!" << std::endl;
        return ret;
    }

    ret = getInterface ()->writeA32D16(conf_.base_addr + CAEN965_BIT_CLR1, (1 << CAEN965_B1_SOFTRST));
    if (ret)
        std::cout << "Error " << ret << " at Caen965 Soft Reset!" << std::endl;
    return ret;
}

int Caen965Module::reset () {
    counterReset();
    dataReset();
    return softReset ();

}

uint16_t Caen965Module::getInfo () const {
	if (!info_)
        getInterface ()->readA32D16 (conf_.base_addr + CAEN965_FIRMWARE, &info_);
	return info_;
}

int Caen965Module::readStatus () {
    AbstractInterface *iface = getInterface ();
    int ret;
    uint16_t data;

    ret = iface->readA32D16 (conf_.base_addr + CAEN965_BIT_SET1, &bitset1);
    if (ret)
            printf ("Error %d at CAEN965_BIT_SET1\n", ret);
    ret = iface->readA32D16 (conf_.base_addr + CAEN965_BIT_SET2, &bitset2);
    if (ret)
            printf ("Error %d at CAEN965_BIT_SET2\n", ret);
    ret = iface->readA32D16 (conf_.base_addr + CAEN965_STAT1, &status1);
    if (ret)
            printf ("Error %d at CAEN965_STAT1\n", ret);
    ret = iface->readA32D16 (conf_.base_addr + CAEN965_STAT2, &status2);
    if (ret)
            printf ("Error %d at CAEN965_STAT2\n", ret);
    ret = iface->readA32D16 (conf_.base_addr + CAEN965_EVCNT_L, &data);
    if (ret)
            printf ("Error %d at CAEN965_EVCNT_L\n", ret);
    evcnt = data;
    ret = iface->readA32D16 (conf_.base_addr + CAEN965_EVCNT_H, &data);
    if (ret)
        printf ("Error %d at CAEN965_EVCNT_H\n", ret);
    evcnt |= (((uint32_t)(data) & 0x000000ff) << 16);
    return 0;
}

bool Caen965Module::dataReady () {
    int ret;
    ret = getInterface ()->readA32D16 (conf_.base_addr + CAEN965_STAT1, &status1);
    if (ret)
        printf ("Error %d at CAEN965_STAT1\n", ret);

    return (status1 & (1 << CAEN965_S1_DREADY)) != 0;
}

int Caen965Module::acquire (Event* ev) {
    int ret;

    ret = acquireSingle (data, &rd);
    if (ret == 0) writeToBuffer(ev);
    else printf("Error at acquireSingle\n");

    return rd;
}

void Caen965Module::writeToBuffer(Event *ev)
{
    bool go_on = dmx_.processData (ev, data, rd, RunManager::ref ().isSingleEventMode ());
    if (!go_on)
        dataReset ();
}

int Caen965Module::acquireSingle (uint32_t *data, uint32_t *rd) {
    *rd = 0;

    uint32_t addr = conf_.base_addr + CAEN965_MEB;
    //printf("caen965: acquireSingle from addr = 0x%08x",addr);
    int ret = getInterface ()->readA32MBLT64 (addr, data, CAEN_V965_MAX_NOF_WORDS, rd);
    if (!*rd && ret && !getInterface ()->isBusError (ret)) {
        printf ("Error %d at CAEN965_MEB\n", ret);
        return ret;
    }

    /*printf("\nEvent dump:\n");
    for(int i =0; i < (*rd); ++i) {
        printf("<%d> 0x%08x\n",i,data[i]);
    }
    printf("\n"); fflush(stdout);*/

    return 0;
}

void Caen965Module::singleShot (uint32_t *data, uint32_t *rd) {
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

typedef ConfMap::confmap_t<Caen965ModuleConfig> confmap_t;
static const confmap_t confmap [] = {
    confmap_t ("align64", &Caen965ModuleConfig::align64),
    confmap_t ("alwaysIncrementEventCounter", &Caen965ModuleConfig::alwaysIncrementEventCounter),
    confmap_t ("autoIncrementEnabled", &Caen965ModuleConfig::autoIncrementEnabled),
    confmap_t ("base_addr", &Caen965ModuleConfig::base_addr),
    confmap_t ("berr_enable", &Caen965ModuleConfig::berr_enable),
    confmap_t ("block_end", &Caen965ModuleConfig::block_end),
    confmap_t ("cratenumber", &Caen965ModuleConfig::cratenumber),
    confmap_t ("emptyEventWriteEnabled", &Caen965ModuleConfig::emptyEventWriteEnabled),
    confmap_t ("ev_trg", &Caen965ModuleConfig::ev_trg),
    confmap_t ("fastclear", &Caen965ModuleConfig::fastclear),
    confmap_t ("slideconst", &Caen965ModuleConfig::slideconst),
    confmap_t ("i_ped", &Caen965ModuleConfig::i_ped),
    confmap_t ("irq_level", &Caen965ModuleConfig::irq_level),
    confmap_t ("irq_vector", &Caen965ModuleConfig::irq_vector),
    confmap_t ("memTestModeEnabled", &Caen965ModuleConfig::memTestModeEnabled),
    confmap_t ("offline", &Caen965ModuleConfig::offline),
    confmap_t ("overRangeSuppressionEnabled", &Caen965ModuleConfig::overRangeSuppressionEnabled),
    confmap_t ("pollcount", &Caen965ModuleConfig::pollcount),
    confmap_t ("program_reset", &Caen965ModuleConfig::program_reset),
    confmap_t ("slideSubEnabled", &Caen965ModuleConfig::slideSubEnabled),
    confmap_t ("slidingScaleEnabled", &Caen965ModuleConfig::slidingScaleEnabled),
    confmap_t ("zeroSuppressionEnabled", &Caen965ModuleConfig::zeroSuppressionEnabled),
    confmap_t ("zeroSuppressionThr", &Caen965ModuleConfig::zeroSuppressionThr),
    confmap_t ("tdc_stop_mode", &Caen965ModuleConfig::stop_mode),
    confmap_t ("tdc_fsr", &Caen965ModuleConfig::fsr)
};

void Caen965Module::applySettings (QSettings *settings) {
    std::cout << "Applying settings for " << getName ().toStdString () << "... ";
    settings->beginGroup (getName ());
    ConfMap::apply (settings, &conf_, confmap);
    for (int i = 0; i < 32; ++i) {
        QString key = QString ("thresholds%1").arg (i);
        if (settings->contains (key)) {
            conf_.thresholds [i] = settings->value (key).toUInt ();
            //printf("Found key %s with value %d\n",key.toStdString().c_str(),conf_.thresholds[i]);
        }
        key = QString ("killChannel%1").arg (i);
        if (settings->contains (key))
            conf_.killChannel [i] = settings->value (key).toBool ();
    }
    settings->endGroup ();
    std::cout << "done" << std::endl;

    getUI ()->applySettings ();
}

void Caen965Module::saveSettings (QSettings *settings) {
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

void Caen965Module::setBaseAddress (uint32_t baddr) {
    conf_.base_addr = baddr;
    getUI ()->applySettings ();
}

uint32_t Caen965Module::getBaseAddress () const {
    return conf_.base_addr;
}
/*!
\page caen965mod Caen V965 QDC
<b>Module name:</b> \c caen965

\section desc Module Description
The Caen V965 is a 16-channel Dual Range QDC.
It is in functionality and handling very similar to the V792 32-channel QDC.

\section cpanel Configuration Panel
\subsection devctrl Dev Ctrl
This panel contains facilities for basic functionality tests of the device. It will go away in a future version.

\subsection settings Settings
This panel controls the operation mode of the device.
\li <b>Block End</b> and <b>Berr En</b> control the way data is read from the module. The readout depends on the specific settings of these switches, so <b>DO NOT CHANGE</b>!
These will go away in a future version.
\li <b>Prog Reset</b> controls how the module reacts to pushing the reset button on its front panel. See the Caen V965 manual for further details.
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
See the Caen V965 manual for further details
\li <b>Slide subtract En</b> disables the subtraction of the sliding scale offset when checked. For testing purposes only.
\li <b>High resolution thresholds</b> causes the thresholds to only be shifted by one instead of 4 bits, thus increasing threshold resolution at the cost of range.

The last group contains parameters for fine-grained control over the mode of operation.
\li <b>Crate number</b> an arbitrary number that is passed back to GECKO when data is retrieved from the module. Unused as of now.
\li <b>Pedestal current</b> controls the amount of current always present at the QDC. This current is required for correct operation.
See the Caen V965 manual for further details and for converting this number into a current and back.
\li <b>Fastclear window</b> sets the with of the window during which a fast clear signal on the front panel connector aborts an ongoing conversion.
See the manual for further details
\li <b> Sliding constant</b> sets the value that is added to the ADC inputs when the sliding scale is disabled.

\subsection irq IRQ
The IRQ panel controls the conditions on which VME interrupt requests are generated. IRQs are not yet used in data readout.

\subsection info Info
The Info panel provides some basic information about the firmware that runs on the module.
This page is not fully implemented yet.

\section outs Outputs
The module provides two outputs for each QDC channel. These outputs contain single-element vectors with the output value (as \c uint32) of the respective ADC for each event.
One of the outputs carries the low range signal, while the other covers the high range.
*/

