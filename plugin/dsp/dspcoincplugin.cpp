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

#include "dspcoincplugin.h"
#include "pluginconnectorqueued.h"
#include "pluginconnectorplain.h"
#include "confmap.h"
#include "samdsp.h"
#include "samqvector.h"
#include "pluginmanager.h"

#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QTimer>
#include <limits>
#include <iostream>
#include <algorithm>

static PluginRegistrar reg ("dspcoinc", DspCoincPlugin::create, AbstractPlugin::GroupDSP, DspCoincPlugin::attributeMap ());

struct ConfigDspCoinc {
    int delay;
    int width;
    bool anyopener;
    bool trgtimestamps;

    ConfigDspCoinc ()
    : delay (-5)
    , width (10)
    , anyopener (false)
    , trgtimestamps (false)
    {}
};

/*static*/ AbstractPlugin::AttributeMap DspCoincPlugin::attributeMap () {
    AttributeMap map;
    map.insert ("nofTriggers", QVariant::Int);
    map.insert ("nofDataChannels", QVariant::Int);
    return map;
}

DspCoincPlugin::DspCoincPlugin (int id, QString name, Attributes attrs)
: BasePlugin (id, name)
, attrs_ (attrs)
, conf_ (new ConfigDspCoinc)
, nCoinc (0)
, nNoCoinc (0)
{
    ntriggers_ = attrs_.value ("nofTriggers", 2).toInt ();
    ndata_ = attrs_.value ("nofDataChannels", 1).toInt ();

    if (ntriggers_ < 2) {
        std::cout << "Invalid number of trigger channels. Setting to 2!" << std::endl;
        ntriggers_ = 2;
    }

    if (ndata_ <= 0) {
        std::cout << "Invalid number of data channels. Setting to 1" << std::endl;
        ndata_ = 1;
    }

    attrs_.insert ("nofTriggers", ntriggers_);
    attrs_.insert ("nofDataChannels", ndata_);

    createSettings (settingsLayout);

    for (int i = 0; i < ntriggers_; ++i) {
        addConnector (new PluginConnectorPlain (this, ScopeCommon::in, QString("trigger%1").arg (i), PluginConnector::VectorDouble));
    }

    for (int i = 0; i < ndata_; ++i) {
        addConnector (new PluginConnectorPlain (this, ScopeCommon::in, QString("in%1").arg (i), PluginConnector::VectorDouble));
        addConnector (new PluginConnectorQVDouble (this, ScopeCommon::out, QString("out%1").arg (i)));
    }
}

void DspCoincPlugin::createSettings (QGridLayout *l) {
    l->addWidget (new QLabel (tr ("Trigger inputs:")), 0, 0, 1, 1);
    l->addWidget (new QLabel (QString ("%1").arg(ntriggers_)), 0, 1, 1, 1);

    l->addWidget (new QLabel (tr ("Data channels:")), 1, 0, 1, 1);
    l->addWidget (new QLabel (QString ("%1").arg(ndata_)), 1, 1, 1, 1);

    boxGateOpener_ = new QComboBox ();
    boxGateOpener_->addItem (tr ("First"), QVariant::fromValue (false));
    boxGateOpener_->addItem (tr ("Any"), QVariant::fromValue (true));
    boxGateOpener_->setCurrentIndex (0);
    l->addWidget (new QLabel (tr ("Gate opener:")), 2, 0, 1, 1);
    l->addWidget (boxGateOpener_, 2, 1, 1, 1);

    sbDelay_ = new QSpinBox ();
    sbDelay_->setMinimum (std::numeric_limits<int>::min ());
    sbDelay_->setMaximum (std::numeric_limits<int>::max ());
    sbDelay_->setValue (conf_->delay);
    l->addWidget (new QLabel (tr ("Delay:")), 3, 0, 1, 1);
    l->addWidget (sbDelay_, 3, 1, 1, 1);

    sbWidth_ = new QSpinBox ();
    sbWidth_->setMinimum (0);
    sbWidth_->setMaximum (std::numeric_limits<int>::max ());
    sbWidth_->setValue (conf_->width);
    l->addWidget (new QLabel (tr ("Width:")), 4, 0, 1, 1);
    l->addWidget (sbWidth_, 4, 1, 1, 1);

    cbTimestamps_ = new QCheckBox (tr ("Trigger inputs carry timestamps"));
    cbTimestamps_->setChecked (conf_->trgtimestamps);
    l->addWidget (cbTimestamps_, 5, 0, 1, 2);

    l->setRowStretch (6, 1);

    lblCoinc = new QLabel (tr ("Coincidences:   0.0%"));
    l->addWidget (lblCoinc, 7, 0, 1, 2);

    QTimer *tim = new QTimer (this);
    tim->setInterval (500);
    tim->start ();

    connect (boxGateOpener_, SIGNAL(currentIndexChanged(int)), SLOT(gateOpenerChanged(int)));
    connect (sbDelay_, SIGNAL(valueChanged(int)), SLOT(delayChanged(int)));
    connect (sbWidth_, SIGNAL(valueChanged(int)), SLOT(widthChanged(int)));
    connect (cbTimestamps_, SIGNAL(toggled(bool)), SLOT(timestampChanged(bool)));
    connect (tim, SIGNAL(timeout()), SLOT(updateCoincData()));
}

void DspCoincPlugin::userProcess () {
    QVector< QVector<double> > triggers;
    triggers.reserve (ntriggers_);

    if (conf_->trgtimestamps) {
        for (int i = 0; i < ntriggers_; ++i)
            triggers << inputs->at (i)->getData ().value< QVector<double> > ();
    } else {
        // generate trigger timestamps from signal
        SamDSP dsp;
        for (int i = 0; i < ntriggers_; ++i) {
            QVector<double> t = inputs->at (i)->getData ().value< QVector<double> > ();
            triggers << dsp.select (t, t)[TIME];
        }
    }

    for (int i = 0; i < (conf_->anyopener ? ntriggers_: 1); ++i) {
        for (int j = 0; j < triggers.at (i).size (); ++j) {
            double triggertime = triggers.at(i).at(j);
            bool coinc = true;

            // look for triggers within window [triggertime+delay,triggertime+delay+width]
            for (int k = 0; k < ntriggers_; ++k) {
                if (k == i)
                    continue;

                // find first value not smaller than lower interval boundary
                const QVector<double> &r = triggers.at (k);
                QVector<double>::const_iterator lb = std::lower_bound (r.constBegin(), r.constEnd (), triggertime + conf_->delay);

                if (lb == r.end () || *lb > triggertime + conf_->delay + conf_->width) {
                    // no match, coincidence between all channels not possible. Try next timestamp
                    coinc = false;
                    break;
                }
            }

            if (coinc) { // found a coincidence, pass data on and finish processing
                ++nCoinc;
                for (int i = 0; i < ndata_; ++i)
                    outputs->at (i)->setData (inputs->at(i + ntriggers_)->getData ());
                return;
            }
        }
        // no coincidence from trigger gates opened by this channel, try next one
    }
    // no coincidences found
    ++nNoCoinc;
}

typedef ConfMap::confmap_t<ConfigDspCoinc> confmap_t;
static const confmap_t confmap [] = {
    confmap_t ("delay", &ConfigDspCoinc::delay),
    confmap_t ("width", &ConfigDspCoinc::width),
    confmap_t ("any_opener", &ConfigDspCoinc::anyopener),
    confmap_t ("trg_timestamps", &ConfigDspCoinc::trgtimestamps)
};

void DspCoincPlugin::applySettings (QSettings *s) {
    s->beginGroup (getName());
    ConfMap::apply (s, conf_, confmap);
    s->endGroup ();

    for (int i = 0; i < boxGateOpener_->count(); ++i) {
        if (boxGateOpener_->itemData (i).toBool () == conf_->anyopener) {
            boxGateOpener_->setCurrentIndex (i);
        }
    }
    sbDelay_->setValue (conf_->delay);
    sbWidth_->setValue (conf_->width);
    cbTimestamps_->setChecked (conf_->trgtimestamps);
}

void DspCoincPlugin::saveSettings (QSettings *s) {
    s->beginGroup (getName());
    ConfMap::save (s, conf_, confmap);
    s->endGroup ();
}

void DspCoincPlugin::gateOpenerChanged (int idx) {
    if (idx < 0)
        return;

    conf_->anyopener = boxGateOpener_->itemData (idx).toBool();
}

void DspCoincPlugin::delayChanged (int del) {
    conf_->delay = del;
}

void DspCoincPlugin::widthChanged (int wdt) {
    conf_->width = wdt;
}

void DspCoincPlugin::timestampChanged (bool chk) {
    conf_->trgtimestamps = chk;
}

void DspCoincPlugin::updateCoincData () {
    if (nCoinc + nNoCoinc > 0)
        lblCoinc->setText (tr ("Coincidences: %1%").arg((100.0 * nCoinc)/(nCoinc + nNoCoinc), 4, 'f', 1));
}

void DspCoincPlugin::runStartingEvent () {
    nCoinc = 0;
    nNoCoinc = 0;
}
