#include "dspcfdplugin.h"
#include "confmap.h"
#include "pluginconnectorqueued.h"
#include "../../samdsp/samdsp.h"
#include "samqvector.h"

#include <QLabel>
#include <QGridLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>

struct DspCfdConfig {
    double fraction;
    uint32_t threshold;
    uint32_t delay;
    uint32_t holdoff;
};

DspCfdPlugin::DspCfdPlugin(int _id, QString _name)
: BasePlugin(_id, _name)
, conf (new DspCfdConfig)
{
    createSettings (settingsLayout);

    addConnector (new PluginConnectorQVDouble (this, ScopeCommon::in, "signal"));
    addConnector (new PluginConnectorQVDouble (this, ScopeCommon::out, "trigger"));
    addConnector (new PluginConnectorQVDouble (this, ScopeCommon::out, "times"));
    addConnector (new PluginConnectorQVDouble (this, ScopeCommon::out, "cfdsig"));

}

void DspCfdPlugin::createSettings(QGridLayout *l) {
    QLabel *lbl = new QLabel (tr ("Performs Constant Fraction Discrimination and outputs a trigger and timestamp"));
    l->addWidget (lbl, 0, 0, 1, 2);

    fractionSpinner_ = new QDoubleSpinBox ();
    fractionSpinner_->setDecimals (3);
    fractionSpinner_->setMinimum (0);
    fractionSpinner_->setMaximum (1);
    l->addWidget (new QLabel (tr ("Fraction:")), 1, 0, 1, 1);
    l->addWidget (fractionSpinner_, 1, 1, 1, 1);

    delaySpinner_ = new QSpinBox ();
    delaySpinner_->setMinimum (0);
    delaySpinner_->setMaximum (1000);
    l->addWidget (new QLabel (tr ("Delay:")), 2, 0, 1, 1);
    l->addWidget (delaySpinner_, 2, 1, 1, 1);

    thresholdSpinner_ = new QSpinBox ();
    thresholdSpinner_->setMinimum (0);
    thresholdSpinner_->setMinimum (4096);
    l->addWidget (new QLabel (tr ("Threshold:")), 3, 0, 1, 1);
    l->addWidget (thresholdSpinner_, 3, 1, 1, 1);

    holdoffSpinner_ = new QSpinBox ();
    holdoffSpinner_->setMinimum (0);
    holdoffSpinner_->setMaximum (1000);
    l->addWidget (new QLabel (tr ("Holdoff:")), 4, 0, 1, 1);
    l->addWidget (holdoffSpinner_, 4, 1, 1, 1);

    l->setRowStretch (5, 1);

    connect (fractionSpinner_, SIGNAL(valueChanged(double)), SLOT(fractionChanged(double)));
    connect (delaySpinner_, SIGNAL(valueChanged(int)), SLOT(delayChanged(int)));
    connect (thresholdSpinner_, SIGNAL(valueChanged(int)), SLOT(thresholdChanged(int)));
    connect (holdoffSpinner_, SIGNAL(valueChanged(int)), SLOT(holdoffChanged(int)));
}

void DspCfdPlugin::fractionChanged (double frac) {
    conf->fraction = frac;
}

void DspCfdPlugin::delayChanged (int del) {
    conf->delay = del;
}

void DspCfdPlugin::thresholdChanged (int thr) {
    conf->threshold = thr;
}

void DspCfdPlugin::holdoffChanged (int hol) {
    conf->holdoff = hol;
}

void DspCfdPlugin::userProcess () {
    QVector<double> signal = inputs->at (0)->getData ().value< QVector<double> > ();
    SamDSP dsp;

    QVector< QVector<double> > cfd = dsp.triggerACFD (signal, conf->threshold, conf->fraction, conf->delay, conf->holdoff);
    QVector< QVector<double> > cfdtimes = dsp.select (cfd.at (AMP), cfd.at (TIME));

    outputs->at(0)->setData (QVariant::fromValue (cfd.at (TIME)));
    outputs->at(1)->setData (QVariant::fromValue (dsp.add (cfdtimes.at (0), cfdtimes.at (1)))); // precision timestamps
    outputs->at(2)->setData (QVariant::fromValue (cfd.at(2)));
}
