#include "baseplugin.h"
#include "cachesignalplugin.h"
#include "pluginmanager.h"
#include "samqvector.h"

static PluginRegistrar registrar ("cachesignalplugin", CacheSignalPlugin::create, AbstractPlugin::GroupCache);

CacheSignalPlugin::CacheSignalPlugin(int _id, QString _name)
        : BasePlugin(_id, _name),
        BaseCachePlugin(_id, _name)
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
        if(signal.size() != 0) plot->getChannelById(0)->setData(signal);
    }

    outputs->at(0)->setData(QVariant::fromValue (signal));
    outputs->at(1)->setData(QVariant::fromValue (signal));
}

