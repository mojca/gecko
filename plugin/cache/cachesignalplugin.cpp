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

#include "baseplugin.h"
#include "cachesignalplugin.h"
#include "pluginmanager.h"
#include "samqvector.h"

static PluginRegistrar registrar ("cachesignalplugin", CacheSignalPlugin::create, AbstractPlugin::GroupCache);

CacheSignalPlugin::CacheSignalPlugin(int _id, QString _name)
        : BaseCachePlugin(_id, _name)
{
    lastRead.setTime_t(0);

    createSettings(settingsLayout);

    plot->addChannel(0,tr("signal"),QVector<double>(1,0),
                     QColor(Qt::red),Channel::line,1);
    plot->addChannel(1,tr("signalFromDisk"),QVector<double>(1,0),
                     QColor(Qt::blue),Channel::line,1);

    halfSecondTimer = new QTimer();
    halfSecondTimer->start(msecsToTimeout);
    connect(halfSecondTimer,SIGNAL(timeout()),plot,SLOT(update()));

    std::cout << "Instantiated CacheSignalPlugin" << std::endl;
}

void CacheSignalPlugin::userProcess()
{
    //std::cout << "CacheSignalPlugin userProcess" << std::endl;
    QVector<double> idata = inputs->first()->getData().value< QVector<double> > ();

    SamDSP dsp;

    if(conf.useStoredData)
    {
        QFileInfo info(conf.fileName);
        if(info.lastModified() > lastRead)
        {
            if(info.isReadable())
            {
                dsp.vectorFromFile(signal,conf.fileName.toStdString());
                if(!signal.empty ()) plot->getChannelById(1)->setData(signal);
            }
            else
            {
                std::cout << "File " << conf.fileName.toStdString() << " is not readable." << std::endl;
                return;
            }
            if(conf.normalize)
            {
                double min = dsp.min(signal)[AMP];
                dsp.fast_addC(signal,-min);
                dsp.fast_scale(signal,1.0/(dsp.max(signal)[AMP]));
            }

            lastRead = QDateTime::currentDateTime();
        }

    }
    else
    {

        if(scheduleReset)
        {
            signal.clear();
            signal.fill (0, idata.size());
            scheduleReset = false;
            plot->resetBoundaries(0);
        }

        if(signal.size() != idata.size()) signal.resize(idata.size());

        if(conf.inputWeight != 1.0) dsp.fast_scale(idata,conf.inputWeight);
        if(conf.useInputWeight == true) dsp.fast_add(signal,idata);
        else signal = idata;

        if(conf.normalize)
        {
            double min = dsp.min(signal)[AMP];
            dsp.fast_addC(signal,-min);
            dsp.fast_scale(signal,1.0/(dsp.max(signal)[AMP]));
        }
        if(signal.size() != 0) {
            QWriteLocker wr (plot->getChanLock ());
            plot->getChannelById(0)->setData(signal);
        }
    }

    outputs->at(0)->setData(QVariant::fromValue (signal));
    outputs->at(1)->setData(QVariant::fromValue (signal));
}

void CacheSignalPlugin::runStartingEvent () {
    // reset all timers and the signal before starting anew
    halfSecondTimer->stop();
    scheduleReset = true;
    halfSecondTimer->start(msecsToTimeout);
}

/*!
\page cachesignalplg Signal Cache Plugin
\li <b>Plugin names:</b> \c cachesignalplugin
\li <b>Group:</b> Cache

\section pdesc Plugin Description
The signal cache plugin stores the signal fed to its input connector.
It can compute an (exponential) average over all signals or normalise it to its maximum.
The result is passed to both output connectors.

The cache may also load a signal from disk and make it available for further processing.

A live preview of the stored data is available.

\section attrs Attributes
None

\section conf Configuration
\li <b>Input weight</b>: The weight with which new data is added
\li <b>Use Input Weight</b>: Enable weighting the input data
\li <b>Normalize</b>: Normalizes the signal to its maximum value
\li <b>Preview</b>: Shows a live plot of the signal cache
\li <b>Reset</b>: Manually reset the signal cache
\li <b>Update Speed</b>: Interval between updates of the histogram plot and counter
\li <b>Use File</b>: Enable use of the given file instead of the input data

\section inputs Input Connectors
\c in \c &lt;double>: Input for the data to be cached

\section outputs Output Connectors
\c fileOut, out \c &lt;double>: Contains the current cache contents
*/
