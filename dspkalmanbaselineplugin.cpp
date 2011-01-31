#include "dspkalmanbaselineplugin.h"
#include "pluginmanager.h"
#include "pluginconnectorqueued.h"

#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QCheckBox>

static PluginRegistrar registrar ("dspkalmanbaseline", DspKalmanBaselinePlugin::create, AbstractPlugin::GroupDSP);

DspKalmanBaselinePlugin::DspKalmanBaselinePlugin(int _id, QString _name)
    : BasePlugin(_id, _name)
{
    createSettings(settingsLayout);

    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"signal"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::out,"baseline"));

    std::cout << "Instantiated DspKalmanBaselinePlugin" << std::endl;
}

void DspKalmanBaselinePlugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        QLabel* label = new QLabel(tr("This plugin implements a baseline follower based on Kalman filtering."));
        QLabel* errlabel = new QLabel(tr("Error (Model)"));
        QLabel* errIlabel = new QLabel(tr("Error (Invention)"));
        QLabel* deltalabel = new QLabel(tr("Delta"));

        errSpinner = new QDoubleSpinBox();
        errISpinner = new QDoubleSpinBox();
        deltaSpinner = new QDoubleSpinBox();

        deltaSpinner->setMaximum(10);
        deltaSpinner->setMinimum(0.001);
        deltaSpinner->setSingleStep(0.001);
        deltaSpinner->setDecimals(3);
        errSpinner->setMaximum(10);
        errSpinner->setMinimum(0.001);
        errSpinner->setSingleStep(0.001);
        errSpinner->setDecimals(3);
        errISpinner->setMaximum(10);
        errISpinner->setMinimum(0.001);
        errISpinner->setSingleStep(0.001);
        errISpinner->setDecimals(3);

        errSpinner->setValue(conf.err);
        errISpinner->setValue(conf.errI);
        deltaSpinner->setValue(conf.delta);

        connect(errSpinner,SIGNAL(valueChanged(double)),this,SLOT(errChanged(double)));
        connect(errISpinner,SIGNAL(valueChanged(double)),this,SLOT(errIChanged(double)));
        connect(deltaSpinner,SIGNAL(valueChanged(double)),this,SLOT(deltaChanged(double)));

        cl->addWidget(label,0,0,1,2);
        cl->addWidget(errlabel,1,0,1,1);
        cl->addWidget(errSpinner,1,1,1,1);
        cl->addWidget(errIlabel,2,0,1,1);
        cl->addWidget(errISpinner,2,1,1,1);
        cl->addWidget(deltalabel,3,0,1,1);
        cl->addWidget(deltaSpinner,3,1,1,1);

        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void DspKalmanBaselinePlugin::errChanged(double newValue)
{
    this->conf.err = newValue;
}
void DspKalmanBaselinePlugin::errIChanged(double newValue)
{
    this->conf.errI = newValue;
}
void DspKalmanBaselinePlugin::deltaChanged(double newValue)
{
    this->conf.delta = newValue;
}

void DspKalmanBaselinePlugin::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
        set = "err";     if(settings->contains(set)) conf.err   = settings->value(set).toDouble();
        set = "errI";    if(settings->contains(set)) conf.errI  = settings->value(set).toDouble();
        set = "delta";   if(settings->contains(set)) conf.delta = settings->value(set).toDouble();
    settings->endGroup();

    // UI update
    errSpinner->setValue(conf.err);
    errISpinner->setValue(conf.errI);
    deltaSpinner->setValue(conf.delta);

}

void DspKalmanBaselinePlugin::saveSettings(QSettings* settings)
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
            settings->setValue("err",conf.err);
            settings->setValue("errI",conf.errI);
            settings->setValue("delta",conf.delta);
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void DspKalmanBaselinePlugin::userProcess()
{
    //std::cout << "DspKalmanBaselinePlugin Processing" << std::endl;
    const vector<double>* psignal = reinterpret_cast<const std::vector<double>*>(inputs->at(0)->getData());

    SamDSP dsp;

    // Compact input data
    //outData.resize(psignal->size(),0);
    double x0 = 0;
    if(outData.size() > 0) x0 = outData.back();
    outData.clear();

    // Kalman filter data
    double r  = conf.err;
    double ri = conf.errI;
    double q  = conf.delta;
    dsp.kalmanBaseline((*psignal),outData,r,ri,q,x0);


    outputs->first()->setData(&outData);
}

