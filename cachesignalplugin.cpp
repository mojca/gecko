#include "baseplugin.h"
#include "cachesignalplugin.h"
#include "pluginmanager.h"

static PluginRegistrar registrar ("cachesignalplugin", CacheSignalPlugin::create, AbstractPlugin::GroupCache);

CacheSignalPlugin::CacheSignalPlugin(int _id, QString _name)
        : BasePlugin(_id, _name),
        BaseCachePlugin(_id, _name)
{
    lastRead.setTime_t(0);

    createSettings(settingsLayout);

    plot->addChannel(0,tr("signal"),std::vector<double>(1,0),
                     QColor(Qt::red),Channel::line,1);
    plot->addChannel(1,tr("signalFromDisk"),std::vector<double>(1,0),
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
                if(signal.size() != 0) plot->getChannelById(1)->setData(signal);
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
            signal.resize(idata.size(),0);
            scheduleReset = false;
            plot->resetBoundaries(0);
        }

        if(signal.size() != idata.size()) signal.resize(idata.size(),0);

        curData.assign(idata.begin(), idata.end());

        if(conf.inputWeight != 1.0) dsp.fast_scale(curData,conf.inputWeight);
        if(conf.useInputWeight == true) dsp.fast_add(signal,curData);
        else signal = curData;

        if(conf.normalize)
        {
            double min = dsp.min(signal)[AMP];
            dsp.fast_addC(signal,-min);
            dsp.fast_scale(signal,1.0/(dsp.max(signal)[AMP]));
        }
        if(signal.size() != 0) plot->getChannelById(0)->setData(signal);
    }

    QVector<double> out = QVector<double>::fromStdVector (signal);
    outputs->at(0)->setData(QVariant::fromValue (out));
    outputs->at(1)->setData(QVariant::fromValue (out));
}

