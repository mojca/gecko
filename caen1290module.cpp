#include "caen1290module.h"
#include "caen_v1290.h"
#include "modulemanager.h"
#include "runmanager.h"
#include "demuxcaen1290plugin.h"

#include "scopechannel.h"
#include "caen1290ui.h"
#include "confmap.h"

#include <vector>
#include <cstdio>

static ModuleRegistrar reg1 ("caen1290a", Caen1290Module::create1290a, AbstractModule::TypeDAq);
static ModuleRegistrar reg2 ("caen1290n", Caen1290Module::create1290n, AbstractModule::TypeDAq);
static ModuleRegistrar reg3 ("caen1190a", Caen1290Module::create1190a, AbstractModule::TypeDAq);
static ModuleRegistrar reg4 ("caen1190b", Caen1290Module::create1190b, AbstractModule::TypeDAq);

// initialise "enum" constants
const Caen1290Module::ModuleType Caen1290Module::ModuleType::V1290A (V1290A_v, 0x00000005000A0000LL, 32, 4);
const Caen1290Module::ModuleType Caen1290Module::ModuleType::V1290N (V1290N_v, 0x00000005000A0002LL, 16, 2);
const Caen1290Module::ModuleType Caen1290Module::ModuleType::V1190A (V1190A_v, 0x0000000400A60000LL, 128, 4);
const Caen1290Module::ModuleType Caen1290Module::ModuleType::V1190B (V1190B_v, 0x0000000400A60001LL, 64, 2);

Caen1290Module::Caen1290Module(int id, QString name, ModuleType type)
: BaseDAqModule (id, name)
, conf_ (new Caen1290Config)
, type_ (type)
{
    setChannels ();
    createOutputPlugin ();
    setUI (new Caen1290UI (this));
}

Caen1290Module::~Caen1290Module () {
    delete conf_;
}

void Caen1290Module::setChannels () {
    for (int i = 0; i < type_.channels (); ++i)
        getChannels ()->append (new ScopeChannel (this, QString ("out %1").arg (i), ScopeCommon::eventBuffer, 1, 1));
    getChannels ()->append (new ScopeChannel (this, "Poll Trigger", ScopeCommon::trigger, 1, 1));
}

void Caen1290Module::createOutputPlugin () {
    out_ = new DemuxCaen1290Plugin (-getId (), getName () + " output", type_.channels (), type_.hires ());
    output = out_;
}

ThreadBuffer<uint32_t> *Caen1290Module::getBuffer () {
    return NULL;
}

void Caen1290Module::setBaseAddress (uint32_t baddr) {
    conf_->base_addr = baddr;
}

uint32_t Caen1290Module::getBaseAddress () const {
    return conf_->base_addr;
}

void Caen1290Module::prepareForNextAcquisition () {
}

int Caen1290Module::reset () {
    int ret = 0;
    if (!iface)
        return -1;

    if ((ret = iface->writeA32D16 (conf_->base_addr + CAEN1290_RESET, 1)))
        std::cout << "Error " << ret << " at CAEN1290_RESET" << std::endl;
    return ret;
}

int Caen1290Module::softClear () {
    int ret = 0;
    if (!iface)
        return -1;

    if ((ret = iface->writeA32D16 (conf_->base_addr + CAEN1290_SCLR, 1)))
        std::cout << "Error " << ret << " at CAEN1290_SCLR" << std::endl;
    return ret;
}

bool Caen1290Module::dataReady () {
    int ret;
    uint16_t status;

    if ((ret = iface->readA32D16(conf_->base_addr + CAEN1290_STATUS, &status))) {
        std::cout << "Error " << ret << " at CAEN1290_STATUS" << std::endl;
        return false;
    }

    return (status & (1 << CAEN1290_STA_DREADY)) != 0;
}

int Caen1290Module::acquire () {
    std::vector<uint32_t> data;
    const uint32_t len = 0xFFC;
    uint32_t buf [len];
    uint32_t got = 0;
    int ret = 0;

    while (true) {
        got = 0;
        // only fatal if no data has been read (at least for sis modules)
        if ((ret = iface->readA32MBLT64 (conf_->base_addr + CAEN1290_MEB, &buf [0], len, &got)) && got == 0) {
            std::cout << "Error " << ret << " at MBLT from CAEN1290_MEB" << std::endl;
            return ret;
        }

        for (uint32_t i = 0; i < got; ++i) {
            if (((buf [i] >> 27) & 0x1F) == 0x18) { // filler detected
                got = i;
                break;
            }
        }

        data.assign (buf, buf + got);

        // check whether we are in single event mode and the demux plugin just completed an event
        bool go_on = out_->processData (data, RunManager::ref().isSingleEventMode());
        if (!go_on) {
            softClear (); // discard the rest of the events
            return 0;
        }

        if (len != got)
            break;
    }

    return ret;
}

int Caen1290Module::configure () {
    uint32_t baddr = conf_->base_addr;
    int ret;
    int i;

    if (!iface)
        return -1;

    reset ();

    if (!isCorrectModule ()) {
        printf ("Incorrect module type!\n");
        return -2;
    }

    if ((ret = iface->writeA32D16 (baddr + CAEN1290_CONTROL,
            (conf_->berr_enable ? (1 << CAEN1290_CTL_BERREN) : 0) |
            (conf_->term ? (1 << CAEN1290_CTL_TERM) : 0) |
            (conf_->term_sw ? (1 << CAEN1290_CTL_TERM_SW) : 0) |
            (conf_->emptyev ? (1 << CAEN1290_CTL_EMPTYEV) : 0) |
            (conf_->align64 ? (1 << CAEN1290_CTL_ALIGN64) : 0) |
            (conf_->comp_enable ? (1 << CAEN1290_CTL_COMPEN) : 0) |
            (conf_->test_fifo_enable ? (1 << CAEN1290_CTL_TESTFIFOEN) : 0) |
            (conf_->read_comp_sram ? (1 << CAEN1290_CTL_READCOMPEN) : 0 ) |
            (conf_->ev_fifo_enable ? (1 << CAEN1290_CTL_EVFIFOEN) : 0) |
            (conf_->extrigtimetag_enable ? (1 << CAEN1290_CTL_EXTRGTIMETAG) : 0))))
        std::cout << "Error " << ret << " at CAEN1290_CONTROL" << std::endl;
    
    if ((ret = iface->writeA32D16 (baddr + CAEN1290_AMFULL_LVL, conf_->almost_full_lvl)))
        std::cout << "Error " << ret << " at CAEN1290_AMFULL_LVL" << std::endl;

    if ((ret = iface->writeA32D16(baddr + CAEN1290_OUTP_CTRL, conf_->out_prog)))
        std::cout << "Error " << ret << " at CAEN1290_OUTP_CTRL" << std::endl;

    if ((ret = iface->writeA32D16(baddr + CAEN1290_BLT_EVNR, conf_->nof_aln_events)))
        std::cout << "Error " << ret << " at CAEN1290_BLT_EVNR" << std::endl;

    // configure the TDCs

    if ((ret = writeToMC (conf_->trig_match_mode ? MC1290_TRG_MATCH : MC1290_CONT_STOR)))
        std::cout << "Error " << ret << " writing acq mode" << std::endl;

    if ((ret = writeToMC (MC1290_SET_WIN_WIDTH)) || (ret = writeToMC (conf_->win_width & 0x0FFF)))
        std::cout << "Error " << ret << " writing window width" << std::endl;

    if ((ret = writeToMC (MC1290_SET_WIN_OFFS)) || (ret = writeToMC (conf_->win_offset & 0x0FFF)))
        std::cout << "Error " << ret << " writing window offset" << std::endl;

    if ((ret = writeToMC (MC1290_SET_SW_MARGIN)) || (ret = writeToMC (conf_->sw_margin & 0x0FFF)))
        std::cout << "Error " << ret << " writing search margin" << std::endl;

    if ((ret = writeToMC (MC1290_SET_REJ_MARGIN)) || (ret = writeToMC (conf_->rej_margin & 0x0FFF)))
        std::cout << "Error " << ret << " writing reject margin" << std::endl;

    if ((ret = writeToMC (conf_->sub_trg ? MC1290_EN_SUB_TRG : MC1290_DIS_SUB_TRG)))
        std::cout << "Error " << ret << " writing trigger subtract mode" << std::endl;

    if ((ret = writeToMC (MC1290_SET_DETECTION)) || (ret = writeToMC (conf_->edge_detect_mode & 0x03)))
        std::cout << "Error " << ret << " writing detection mode" << std::endl;

    if ((ret = writeToMC (MC1290_SET_TR_LEAD_LSB)) || (ret = writeToMC (conf_->edge_lsb_val & 0x03)))
        std::cout << "Error " << ret << " writing time resolution" << std::endl;

    if ((ret = writeToMC (MC1290_DIS_HEAD_TRAILER)))
        std::cout << "Error " << ret << " disabling tdc header & trailer" << std::endl;

    if ((ret = writeToMC (MC1290_SET_PAIR_RES)) || (ret = writeToMC (
            (conf_->pair_lead_time & 0x07) | ((conf_->pair_width_res & 0x0F) << 8))))
        std::cout << "Error " << ret << " writing pair mode settings" << std::endl;

    if ((ret = writeToMC (MC1290_SET_DEAD_TIME)) || (ret = writeToMC (conf_->hit_dead_time & 0x03)))
        std::cout << "Error " << ret << " writing dead time" << std::endl;

    if ((ret = writeToMC (MC1290_WRITE_EN_PATTERN)))
        std::cout << "Error " << ret << " writing channel enable pattern" << std::endl;
    for (int chans = type_.channels(), i = 0; chans > 0; chans -= 16, ++i) {
        if ((ret = writeToMC (conf_->enable_chan_bitmap [i]))) {
            std::cout << "Error " << ret << " writing channel enable pattern" << std::endl;
            break;
        }
    }

    if ((ret = writeToMC (MC1290_SET_GLOB_OFFS))
        || (ret = writeToMC (conf_->global_offset_coarse & 0x07FF))
        || (ret = writeToMC (conf_->global_offset_fine & 0x1F)))
        std::cout << "Error " << ret << " writing global offset" << std::endl;

    /* Disabled for now as it takes way too long
    for (i = 0; i < type_.channels (); ++i)
        if ((ret = writeToMC (MC1290_SET_ADJUST_CH | (i & 0xFF)))
            || (ret = writeToMC (conf_->channel_adj [i] & 0xFF)))
            std::cout << "Error " << ret << " writing channel adjust for channel " << i << std::endl;
    */
    for (i = 0; i < type_.tdcs (); ++i)
        if ((ret = writeToMC (MC1290_SET_RC_ADJUST | (i & 0x0F)))
            || (ret = writeToMC (conf_->rc_adj [i])))
            std::cout << "Error " << ret << " writing rc adjust for tdc " << i << std::endl;

    softClear ();

    return ret;
}

int Caen1290Module::writeToMC (uint16_t data) {
    int ret;
    uint16_t stat;
    while (!(ret = iface->readA32D16 (conf_->base_addr + CAEN1290_MICRO_HS, &stat))) {
        if (!(stat & (1 << CAEN1290_MCH_WRITE_OK)))
            continue;
        ret = iface->writeA32D16 (conf_->base_addr + CAEN1290_MICRO, data);
        break;
    }
    return ret;
}

bool Caen1290Module::isCorrectModule () {
    int ret;
    const uint64_t CaenOui = 0x0000004000E6LL;
    uint16_t oui0, oui1, oui2;
    uint16_t board0, board1, board2, vers;

    if ((ret = iface->readA32D16 (conf_->base_addr + 0x402C, &oui0)) ||
        (ret = iface->readA32D16 (conf_->base_addr + 0x4028, &oui1)) ||
        (ret = iface->readA32D16 (conf_->base_addr + 0x4024, &oui2)) ||
        (ret = iface->readA32D16 (conf_->base_addr + 0x403C, &board0)) ||
        (ret = iface->readA32D16 (conf_->base_addr + 0x4038, &board1)) ||
        (ret = iface->readA32D16 (conf_->base_addr + 0x4034, &board2)) ||
        (ret = iface->readA32D16 (conf_->base_addr + 0x4030, &vers)))
    {
        std::cout << "Error " << ret << " while reading Configuration ROM" << std::endl;
        return false;
    }

    uint64_t oui = static_cast<uint64_t> (oui0) |
                  (static_cast<uint64_t> (oui1) << 16) |
                  (static_cast<uint64_t> (oui2) << 32);
    uint64_t id  = static_cast<uint64_t> (vers) |
                  (static_cast<uint64_t> (board0) << 16) |
                  (static_cast<uint64_t> (board1) << 32) |
                  (static_cast<uint64_t> (board2) << 48);

    return type_.boardId () == id && oui == CaenOui;
}

typedef ConfMap::confmap_t<Caen1290Config> confmap_t;
static const confmap_t confmap [] = {
    confmap_t ("base_addr", &Caen1290Config::base_addr),
    confmap_t ("berr_enable", &Caen1290Config::berr_enable),
    confmap_t ("term", &Caen1290Config::term),
    confmap_t ("term_sw", &Caen1290Config::term_sw),
    confmap_t ("emptyev", &Caen1290Config::emptyev),
    confmap_t ("align64", &Caen1290Config::align64),
    confmap_t ("comp_enable", &Caen1290Config::comp_enable),
    confmap_t ("test_fifo_enable", &Caen1290Config::test_fifo_enable),
    confmap_t ("read_comp_sram", &Caen1290Config::read_comp_sram),
    confmap_t ("ev_fifo_enable", &Caen1290Config::ev_fifo_enable),
    confmap_t ("extrigtimetag_enable", &Caen1290Config::extrigtimetag_enable),
    confmap_t ("almost_full_lvl", &Caen1290Config::almost_full_lvl),
    confmap_t ("out_prog", &Caen1290Config::out_prog),
    confmap_t ("nof_aln_events", &Caen1290Config::nof_aln_events),

    confmap_t ("trig_match_mode", &Caen1290Config::trig_match_mode),
    confmap_t ("win_width", &Caen1290Config::win_width),
    confmap_t ("win_offset", &Caen1290Config::win_offset),
    confmap_t ("sw_margin", &Caen1290Config::sw_margin),
    confmap_t ("rej_margin", &Caen1290Config::rej_margin),
    confmap_t ("sub_trg", &Caen1290Config::sub_trg),

    confmap_t ("edge_detect_mode", &Caen1290Config::edge_detect_mode),
    confmap_t ("edge_lsb_val", &Caen1290Config::edge_lsb_val),
    confmap_t ("pair_lead_time", &Caen1290Config::pair_lead_time),
    confmap_t ("pair_width_res", &Caen1290Config::pair_width_res),
    confmap_t ("hit_dead_time", &Caen1290Config::hit_dead_time),
    confmap_t ("global_offset_coarse", &Caen1290Config::global_offset_coarse),
    confmap_t ("global_offset_fine", &Caen1290Config::global_offset_fine),
};

void Caen1290Module::applySettings (QSettings *settings) {
    std::cout << "Applying settings for " << getName ().toStdString () << "... ";
    settings->beginGroup (getName ());
    ConfMap::apply (settings, conf_, confmap);

    // channel adjusts and channel bitmaps are stored packed as uint32
    for (int i = 0; i < 32; ++i) {
        QString key = QString ("channel_adj%1").arg (i);
        if (settings->contains (key)) {
            uint32_t val = settings->value (key).toUInt ();
            conf_->channel_adj [4*i] = (val & 0xFF);
            conf_->channel_adj [4*i+1] = ((val >> 8) & 0xFF);
            conf_->channel_adj [4*i+2] = ((val >> 16)& 0xFF);
            conf_->channel_adj [4*i+3] = ((val >> 24)& 0xFF);
        }
    }
    for (int i = 0; i < 4; ++i) {
        QString key = QString ("enable_chan_bitmap%1").arg (i);
        if (settings->contains (key)) {
            uint32_t val = settings->value (key).toUInt ();
            conf_->enable_chan_bitmap [2*i]   = (val & 0xFFFF);
            conf_->enable_chan_bitmap [2*i+1] = ((val >> 16) & 0xFFFF);
        }
    }
    for (int i = 0; i < 4; ++i) {
        QString key = QString ("rc_adj%1").arg (i);
        if (settings->contains (key))
            conf_->rc_adj [i] = settings->value (key).toUInt ();
    }
    settings->endGroup ();
    std::cout << "done" << std::endl;

    getUI ()->applySettings ();
}

void Caen1290Module::saveSettings (QSettings *settings) {
    std::cout << "Saving settings for " << getName ().toStdString () << "... ";
    settings->beginGroup (getName ());
    ConfMap::save (settings, conf_, confmap);
    for (int i = 0; i < 32; ++i) {
        QString key = QString ("channel_adj%1").arg (i);
        settings->setValue (key, static_cast<uint32_t> (
                conf_->channel_adj [4*i] |
                (conf_->channel_adj [4*i+1] << 8) |
                (conf_->channel_adj [4*i+2] << 16) |
                (conf_->channel_adj [4*i+3] << 24)));
    }
    for (int i = 0; i < 4; ++i) {
        QString key = QString ("enable_chan_bitmap%1").arg (i);
        settings->setValue (key, static_cast<uint32_t> (
            conf_->enable_chan_bitmap [2*i] |
            (conf_->enable_chan_bitmap [2*i+1] << 16)));
    }
    for (int i = 0; i < 4; ++i) {
        QString key = QString ("rc_adj%1").arg (i);
        settings->setValue (key, QString ("0x%1").arg (conf_->rc_adj [i]));
    }
    settings->endGroup ();
    std::cout << "done" << std::endl;
}

/*!
\page caen1290mod Caen V1x90 Multi-Hit TDC
<b>Module names:</b> \c caen1190a, \c caen1190b, \c caen1290a, \c caen1290n

\section desc Module Description
The Caen V1x90 family is a family of multi-hit TDCs. The V1290A/N features 32/16 high-resolution channels, the 1190A/B has 128/64 channels with a slightly lower resolution.

\section cpanel Configuration Panel
The configuration features multiple tabs through which the device may be configured.
\subsection general General
The General tab controls some basic device functionality.
\li <b>BERR</b> controls whether the end of the filled part of the event buffer is signalled by a bus error thus ending the transfer from the device to GECKO.
The readout mechanism relies on te setting of this switch and it will be removed in a future version. <b>DO NOT CHANGE!</b>
\li <b>ALIGN64</b> causes the length of an event to be a multiple of 64 bits.
Use this switch if the VME interface cuts off the last 32-bit word from a block transfer.
\li <b>Software TERM</b> controls whether the termination of the front panel connector is controlled via software or via the on-board dip-switches.
See the Caen V1x90 manual for details.
\li <b>TERM</b> terminates the front panel connector.
Only works when Software TERM is checked.
\li <b>Compensation</b> enables the compensation of the TDC's integral non-linearity.
\li <b>Event FIFO</b> controls the filling of the event FIFO. The event FIFO is currently unused, so leave this option unchecked.
\li <b>Ext. Time Tag</b> controls whether a trigger time tag is generated, denoting the trigger time relative to some aritrary point. This tag is currently unused, so leave this option unchecked.

\subsection acqmode Acquisition Mode
This tab controls how the multi-hit TDC gets data from its inputs.
The mode is either \em continuous, i.e. every time an event occurs a timestamp is written to the output buffer, or \em triggered, i.e. a global trigger causes the acquisition of events in a certain window before the trigger occurred.
The remaining fields control the layout of the acquisition window.
Please consult the Caen V1x90 manual for further information on how to set these values.
\attention The continuous mode is not tested and may not work (as expected).

\subsection edgedet Edge Detection
This tab defines the notion of an event for the device. The device may trigger on leading, trailing or both edges.
The time resolution of and the dead time after the trigger events is adjustable.
Please consult the Caen V1x90 manual for further information.

\subsection adj Adjustments
This tab allows for fine tuning the acquisition process.

The first part allows to enable or disable any channel. Disabled channels will never trigger events.
Through the second panel the user may assign individual offsets to each channel. This functionality is currently disabled because it slows down the configuration process very much.
A global offset may still be set using the two spinboxes at the bottom of the panel.

The last panel can be used to fine-tune the RC circuitry used by the V1290 for high-resolution timing.
Please consult the Caen V1x90 manual for further information.

\section outs Outputs
The module contains an output for each channel, outputting a vector of hit timestamps for each event.
*/

