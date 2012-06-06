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

#ifndef FILEREADERPLUGIN_H
#define FILEREADERPLUGIN_H

#include <algorithm>
#include <map>
#include <cstdio>
#include <stdint.h>
#include "mesytec_madc_32_v2.h"
#include "filereader.h"

#include <QVector>
#include <QFile>

class Event;
class EventSlot;
class AbstractModule;
template <typename T> class QVector;

class FileReaderDemux
{
private:
    bool inEvent;
    int cnt;

    bool enable_raw_output;
    bool enable_per_channel_output;

    uint8_t nofChannels;
    uint8_t nofChannelsInEvent;
    uint8_t nofBits;
    uint8_t crateNumber;
    uint32_t eventCounter;
    uint8_t id;

    std::map<uint8_t, uint16_t> chData;
    QVector<uint32_t> rawData;
    uint32_t rawCnt;
    QVector<bool> enable_ch;

    const QVector<EventSlot*>& evslots;
    const AbstractModule *owner;

    uint32_t* it;

    void startNewEvent();
    void continueEvent();
    bool finishEvent(Event *ev);
    void printHeader();
    void printEob();

    madc32_header_t header;
    madc32_end_of_event_t trailer;

public:
    FileReaderDemux(const QVector<EventSlot*>& _evslots, const AbstractModule* op,
                 uint chans = FILEREADER_NUM_CHANNELS,
                 uint bits = FILEREADER_NUM_BITS);
//    FileReaderDemux(const QVector<EventSlot*>& _evslots, const AbstractModule* op);

//    bool processData (Event *ev, uint32_t* data, uint32_t len, bool singleev);
    bool processData (Event *ev, QFile *file, uint32_t len, bool singleev);
    void runStartingEvent();
};

#endif // FILEREADERPLUGIN_H
