#include <cstdlib>
#include "dspqdcspecplugin.h"
#include "pluginmanager.h"
#include "runmanager.h"
#include "pluginconnectorqueued.h"

#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>

static PluginRegistrar registrar ("dspqdcspec", DspQdcSpecPlugin::create, AbstractPlugin::GroupDSP);

DspQdcSpecPlugin::DspQdcSpecPlugin(int _id, QString _name)
    : BasePlugin(_id, _name)
{
    srand(time(NULL));

    nofLowClip = 0;
    nofHiClip = 0;
    estimateForBaseline = 0;

    outData.fill (0., 4096);

    createSettings(settingsLayout);

    addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,"in"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::out,"spectrum"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::out,"value"));

    halfSecondTimer = new QTimer();
    halfSecondTimer->start(500);
    connect(halfSecondTimer,SIGNAL(timeout()),this,SLOT(updateUI()));

    scheduleResize = false;

    std::cout << "Instantiated DspQdcSpecPlugin" << std::endl;
}

void DspQdcSpecPlugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        QLabel* label = new QLabel(tr("This plugin creates a qdc spectrum from the input data."));
        QLabel* wlabel = new QLabel(tr("Integration Width"));
        QLabel* blabel = new QLabel(tr("Points for Baseline"));
        QLabel* minlabel = new QLabel(tr("Min value"));
        QLabel* maxlabel = new QLabel(tr("Max value"));
        QLabel* nofBinslabel = new QLabel(tr("Number of bins"));
        QLabel* llabel = new QLabel(tr("Low Clip"));
        QLabel* hlabel = new QLabel(tr("High Clip"));

        widthSpinner = new QSpinBox();
        baselineSpinner = new QSpinBox();
        minValueSpinner = new QSpinBox();
        maxValueSpinner = new QSpinBox();
        nofBinsSpinner = new QSpinBox();

        lowClip = new QLineEdit(tr("%1").arg(nofLowClip,1,10));
        lowClip->setReadOnly(true);
        hiClip = new QLineEdit(tr("%1").arg(nofHiClip,1,10));
        hiClip->setReadOnly(true);

        widthSpinner->setRange(1,16000);
        baselineSpinner->setRange(1,16000);
        minValueSpinner->setRange(0,1000000);
        maxValueSpinner->setRange(1,1000000);
        nofBinsSpinner->setRange(1,1000000);

        widthSpinner->setValue(20);
        baselineSpinner->setValue(10);
        minValueSpinner->setValue(0);
        maxValueSpinner->setValue(100);
        nofBinsSpinner->setValue(4096);

        resetButton = new QPushButton(tr("Reset spectra"));
        connect(resetButton,SIGNAL(clicked()),this,SLOT(resetSpectra()));

        connect(widthSpinner,SIGNAL(valueChanged(int)),this,SLOT(widthChanged()));
        connect(baselineSpinner,SIGNAL(valueChanged(int)),this,SLOT(baselineChanged()));
        connect(minValueSpinner,SIGNAL(valueChanged(int)),this,SLOT(minChanged()));
        connect(maxValueSpinner,SIGNAL(valueChanged(int)),this,SLOT(maxChanged()));
        connect(nofBinsSpinner,SIGNAL(valueChanged(int)),this,SLOT(nofBinsChanged()));

        cl->addWidget(label,0,0,1,2);
        cl->addWidget(wlabel,1,0,1,1);
        cl->addWidget(blabel,2,0,1,1);
        cl->addWidget(minlabel,3,0,1,1);
        cl->addWidget(maxlabel,4,0,1,1);
        cl->addWidget(nofBinslabel,5,0,1,1);
        cl->addWidget(resetButton,6,0,1,2);
        cl->addWidget(llabel,7,0,1,1);
        cl->addWidget(hlabel,8,0,1,1);
        cl->addWidget(widthSpinner,1,1,1,1);
        cl->addWidget(baselineSpinner,2,1,1,1);
        cl->addWidget(minValueSpinner,3,1,1,1);
        cl->addWidget(maxValueSpinner,4,1,1,1);
        cl->addWidget(nofBinsSpinner,5,1,1,1);
        cl->addWidget(lowClip,7,1,1,1);
        cl->addWidget(hiClip,8,1,1,1);

        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void DspQdcSpecPlugin::widthChanged()
{
    int newValue = widthSpinner->value();
    this->conf.width = newValue;
    scheduleResize = true;
}

void DspQdcSpecPlugin::baselineChanged()
{
    int newValue = baselineSpinner->value();
    this->conf.pointsForBaseline = newValue;
    scheduleResize = true;
}

void DspQdcSpecPlugin::minChanged()
{
    int newValue = minValueSpinner->value();
    this->conf.min = newValue;
    scheduleResize = true;
}

void DspQdcSpecPlugin::maxChanged()
{
    int newValue = maxValueSpinner->value();
    this->conf.max = newValue;
    scheduleResize = true;
}

void DspQdcSpecPlugin::nofBinsChanged()
{
    int newValue = nofBinsSpinner->value();
    this->conf.nofBins = newValue;
    scheduleResize = true;
}

void DspQdcSpecPlugin::updateUI()
{
    hiClip->setText(tr("%1").arg(nofHiClip,1,10));
    lowClip->setText(tr("%1").arg(nofLowClip,1,10));
}

void DspQdcSpecPlugin::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
        set = "width";   if(settings->contains(set)) conf.width = settings->value(set).toInt();
        set = "pointsForBaseline";   if(settings->contains(set)) conf.pointsForBaseline = settings->value(set).toInt();
        set = "min";   if(settings->contains(set)) conf.min = settings->value(set).toInt();
        set = "max";   if(settings->contains(set)) conf.max = settings->value(set).toInt();
        set = "nofBins";   if(settings->contains(set)) conf.nofBins = settings->value(set).toInt();
    settings->endGroup();

    outData.fill (0., conf.nofBins);
    widthSpinner->setValue(conf.width);
    baselineSpinner->setValue(conf.pointsForBaseline);
    minValueSpinner->setValue(conf.min);
    maxValueSpinner->setValue(conf.max);
    nofBinsSpinner->setValue(conf.nofBins);
}

void DspQdcSpecPlugin::saveSettings(QSettings* settings)
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
            settings->setValue("width",conf.width);
            settings->setValue("pointsForBaseline",conf.pointsForBaseline);
            settings->setValue("min",conf.min);
            settings->setValue("max",conf.max);
            settings->setValue("nofBins",conf.nofBins);
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void DspQdcSpecPlugin::userProcess()
{
    int tmp = 0;
    int baselineArea = 0;
    int hiCnt = 0;
    int loCnt = 0;
    int bin = 0;

    if(scheduleResize)
    {
        outData.fill (0, conf.nofBins);
        scheduleResize = false;
        nofHiClip = 0;
        nofLowClip = 0;
    }

    //std::cout << "DspQdcSpecPlugin Processing" << std::endl;
    QVector<uint32_t> idata = inputs->first()->getData().value< QVector<uint32_t> > ();

    // Estimate baseline
    tmp = 0.;
    for(int i = 0; i<conf.pointsForBaseline && i < idata.size(); i++)
    {
        tmp += idata[i];
        //std::cout << i << "  " << data[i] << std::endl;
    }
    baselineArea = (tmp * conf.width) / conf.pointsForBaseline;

    // Integrate
    tmp = 0.;
    for(int i = conf.pointsForBaseline; i<conf.width+conf.pointsForBaseline && i< idata.size(); i++)
    {
        tmp += idata[i];
        if(idata[i] == 0) loCnt++;
        if(idata[i] == 4095) hiCnt++;
    }

    // Check for clipping
    if(hiCnt > 1)
    {
        //std::cout << "hiclip" << std::endl;
        nofHiClip++;
    }
    else if(loCnt > 1)
    {
        //std::cout << "loclip" << std::endl;
        nofLowClip++;
    }
    else
    {
        // Correct baseline
        tmp -= baselineArea;

        // Correct polarity
        if(tmp < 0)
        {
            tmp *= -1;
        }

        outputs->at(1)->setData (QVariant::fromValue<double> (tmp));

        // Determine bin
        tmp -= conf.min;
        bin = floor(((conf.nofBins * tmp) / conf.max) + (rand()/(RAND_MAX+1.0))-0.5);

        // Sort into histogram
        if(bin > 0 && bin < conf.nofBins)
        {
            //std::cout << "qdc: "  << tmp << std::endl;
            outData [bin]++;
        }
        else
        {
            //std::cout << "out of range: " << std::dec << bin << std::endl;
        }
    }

    outputs->first()->setData(QVariant::fromValue (outData));
}

void DspQdcSpecPlugin::resetSpectra()
{
    scheduleResize = true;

    nofLowClip = 0;
    lowClip->setText(tr("%1").arg(nofLowClip,1,10));
    nofHiClip = 0;
    hiClip->setText(tr("%1").arg(nofHiClip,1,10));

}
