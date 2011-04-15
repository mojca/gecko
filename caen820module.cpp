#include "caen820module.h"
#include "caen820ui.h"
#include "caen820dmx.h"
#include "caen_v820.h"
#include "confmap.h"
#include "modulemanager.h"
#include "runmanager.h"
#include "scopechannel.h"

#include <vector>

static ModuleRegistrar reg ("caen820", &Caen820Module::create);

AbstractModule *Caen820Module::create (int id, const QString &name) {
    return new Caen820Module (id, name);
}

Caen820Module::Caen820Module (int id, const QString &name)
: BaseModule (id, name)
, dmx_ (evslots_)
{
    setUI (new Caen820UI (this));
    setChannels ();
    createOutputPlugin ();
}

void Caen820Module::setChannels () {
    EventBuffer *evbuf = RunManager::ref().getEventBuffer();
    for (int i = 0; i < 32; ++i)
        evslots_ << evbuf->registerSlot (this, QString ("out %1").arg (i), PluginConnector::VectorUint32);
}

int Caen820Module::configure () {
    int err = 0;
    const uint32_t &baddr = conf_.baddr;
    AbstractInterface *iface = getInterface ();

    if (!iface)
        return -1;

    if ((err = iface->writeA32D16 (baddr + CAEN820_CONTROL,
                (conf_.acq_mode & 0x03) |
                (conf_.berr_enable ? (1 << CAEN820_CTL_BERREN) : 0) |
                (conf_.clear_meb ? (1 << CAEN820_CTL_CLRMEB) : 0) |
                (conf_.hdr_enable ? (1 << CAEN820_CTL_HDREN) : 0) |
                (conf_.short_data_format ? (1 << CAEN820_CTL_DFMT) : 0) |
                (conf_.auto_reset ? (1 << CAEN820_CTL_AUTORST) : 0)
                )))
        std::cout << "Error " << err << " at CAEN820_CONTROL" << std::endl;

    if ((err = iface->writeA32D32 (baddr + CAEN820_CH_EN, conf_.channel_enable)))
        std::cout << "Error " << err << " at CAEN820_CH_EN" << std::endl;

    if ((err = iface->writeA32D32 (baddr + CAEN820_DWELL, conf_.dwell_time)))
        std::cout << "Error " << err << " at CAEN820_CH_EN" << std::endl;

    dmx_.setChannelBitmap (conf_.channel_enable);
    dmx_.setHeaderEnabled (conf_.hdr_enable);
    dmx_.setShortDataFmt (conf_.short_data_format);

    return err;
}

bool Caen820Module::dataReady () {
    int err;
    uint16_t stat;
    AbstractInterface *iface = getInterface ();

    if (!iface)
        return false;

    if ((err = iface->readA32D16 (conf_.baddr + CAEN820_STATUS, &stat))) {
        std::cout << "Error " << err << " at CAEN820_STATUS" << std::endl;
        return false;
    }

    return (stat & (1 << CAEN820_STA_DREADY)) != 0;
}

int Caen820Module::reset () {
    int err = 0;
    AbstractInterface *iface = getInterface ();

    if (!iface)
        return -1;

    if ((err = iface->writeA32D16 (conf_.baddr + CAEN820_SRESET, 1)))
        std::cout << "Error " << err << " at CAEN820_SRESET" << std::endl;

    return err;
}

int Caen820Module::dataClear () {
    int err = 0;
    AbstractInterface *iface = getInterface ();

    if (!iface)
        return -1;

    if ((err = iface->writeA32D16 (conf_.baddr + CAEN820_SCLR, 1)))
        std::cout << "Error " << err << " at CAEN820SCLR" << std::endl;
    return err;
}

int Caen820Module::acquire (Event *ev) {
    int err = 0;
    unsigned evlen = getNofActiveChannels ();
    uint32_t buffer [33] = {0};
    AbstractInterface *iface = getInterface ();

    if (!iface)
        return -1;

    while (!err) {
        int toread = evlen + (conf_.hdr_enable ? 1 : 0);
        uint32_t got = 0;

        // only error if not the whole event has been transferred
        if ((err = iface->readA32BLT32 (conf_.baddr + CAEN820_MEB, buffer, toread, &got)) && got != evlen) {
            if (got == 0 && iface->isBusError (err))
                break;
            std::cout << "Error " << err << " reading CAEN820_MEB" << std::endl;
            return err;
        }

        bool sem = RunManager::ref ().isSingleEventMode ();
        bool go_on = dmx_.processData (ev, buffer, got);
        if (sem || ! go_on) {
            dataClear ();
            break;
        }
    }

    return 0;
}

int Caen820Module::getNofActiveChannels () {
    int cnt = 0;
    for (int i = 0; i < 32; ++i)
        if (conf_.channel_enable & (1 << i))
            ++cnt;

    return cnt;
}

typedef ConfMap::confmap_t<Caen820Config> confmap_t;
static const confmap_t confmap [] = {
    confmap_t ("base_addr", &Caen820Config::baddr),
    confmap_t ("acq_mode", &Caen820Config::acq_mode),
    confmap_t ("dwell_time", &Caen820Config::dwell_time),
    confmap_t ("channel_enable", &Caen820Config::channel_enable),
    confmap_t ("short_data_format", &Caen820Config::short_data_format),
    confmap_t ("berren", &Caen820Config::berr_enable),
    confmap_t ("header_enable", &Caen820Config::hdr_enable),
    confmap_t ("clear_meb", &Caen820Config::clear_meb),
    confmap_t ("auto_reset", &Caen820Config::auto_reset)
};

void Caen820Module::applySettings (QSettings *s) {
    std::cout << "Applying settings for " << getName ().toStdString () << "... ";
    s->beginGroup (getName ());
    ConfMap::apply (s, &conf_, confmap);
    s->endGroup ();

    getUI ()->applySettings ();
    std::cout << "done" << std::endl;
}

void Caen820Module::saveSettings (QSettings *s) {
    std::cout << "Saving settings for " << getName ().toStdString () << "... ";
    s->beginGroup (getName ());
    ConfMap::save (s, &conf_, confmap);
    s->endGroup ();
    std::cout << "done" << std::endl;
}

void Caen820Module::setBaseAddress (uint32_t baddr) {
    conf_.baddr = baddr;
}

uint32_t Caen820Module::getBaseAddress () const {
    return conf_.baddr;
}

/*!
\page caen820mod Caen V820 Scaler
<b>Module name:</b> \c caen820

\section desc Module Description
The Caen V820 is a 32-channel Scaler.

\section cpanel Configuration Panel
Only a few options are necessary to control the operation mode of this scaler module.
\li <b>Enable header</b> causes the device to output a header before each event.
The header is not used yet, but safely discarded.
\li <b>26 bit data format</b> makes the module output data with less range (26 instead of 32 bits).
\li <b>Auto-reset on trigger</b> resets all scaler channels after each trigger.
\li <b>Acquisition mode</b> can either be set to \em periodic or \em triggered.
Periodic mode causes an event to be put into the event buffer every time the dwell time elapses, recording the current scaler values.
Triggered mode outputs an event every time the trigger input is signalled.
\li <b>Dwell time</b> is the interval between events in periodic mode. It can be set in steps of 400 ns.

Next is a grid where each of the channel may be enabled separately. Disabled channels will not be recorded in any event.

\section outs Outputs
This module has one output connector for each scaler channel. They contain single-element vectors for each event with the current scaler value as a \c uint32.
*/

