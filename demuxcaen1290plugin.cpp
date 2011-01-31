#include "demuxcaen1290plugin.h"
#include "pluginconnectorthreadbuffered.h"
#include "pluginmanager.h"

#include <cstdio>

static PluginRegistrar registrar ("demuxcaen1290", AbstractPlugin::GroupDemux);

DemuxCaen1290Plugin::DemuxCaen1290Plugin (int id, const QString &name, int channels, bool hires)
: BasePlugin (id, name)
, status_ (Start)
, chans_ (channels)
, measurementBits_ (hires ? 21 : 19)
, channelBits_ (hires ? 5 : 7)
{
    evbuf_.resize (chans_);
    for (int i = 0; i < chans_; ++i) {
        int cid = (id << 16) + i;
        addConnector (new PluginConnectorThreadBuffered (
                this, QString ("out %1").arg (i), 1, 32, cid
                ));

    }
}

bool DemuxCaen1290Plugin::processData (const std::vector<uint32_t> &data, bool singleev) {
    for (std::vector<uint32_t>::const_iterator i = data.begin (); i != data.end (); ++i) {
        uint32_t tag = (*i >> 27) & 0x1F;

        switch (tag) {
        case GlobalHeader:
            if (status_ != Start) {
                printf ("DemuxCaen1290: Header while event processing\n");
                endEvent ();
            }
            startEvent (*i);
            break;
        case GlobalTrailer:
            if (status_ != Measurements) {
                std::cout << "DemuxCaen1290: Trailer while not processing" << std::endl;
            } else {
                bool go_on = endEvent ();
                if (*i & (1 << 25))
                    std::cout << "MHTDC Overflow!" << std::endl;

                if (singleev || ! go_on)
                    return false;
            }
            break;
        case TDCMeasurement:
            if (status_ != Measurements)
                std::cout << "DemuxCaen1290: Data outside of event. Ignoring" << std::endl;
            else
                processEvent (*i);
            break;
        case TDCHeader:
        case TDCTrailer:
        case Filler:
        case TrigTimeTag:
            break;
        case TDCError:
            printf ("DemuxCaen1290: Got TDC error %#04x from TDC %d\n", (*i & 0x3FFF), (*i >> 24) & 0x3);
            break;
        default:
            printf ("DemuxCaen1290: Unknown tag: %#02x\n", tag);
        }
    }
    return true;
}

void DemuxCaen1290Plugin::startEvent (uint32_t info) {
    Q_UNUSED (info);
    status_ = Measurements;
    for (int i = 0; i < chans_; ++i)
        evbuf_ [i].clear ();
}

void DemuxCaen1290Plugin::processEvent (uint32_t ev) {
    uint16_t channel = (ev >> measurementBits_) & ((1 << channelBits_) - 1);
    uint32_t value = ev & ((1 << measurementBits_) - 1);

    evbuf_ [channel].push_back (value);
}

bool DemuxCaen1290Plugin::endEvent () {
    bool isfirst = true;
    for (int i = 0; i < chans_; ++i) {
        if (outputs->at (i)->hasOtherSide ()) {
            if (isfirst) {
                isfirst = false;
                if (!dynamic_cast<PluginConnectorThreadBuffered*> (outputs->at (0))->elementsFree())
                    return false;
            }

            outputs->at (i)->setData (new std::vector<uint32_t> (evbuf_ [i]));
        }
    }
    status_ = Start;

    return true;
}
