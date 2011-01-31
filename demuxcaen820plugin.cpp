#include "demuxcaen820plugin.h"
#include "pluginconnectorthreadbuffered.h"
#include "pluginmanager.h"

static PluginRegistrar registrar ("demuxcaen820", AbstractPlugin::GroupDemux);

DemuxCaen820Plugin::DemuxCaen820Plugin(int id, QString name)
: BasePlugin (id, name)
, enabledch_ (0xFFFFFFFFU)
, shortfmt_ (false)
, hdrenabled_ (false)
{
    for (int i = 0; i < 32; ++i)
        addConnector (new PluginConnectorThreadBuffered (this, QString ("out %1").arg (i), 1, 10, (id << 16) | i));
}

bool DemuxCaen820Plugin::processData (const uint32_t *data, int len) {
    if (hdrenabled_) { // no use for the header yet
        ++data; --len;
    }

    bool isfirst = true;

    for (int i = 0; i < 32; ++i) {
        if (!((enabledch_ & (1 << i)) && len)) {
            if (!len && (enabledch_ & (1 << i)))
                std::cout << "DemuxCaen820: Warning! not enough data to serve channel " << i << std::endl;

//            if (outputs->at (i)->hasOtherSide ())
//                outputs->at (i)->setData (new std::vector<uint32_t> ());
            continue;
        }

        if (outputs->at (i)->hasOtherSide ()) {
            if (isfirst) { // if the first connector is full, we probably overtook the plugin thread by far, so drop additional data
                isfirst = false;
                if (!dynamic_cast<PluginConnectorThreadBuffered*> (outputs->at (i))->elementsFree())
                    return false;
            }

            outputs->at (i)->setData (new std::vector<uint32_t> (1, *data));
        }
        ++data; --len;
    }

    return true;
}
