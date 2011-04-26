#include "caen820dmx.h"
#include "eventbuffer.h"

#include <iostream>

Caen820Demux::Caen820Demux (const QVector<EventSlot*> &evslots)
: enabledch_ (0xFFFFFFFFU)
, shortfmt_ (false)
, hdrenabled_ (false)
, evslots_ (evslots)
{
}

bool Caen820Demux::processData (Event *ev, const uint32_t *data, int len) {
    if (hdrenabled_) { // no use for the header yet
        ++data; --len;
    }

    for (int i = 0; i < 32; ++i) {
        if (!((enabledch_ & (1 << i)) && len)) {
            if (!len && (enabledch_ & (1 << i)))
                std::cout << "DemuxCaen820: Warning! not enough data to serve channel " << i << std::endl;

//            if (outputs->at (i)->hasOtherSide ())
//                outputs->at (i)->setData (new std::vector<uint32_t> ());
            continue;
        }

        ev->put (evslots_.at (i), QVariant::fromValue (QVector<uint32_t> () << *data));
        ++data; --len;
    }

    return true;
}
