#include "dspcfdplugin.h"
#include "confmap.h"
#include "pluginconnectorqueued.h"
#include <samdsp.h>
#include "samqvector.h"
#include "pluginmanager.h"
#include "confmap.h"

#include <QLabel>
#include <QGridLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>

static PluginRegistrar registrar ("dspcfd", DspCfdPlugin::create, AbstractPlugin::GroupDSP, AbstractPlugin::AttributeMap ());

struct DspCfdConfig {
    double fraction;
    uint32_t threshold;
    bool negative;
    uint32_t holdoff;
    uint32_t baseline;

    DspCfdConfig ()
    : fraction (0.1)
    , threshold (40)
    , negative (false)
    , holdoff (20)
    , baseline (10)
    {}
};

DspCfdPlugin::DspCfdPlugin(int _id, QString _name)
: BasePlugin(_id, _name)
, conf (new DspCfdConfig)
{
    createSettings (settingsLayout);

    addConnector (new PluginConnectorQVDouble (this, ScopeCommon::in, "signal"));
    addConnector (new PluginConnectorQVDouble (this, ScopeCommon::out, "trigger"));
    addConnector (new PluginConnectorQVDouble (this, ScopeCommon::out, "times"));
}

void DspCfdPlugin::createSettings(QGridLayout *l) {
    QLabel *lbl = new QLabel (tr ("Performs Constant Fraction Discrimination and outputs a trigger and timestamp"));
    l->addWidget (lbl, 0, 0, 1, 2);

    fractionSpinner_ = new QDoubleSpinBox ();
    fractionSpinner_->setDecimals (3);
    fractionSpinner_->setSingleStep (0.001);
    fractionSpinner_->setMinimum (0);
    fractionSpinner_->setMaximum (1);
    l->addWidget (new QLabel (tr ("Fraction:")), 1, 0, 1, 1);
    l->addWidget (fractionSpinner_, 1, 1, 1, 1);

    thresholdSpinner_ = new QSpinBox ();
    thresholdSpinner_->setMinimum (0);
    thresholdSpinner_->setMaximum (4096);
    l->addWidget (new QLabel (tr ("Threshold:")), 2, 0, 1, 1);
    l->addWidget (thresholdSpinner_, 2, 1, 1, 1);

    negativeBox_ = new QCheckBox (tr ("Negative polarity"));
    l->addWidget (negativeBox_, 3, 1, 1, 2);

    holdoffSpinner_ = new QSpinBox ();
    holdoffSpinner_->setMinimum (0);
    holdoffSpinner_->setMaximum (1000);
    l->addWidget (new QLabel (tr ("Holdoff:")), 4, 0, 1, 1);
    l->addWidget (holdoffSpinner_, 4, 1, 1, 1);

    baselineSpinner_ = new QSpinBox ();
    baselineSpinner_->setMinimum (0);
    baselineSpinner_->setMaximum (1000);
    l->addWidget (new QLabel (tr ("Points for Baseline:")), 5, 0, 1, 1);
    l->addWidget (baselineSpinner_, 5, 1, 1, 1);

    l->setRowStretch (6, 1);

    fractionSpinner_->setValue (conf->fraction);
    negativeBox_->setChecked (conf->negative);
    thresholdSpinner_->setValue (conf->threshold);
    holdoffSpinner_->setValue (conf->holdoff);
    baselineSpinner_->setValue (conf->baseline);

    connect (fractionSpinner_, SIGNAL(valueChanged(double)), SLOT(fractionChanged(double)));
    connect (negativeBox_, SIGNAL(toggled(bool)), SLOT(negativeChanged(bool)));
    connect (thresholdSpinner_, SIGNAL(valueChanged(int)), SLOT(thresholdChanged(int)));
    connect (holdoffSpinner_, SIGNAL(valueChanged(int)), SLOT(holdoffChanged(int)));
    connect (baselineSpinner_, SIGNAL(valueChanged(int)), SLOT(baselineChanged(int)));
}

void DspCfdPlugin::fractionChanged (double frac) {
    conf->fraction = frac;
}

void DspCfdPlugin::negativeChanged (bool neg) {
    conf->negative = neg;
}

void DspCfdPlugin::thresholdChanged (int thr) {
    conf->threshold = thr;
}

void DspCfdPlugin::holdoffChanged (int hol) {
    conf->holdoff = hol;
}

void DspCfdPlugin::baselineChanged (int bas) {
    conf->baseline = bas;
}

void DspCfdPlugin::userProcess () {
    QVector<double> signal = inputs->at (0)->getData ().value< QVector<double> > ();
    SamDSP dsp;

    // estimate baseline
    double bl = 0;
    for (uint32_t i = 0; i < conf->baseline; ++i)
        bl += signal.at (i);
    bl /= conf->baseline;

    dsp.fast_addC (signal, -bl);
    if (conf->negative)
        dsp.fast_scale (signal, -1);

    QVector<double> lmax = dsp.triggerLMT (signal, conf->threshold, conf->holdoff);
    QVector< QVector<double> > cfd = dsp.triggerCFD (signal, lmax, conf->fraction, conf->holdoff);
    QVector< QVector<double> > cfdtimes = dsp.select (cfd.at (AMP), cfd.at (TIME));

    outputs->at(0)->setData (QVariant::fromValue (cfd.at (TIME)));
    outputs->at(1)->setData (QVariant::fromValue (dsp.add (cfdtimes.at (0), cfdtimes.at (1)))); // precision timestamps
}

typedef ConfMap::confmap_t<DspCfdConfig> confmap_t;
static const confmap_t confmap [] = {
    confmap_t ("fraction", &DspCfdConfig::fraction),
    confmap_t ("negative", &DspCfdConfig::negative),
    confmap_t ("threshold", &DspCfdConfig::threshold),
    confmap_t ("holdoff", &DspCfdConfig::holdoff),
    confmap_t ("baseline", &DspCfdConfig::baseline)
};

void DspCfdPlugin::applySettings(QSettings *settings) {
    settings->beginGroup (getName ());
    ConfMap::apply (settings, conf, confmap);
    settings->endGroup ();

    fractionSpinner_->setValue (conf->fraction);
    negativeBox_->setChecked (conf->negative);
    thresholdSpinner_->setValue (conf->threshold);
    holdoffSpinner_->setValue (conf->holdoff);
}

void DspCfdPlugin::saveSettings(QSettings *settings) {
    settings->beginGroup (getName ());
    ConfMap::save (settings, conf, confmap);
    settings->endGroup ();
}

/*!
\page dspcfdplg Constant Fraction Discriminator Plugin
\li <b>Plugin names:</b> \c dspcfdplugin
\li <b>Group:</b> DSP

\section pdesc Plugin Description
The constant fraction discriminator generates a trigger signal from its input.
It looks for peaks in the signal larger than a given threshold and searches backwards from the peak maximum to the point the signal rises above the given fraction of its maximum amplitude.
At that point it generates a trigger. The crossing time is deduced by linearly interpolating between the last point below and the first point above the fraction so the trigger jitter is less than 0.5 samples.

The plugin makes the trigger available as a logic signal and a list of timestamps with sub-sample precision.

\section attrs Attributes
None

\section conf Configuration
\li \b Fraction: The fraction at which the trigger should be generated
\li \b Threshold: The signal threshold. Only peaks that exceed this threshold are considered
\li <b>Negative Polarity</b>: If checked the discriminator searches for negative peaks
\li \b Holdoff: Holdoff period after a trigger has been generated
\li <b>Points for Baseline</b>: Number of points to average over for estimating the baseline.
    These points are taken to be at the beginning of the input and no trigger search is performed on them.

\section inputs Input Connectors
\li \c signal \c &lt;double>: Signal to generate triggers from

\section outputs Output Connectors
\li \c trigger \c &lt;double>: The generated triggers as logic signal
\li \c times \c &lt;double>: The generated triggers as timestamps with sub-sample precision
*/
