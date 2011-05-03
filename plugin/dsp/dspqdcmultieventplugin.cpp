#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include <samdsp.h>

#include "dspqdcmultieventplugin.h"
#include "pluginmanager.h"
#include "pluginconnectorqueued.h"
#include "geckoui.h"
#include "samqvector.h"

static PluginRegistrar registrar ("dspqdcmultievent", DspQdcMultiEventPlugin::create, AbstractPlugin::GroupDSP);

DspQdcMultiEventPlugin::DspQdcMultiEventPlugin(int _id, QString _name)
    : BasePlugin(_id, _name), uif(this,&tabs), scheduleResize(true)
{
    srand(time(NULL));
    createSettings(settingsLayout);

    addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,"in"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::out,"spectrum"));

    outData.fill (0., 4096);

    std::cout << "Instantiated DspQdcSpecPlugin" << std::endl;
}

void DspQdcMultiEventPlugin::createSettings(QGridLayout * l)
{
    tn.append("QDC"); uif.addTab(tn.last());

    gn.append(""); uif.addGroupToTab(tn.last(),gn.last(),"","v");
    uif.addSpinnerToGroup(tn.last(),gn.last(),"Points for Baseline",tr("pointsForBaseline"),1,0x1fffffff);
    uif.addSpinnerToGroup(tn.last(),gn.last(),"Width",tr("width"),1,0x1fffffff);
    uif.addSpinnerToGroup(tn.last(),gn.last(),"Min",tr("min"),0,0x1fffffff);
    uif.addSpinnerToGroup(tn.last(),gn.last(),"Max",tr("max"),1,0x1fffffff);
    uif.addSpinnerToGroup(tn.last(),gn.last(),"Number of Bins",tr("nofBins"),1,0x1fffffff);
    uif.addSpinnerToGroup(tn.last(),gn.last(),"Number of Events",tr("nofEvents"),1,0x1fffffff);

    tn.append("Control"); uif.addTab(tn.last());

    gn.append("hor2"); uif.addUnnamedGroupToTab(tn.last(),gn.last());
    uif.addButtonToGroup(tn.last(),gn.last(),"Reset","reset_button");

    l->addWidget(dynamic_cast<QWidget*>(&tabs),0,0,1,1);
    connect(uif.getSignalMapper(),SIGNAL(mapped(QString)),this,SLOT(uiInput(QString)));
}

// Slot handling

void DspQdcMultiEventPlugin::uiInput(QString _name)
{
    QSpinBox* sb = findChild<QSpinBox*>(_name);
    if(sb != 0)
    {
        if(_name == "pointsForBaseline") conf.pointsForBaseline = sb->value();
        if(_name == "width") conf.width = sb->value();
        if(_name == "min") conf.min = sb->value();
        if(_name == "max") conf.max = sb->value();
        if(_name == "nofBins") conf.nofBins = sb->value();
        if(_name == "nofEvents") conf.nofEvents = sb->value();
    }

    QPushButton* pb = findChild<QPushButton*>(_name);
    if(pb != 0)
    {
        if(_name == "reset_button") clicked_reset_button();
    }
}

void DspQdcMultiEventPlugin::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
        set = "width";   if(settings->contains(set)) conf.width = settings->value(set).toInt();
        set = "pointsForBaseline";   if(settings->contains(set)) conf.pointsForBaseline = settings->value(set).toInt();
        set = "min";   if(settings->contains(set)) conf.min = settings->value(set).toInt();
        set = "max";   if(settings->contains(set)) conf.max = settings->value(set).toInt();
        set = "nofBins";   if(settings->contains(set)) conf.nofBins = settings->value(set).toInt();
        set = "nofEvents";   if(settings->contains(set)) conf.nofEvents = settings->value(set).toInt();
    settings->endGroup();

    outData.fill (0., conf.nofBins);

    QList<QSpinBox*> csb = findChildren<QSpinBox*>();
    if(!csb.empty())
    {
        QList<QSpinBox*>::const_iterator it = csb.begin();
        while(it != csb.end())
        {
            QSpinBox* w = (*it);
            if(w->objectName() == "width") w->setValue(conf.width);
            if(w->objectName() == "pointsForBaseline") w->setValue(conf.pointsForBaseline);
            if(w->objectName() == "min") w->setValue(conf.min);
            if(w->objectName() == "max") w->setValue(conf.max);
            if(w->objectName() == "nofBins") w->setValue(conf.nofBins);
            if(w->objectName() == "nofEvents") w->setValue(conf.nofEvents);

            it++;
        }
    }
}

void DspQdcMultiEventPlugin::saveSettings(QSettings* settings)
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
            settings->setValue("nofEvents",conf.nofEvents);
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void DspQdcMultiEventPlugin::userProcess()
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
        //nofHiClip = 0;
        //nofLowClip = 0;
    }

    // Sanity checks
    if(conf.nofEvents <= 0) { std::cout << "DspQdcMultiEventPlugin: conf.nofEvents <= 0" << std::endl; return; }
    if(conf.pointsForBaseline <= 0) { std::cout << "DspQdcMultiEventPlugin: conf.pointsForBaseline <= 0" << std::endl; return; }
    if(conf.max <= 0) { std::cout << "DspQdcMultiEventPlugin: conf.max <= 0" << std::endl; return; }

    //std::cout << "DspQdcMultiEventPlugin Processing" << std::endl;
    QVector<uint32_t> idata_full = inputs->first()->getData().value< QVector<uint32_t> > ();

    int length = idata_full.size() / conf.nofEvents;

    for(int ev = 0; ev < conf.nofEvents; ev++)
    {
        int pos = ev*length;
        QVector<uint32_t> idata = idata_full.mid(pos,length);

        // Estimate baseline
        tmp = 0.;
        for(int i = 0; i < conf.pointsForBaseline && i < idata.size(); i++)
        {
            tmp += idata[i];
            //std::cout << i << "  " << data[i] << std::endl;
        }
        baselineArea = (tmp * conf.width) / conf.pointsForBaseline;

        // Integrate
        tmp = 0.;
        for(int i = conf.pointsForBaseline; i < conf.width + conf.pointsForBaseline && i < idata.size(); i++)
        {
            tmp += idata[i];
            if(idata[i] == 0) loCnt++;
            if(idata[i] == 4095) hiCnt++;
        }

        // Check for clipping
        if(hiCnt > 1)
        {
            std::cout << "hiclip" << std::endl;
            //nofHiClip++;
        }
        else if(loCnt > 1)
        {
            std::cout << "loclip" << std::endl;
            //nofLowClip++;
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

            //outputs->at(1)->setData (QVariant::fromValue<double> (tmp));

            // Determine bin
            tmp -= conf.min;
            bin = floor(((conf.nofBins * tmp) / conf.max) + (rand()/(RAND_MAX+1.0))-0.5);

            // Sort into histogram
            if(bin > 0 && bin < conf.nofBins)
            {
                //std::cout << "qdc: "  << tmp << " -> " << bin << std::endl;
                outData [bin]++;
            }
            else
            {
                std::cout << "out of range: " << tmp << " -> " << std::dec << bin << std::endl;
            }
        }
    }

    /*for(int i = 0; i < conf.nofBins; i++)
    {
        if(outData.at(i) > 0)
        {
            std::cout << "dspqdcmulti: <" << i << "> :" << outData.at(i) << std::endl; fflush(stdout);
        }
    }*/

    outputs->first()->setData(QVariant::fromValue (outData));
}

void DspQdcMultiEventPlugin::clicked_reset_button()
{
    scheduleResize = true;
}
