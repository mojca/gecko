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

#include <limits>
#include <QComboBox>
#include "baseplugin.h"
#include "cachehistogramplugin.h"
#include "pluginmanager.h"
#include "runmanager.h"
#include "samqvector.h"

#include <QGridLayout>
#include <QLabel>

static PluginRegistrar registrar ("cachehistogramplugin", CacheHistogramPlugin::create, AbstractPlugin::GroupCache);

CacheHistogramPlugin::CacheHistogramPlugin(int _id, QString _name)
    : BasePlugin(_id, _name),
    BaseCachePlugin(_id, _name),
    binWidth(1),
    writeToFile(false),
    fileCount(0)
{
    createSettings(settingsLayout);

    plot->addChannel(0,tr("histogram"),QVector<double>(1,0),
                     QColor(Qt::red),Channel::steps,1);

    halfSecondTimer = new QTimer();
    halfSecondTimer->start(msecsToTimeout);
    connect(halfSecondTimer,SIGNAL(timeout()),this,SLOT(updateVisuals()));

    writeToFileTimer = new QTimer();
    writeToFileTimer->start(conf.autosaveInt*1000);         // write To File
    connect(writeToFileTimer,SIGNAL(timeout()),this,SLOT(scheduleWriteToFile()));

    resetTimer = new QTimer();
    resetTimer->start(conf.autoresetInt*60*1000);           // reset histogram
    connect(resetTimer,SIGNAL(timeout()),this,SLOT(scheduleResetHistogram()));

    std::cout << "Instantiated CacheSignalPlugin" << std::endl;
}

void CacheHistogramPlugin::recalculateBinWidth()
{
    binWidth = (conf.xmax-conf.xmin)/((double)conf.nofBins);
}

void CacheHistogramPlugin::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
        set = "inputWeight"; if(settings->contains(set)) conf.inputWeight = settings->value(set).toDouble();
        set = "normalize";   if(settings->contains(set)) conf.normalize = settings->value(set).toBool();
        set = "autosave";    if(settings->contains(set)) conf.autosave = settings->value(set).toBool();
        set = "autoreset";   if(settings->contains(set)) conf.autoreset = settings->value(set).toBool();
        set = "nofBins"; if(settings->contains(set)) conf.nofBins = settings->value(set).toInt();
        set = "xmax";    if(settings->contains(set)) conf.xmax = settings->value(set).toDouble();
        set = "xmin";    if(settings->contains(set)) conf.xmin = settings->value(set).toDouble();
        set = "ymax";    if(settings->contains(set)) conf.ymax = settings->value(set).toInt();
        set = "plotGeometry"; if(settings->contains(set)) plot->restoreGeometry(settings->value(set).toByteArray());
        set = "autoresetInt"; if(settings->contains(set)) conf.autoresetInt = settings->value(set).toInt();
        set = "autosaveInt";  if(settings->contains(set)) conf.autosaveInt = settings->value(set).toInt();
        set = "plotVisible"; if(settings->contains(set)) if(settings->value(set).toBool()) plot->show();
    settings->endGroup();

    inputWeightSpinner->setValue(conf.inputWeight);
    xmaxSpinner->setValue(conf.xmax);
    xminSpinner->setValue(conf.xmin);
    ymaxSpinner->setValue(conf.ymax);
    nofBinsBox->setCurrentIndex(nofBinsBox->findData(conf.nofBins,Qt::UserRole));
    normalizeCheck->setChecked(conf.normalize);
    autoresetCheck->setChecked(conf.autoreset);
    autosaveCheck->setChecked(conf.autosave);
    autoresetSpinner->setValue(conf.autoresetInt);
    autosaveSpinner->setValue(conf.autosaveInt);
}

void CacheHistogramPlugin::saveSettings(QSettings* settings)
{
    if(settings == NULL)
    {
        std::cout << getName().toStdString() << ": no settings file" << std::endl;
        return;
    }
    else
    {
        std::cout << getName().toStdString() << " saving settings...";
        settings->beginGroup(getName());
            settings->setValue("inputWeight",conf.inputWeight);
            settings->setValue("normalize",conf.normalize);
            settings->setValue("nofBins",conf.nofBins);
            settings->setValue("xmax",conf.xmax);
            settings->setValue("xmin",conf.xmin);
            settings->setValue("ymax",conf.ymax);
            settings->setValue("autoreset",conf.autoreset);
            settings->setValue("autoresetInt",conf.autoresetInt);
            settings->setValue("autosave",conf.autosave);
            settings->setValue("autosaveInt",conf.autosaveInt);
            settings->setValue("plotGeometry",plot->saveGeometry());
            plot->isVisible() ? settings->setValue("plotVisible",true) : settings->setValue("plotVisible",false);
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void CacheHistogramPlugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        previewButton = new QPushButton(tr("Show..."));
        resetButton = new QPushButton(tr("Reset"));

        normalizeCheck = new QCheckBox(tr("Normalize"));
        normalizeCheck->setChecked(conf.normalize);

        plot = new plot2d(0,QSize(320,240),0);
        plot->setWindowTitle(getName());

        QLabel* updateSpeedLabel = new QLabel(tr("Update Speed (ms)"));
        QLabel* inputWeightLabel = new QLabel(tr("Input Weight"));
        QLabel* xminLabel = new QLabel(tr("From"));
        QLabel* xmaxLabel = new QLabel(tr("To"));
        QLabel* ymaxLabel = new QLabel(tr("Max Height"));
        QLabel* nofBinsLabel = new QLabel(tr("Number of Bins"));
        QLabel* autosaveIntLabel = new QLabel(tr("Interval (s)"));
        QLabel* autoresetIntLabel = new QLabel(tr("Interval (m)"));

        autosaveCheck = new QCheckBox(tr("Auto save"));
        autosaveCheck->setChecked(conf.autosave);
        autoresetCheck = new QCheckBox(tr("Auto reset"));
        autoresetCheck->setChecked(conf.autoreset);

        // In seconds
        autosaveSpinner = new QSpinBox();
        autosaveSpinner->setMinimum(1);
        autosaveSpinner->setMaximum(3600);
        autosaveSpinner->setSingleStep(1);
        autosaveSpinner->setValue(conf.autosaveInt);

        // In minutes
        autoresetSpinner = new QSpinBox();
        autoresetSpinner->setMinimum(1);
        autoresetSpinner->setMaximum(3600);
        autoresetSpinner->setSingleStep(1);
        autoresetSpinner->setValue(conf.autoresetInt);

        // In milliseconds
        updateSpeedSpinner = new QSpinBox();
        updateSpeedSpinner->setMinimum(100);
        updateSpeedSpinner->setMaximum(5000);
        updateSpeedSpinner->setSingleStep(100);
        updateSpeedSpinner->setValue(msecsToTimeout);

        inputWeightSpinner = new QDoubleSpinBox();
        inputWeightSpinner->setMinimum(0.0);
        inputWeightSpinner->setMaximum(1.0);
        inputWeightSpinner->setSingleStep(0.1);

        xmaxSpinner = new QDoubleSpinBox();
        xmaxSpinner->setMinimum(0.1);
        xmaxSpinner->setMaximum(std::numeric_limits<int>::max());
        xmaxSpinner->setSingleStep(0.1);
        xmaxSpinner->setAccelerated(true);
        xmaxSpinner->setValue(conf.xmax);

        xminSpinner = new QDoubleSpinBox();
        xminSpinner->setMinimum(std::numeric_limits<int>::min());
        xminSpinner->setMaximum(std::numeric_limits<int>::max());
        xminSpinner->setSingleStep(0.1);
        xminSpinner->setAccelerated(true);
        xminSpinner->setValue(conf.xmin);

        ymaxSpinner = new QSpinBox();
        ymaxSpinner->setValue(1000);
        ymaxSpinner->setMinimum(std::numeric_limits<int>::min());
        ymaxSpinner->setMaximum(std::numeric_limits<int>::max());
        ymaxSpinner->setAccelerated(true);
        ymaxSpinner->setEnabled(false);

        nofBinsBox = new QComboBox();
        nofBinsBox->addItem("1024",1024);
        nofBinsBox->addItem("2048",2048);
        nofBinsBox->addItem("4096",4096);
        nofBinsBox->addItem("8192",8192);
        nofBinsBox->addItem("16384",16384);
        nofBinsBox->setCurrentIndex(nofBinsBox->findData(conf.nofBins,Qt::UserRole));

        numCountsLabel = new QLabel (tr ("0"));

        connect(previewButton,SIGNAL(clicked()),this,SLOT(previewButtonClicked()));
        connect(resetButton,SIGNAL(clicked()),this,SLOT(resetButtonClicked()));
        connect(updateSpeedSpinner,SIGNAL(valueChanged(int)),this,SLOT(setTimerTimeout(int)));
        connect(inputWeightSpinner,SIGNAL(valueChanged(double)),this,SLOT(inputWeightChanged(double)));
        connect(xmaxSpinner,SIGNAL(valueChanged(double)),this,SLOT(xmaxChanged(double)));
        connect(xminSpinner,SIGNAL(valueChanged(double)),this,SLOT(xminChanged(double)));
        connect(ymaxSpinner,SIGNAL(valueChanged(int)),this,SLOT(ymaxChanged(int)));
        connect(nofBinsBox,SIGNAL(currentIndexChanged(int)),this,SLOT(nofBinsChanged(int)));
        connect(normalizeCheck,SIGNAL(toggled(bool)), this,SLOT(normalizeChanged(bool)));
        connect(autosaveCheck,SIGNAL(toggled(bool)), this,SLOT(autosaveChanged(bool)));
        connect(autoresetCheck,SIGNAL(toggled(bool)), this,SLOT(autoresetChanged(bool)));
        connect(autosaveSpinner,SIGNAL(valueChanged(int)), this,SLOT(autosaveIntChanged(int)));
        connect(autoresetSpinner,SIGNAL(valueChanged(int)), this,SLOT(autoresetIntChanged(int)));

        cl->addWidget(previewButton,0,0,1,2);
        cl->addWidget(resetButton,  0,2,1,2);

        cl->addWidget(normalizeCheck,1,0,1,4);

        cl->addWidget(inputWeightLabel,  2,0,1,1);
        cl->addWidget(inputWeightSpinner,2,1,1,1);
        cl->addWidget(updateSpeedLabel,  2,2,1,1);
        cl->addWidget(updateSpeedSpinner,2,3,1,1);

        cl->addWidget(xminLabel,  3,0,1,1);
        cl->addWidget(xminSpinner,3,1,1,1);
        cl->addWidget(xmaxLabel,  3,2,1,1);
        cl->addWidget(xmaxSpinner,3,3,1,1);

        cl->addWidget(nofBinsLabel,4,0,1,1);
        cl->addWidget(nofBinsBox,  4,1,1,1);
        cl->addWidget(ymaxLabel,   4,2,1,1);
        cl->addWidget(ymaxSpinner, 4,3,1,1);

        cl->addWidget(autosaveCheck,   5,0,1,2);
        cl->addWidget(autosaveIntLabel,5,2,1,1);
        cl->addWidget(autosaveSpinner, 5,3,1,1);

        cl->addWidget(autoresetCheck,   6,0,1,2);
        cl->addWidget(autoresetIntLabel,6,2,1,1);
        cl->addWidget(autoresetSpinner, 6,3,1,1);

        cl->addWidget(new QLabel ("Counts in histogram:"), 7, 0, 1, 1);
        cl->addWidget(numCountsLabel, 7, 1, 1, 3);

        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void CacheHistogramPlugin::xminChanged(double newValue){ conf.xmin = newValue;}
void CacheHistogramPlugin::xmaxChanged(double newValue){ conf.xmax = newValue;}
void CacheHistogramPlugin::ymaxChanged(int newValue){ conf.ymax = newValue;}

void CacheHistogramPlugin::nofBinsChanged(int newValue)
{
    conf.nofBins = nofBinsBox->itemData(newValue,Qt::UserRole).toInt();
}

void CacheHistogramPlugin::normalizeChanged(bool newValue){ conf.normalize = newValue;}
void CacheHistogramPlugin::inputWeightChanged(double newValue){ conf.inputWeight = newValue;}

void CacheHistogramPlugin::autoresetChanged(bool newValue){ conf.autoreset = newValue;}
void CacheHistogramPlugin::autosaveChanged(bool newValue){ conf.autosave = newValue;}
void CacheHistogramPlugin::autoresetIntChanged(int newValue){ conf.autoresetInt = newValue;}
void CacheHistogramPlugin::autosaveIntChanged(int newValue){ conf.autosaveInt = newValue;}

void CacheHistogramPlugin::scheduleWriteToFile()
{
    if(conf.autosave == true) writeToFile = true;
}
void CacheHistogramPlugin::scheduleResetHistogram()
{
    if(conf.autoreset == true)
    {
        fileCount++;
        scheduleReset = true;
    }
}

void CacheHistogramPlugin::updateVisuals()
{
    plot->update ();
    numCountsLabel->setText(tr("%1").arg(nofCounts));
}

/*!
* @fn void CacheHistogramPlugin::userProcess()
* @brief For plugin specific processing code
*
* @warning This function must ONLY be called from the plugin thread;
* @variable pdata wants to be a vector<double> of new values to be inserted into the histogram
*/
void CacheHistogramPlugin::userProcess()
{
    //std::cout << "CacheHistogramPlugin userProcess" << std::endl;
    QVector<double> idata = inputs->first()->getData().value< QVector<double> > ();

    SamDSP dsp;

    if(scheduleReset)
    {
        cache.clear();
        cache.fill(0, conf.nofBins);
        recalculateBinWidth();
        scheduleReset = false;
        plot->resetBoundaries(0);
    }
    if(writeToFile)
    {
        dsp.vectorToFile(cache,RunManager::ref().getRunName().toStdString()
                         +"/"+getName().toStdString()
                         +"_"+tr("%1").arg(fileCount,3,10,QChar('0')).toStdString()
                         +".dat");
        writeToFile = false;
    }

    if((int)(cache.size()) != conf.nofBins) cache.resize(conf.nofBins);

    // Add data to histogram
    foreach(double datum, idata)
    {
        //std::cout << "CacheHistogramPlugin: adding " << std::dec << datum << endl;
        if(datum < conf.xmax && datum >= conf.xmin)
        {
            int bin = (int)((datum - conf.xmin) / binWidth);
            if(bin > 0 && bin < conf.nofBins)
            {
                cache [bin] += conf.inputWeight;
                ++nofCounts;
            }
        }
    }

    if(conf.normalize) dsp.fast_scale(cache,1.0/(dsp.max(cache)[AMP]));

    if(!cache.empty()) {
        QWriteLocker wr (plot->getChanLock());
        plot->getChannelById(0)->setData(cache);
    }

    outputs->at(0)->setData(QVariant::fromValue (cache));
    outputs->at(1)->setData(QVariant::fromValue (cache));
}

void CacheHistogramPlugin::runStartingEvent () {
    // reset all timers and the histogram before starting anew
    halfSecondTimer->stop();
    writeToFileTimer->stop();
    resetTimer->stop();

    scheduleReset = true;
    writeToFile = false;
    nofCounts = 0;

    halfSecondTimer->start(msecsToTimeout);
    writeToFileTimer->start(conf.autosaveInt*1000);
    resetTimer->start(conf.autoresetInt*60*1000);
}
