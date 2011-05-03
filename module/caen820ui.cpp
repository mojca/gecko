#include "caen820ui.h"

#include <QGridLayout>
#include <QTabWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QSignalMapper>
#include <QTabWidget>
#include <QTimer>
#include <QPushButton>
#include <QMessageBox>

Caen820UI::Caen820UI(Caen820Module *mod)
: module_ (mod)
, monitorTimer (new QTimer (this))
{
    createUI ();
    applySettings ();

    monitorTimer->setInterval (1000);
    connect (monitorTimer, SIGNAL(timeout()), SLOT(updateMonitor()));
}

void Caen820UI::createUI () {
    QGroupBox *settingsbox = new QGroupBox (tr ("%1 Settings").arg (module_->getName ()));
    QTabWidget *tabs = new QTabWidget (settingsbox);

    {
        QWidget *settingspane = new QWidget ();
        QGridLayout *l = new QGridLayout (settingspane);

        boxHdrEnable = new QCheckBox (tr ("Enable Header"));
        boxShortData = new QCheckBox (tr ("26 bit data format"));
        boxAutoRst = new QCheckBox (tr ("Auto-Reset on Trigger"));

        int row = 0;
        l->addWidget(boxHdrEnable, row++, 0, 1, 2);
        l->addWidget(boxShortData, row++, 0, 1, 2);
        l->addWidget(boxAutoRst, row++, 0, 1, 2);

        cbAcqMode = new QComboBox ();
        cbAcqMode->setEditable (false);
        cbAcqMode->addItem (tr ("Ext. Trigger"), 1);
        cbAcqMode->addItem (tr ("Periodic"), 2);
        l->addWidget (new QLabel (tr ("Acquisition mode:")), row, 0, 1, 1);
        l->addWidget (cbAcqMode, row++, 1, 1, 1);

        sbDwellTime = new QSpinBox ();
        sbDwellTime->setMinimum (3);
        sbDwellTime->setMaximum (INT_MAX);
        sbDwellTime->setSuffix (tr (" x 400 ns"));
        sbDwellTime->setAccelerated (true);
        l->addWidget (new QLabel (tr ("Dwell Time:")), row, 0, 1, 1);
        l->addWidget (sbDwellTime, row++, 1, 1, 1);

        QGroupBox *grp = new QGroupBox (tr ("Enabled Channels"));
        QGridLayout *grpl = new QGridLayout (grp);
        QSignalMapper *map = new QSignalMapper (this);

        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 8; ++col) {
                QCheckBox *box = new QCheckBox (tr ("%1").arg (row * 8 + col));
                grpl->addWidget (box, row, col, 1, 1);
                map->setMapping (box, row * 8 + col);
                connect (box, SIGNAL(toggled(bool)), map, SLOT(map()));
                boxChEn.push_back (box);
            }
        }

        l->addWidget (grp, row++, 0, 1, 2);

        l->setRowStretch (row, 1);

        connect (map, SIGNAL(mapped(int)), SLOT(updateChannels(int)));
        connect (boxHdrEnable, SIGNAL(toggled(bool)), SLOT(updateHdrEn(bool)));
        connect (boxShortData, SIGNAL(toggled(bool)), SLOT(updateShortData(bool)));
        connect (boxAutoRst, SIGNAL(toggled(bool)), SLOT(updateAutoRst(bool)));
        connect (sbDwellTime, SIGNAL(valueChanged(int)), SLOT(updateDwellTime(int)));
        connect (cbAcqMode, SIGNAL(currentIndexChanged(int)), SLOT(updateAcqMode(int)));
        tabs->addTab (settingspane, tr ("Settings"));
    }

    {
        QWidget *monitorpane = new QWidget ();
        QGridLayout *l = new QGridLayout (monitorpane);
        l->setColumnStretch (1, 1);
        l->setColumnStretch (3, 1);

        btnStartStop = new QPushButton (tr ("Start Monitoring"));
        l->addWidget (btnStartStop, 0, 0, 1, 2);

        btnCounterReset = new QPushButton (tr ("Reset Counters"));
        l->addWidget (btnCounterReset, 0, 2, 1, 2);

        for (int i = 0; i < 32; ++i) {
            QLabel *lbl = new QLabel (tr ("0"));
            lbl->setFrameStyle(QFrame::Panel | QFrame::Sunken);
            lbl->setAlignment (Qt::AlignRight);

            l->addWidget (new QLabel (tr ("%1:").arg (i)), i % 16 + 1, 2 * (i / 16), 1, 1);
            l->addWidget (lbl, i % 16 + 1, 2 * (i / 16) + 1, 1, 1);
            lblChMon.push_back (lbl);
        }
        connect (btnStartStop, SIGNAL(clicked()), SLOT(startMonitor ()));
        connect (btnCounterReset, SIGNAL(clicked()), SLOT(resetCounters()));

        tabs->addTab (monitorpane, tr ("Monitor"));
    }

    (new QGridLayout (settingsbox))->addWidget (tabs, 0, 0, 1, 1);
    (new QGridLayout (this))->addWidget (settingsbox, 0, 0, 1, 1);
}

void Caen820UI::applySettings () {
    boxHdrEnable->setChecked (module_->conf_.hdr_enable);
    boxShortData->setChecked (module_->conf_.short_data_format);
    boxAutoRst->setChecked (module_->conf_.auto_reset);

    sbDwellTime->setValue (module_->conf_.dwell_time);

    for (int i = 0; i < cbAcqMode->count (); ++i ) {
        if (cbAcqMode->itemData(i, Qt::UserRole) == module_->conf_.acq_mode) {
            cbAcqMode->setCurrentIndex (i);
            break;
        }
    }

    for (int i = 0; i < 32; ++i)
        boxChEn.at (i)->setChecked ((module_->conf_.channel_enable & (1 << i)) != 0);
}

void Caen820UI::updateAcqMode(int idx) {
    module_->conf_.acq_mode = cbAcqMode->itemData (idx, Qt::UserRole).toUInt ();
}

void Caen820UI::updateAutoRst(bool enable) {
    module_->conf_.auto_reset = enable;
}

void Caen820UI::updateDwellTime(int time) {
    module_->conf_.dwell_time = time;
}

void Caen820UI::updateHdrEn(bool enable) {
    module_->conf_.hdr_enable = enable;
}

void Caen820UI::updateShortData(bool enable) {
    module_->conf_.short_data_format = enable;
}

void Caen820UI::updateChannels (int ch) {
    bool enabled = boxChEn.at (ch)->isChecked ();

    if (enabled)
        module_->conf_.channel_enable |= (1 << ch);
    else
        module_->conf_.channel_enable &= ~(1 << ch);
}

void Caen820UI::startMonitor () {
    if (module_->getInterface ()) {
        if (!module_->getInterface ()->isOpen ())
            module_->getInterface ()->open ();

        if (0 != module_->configure ()) {
            QMessageBox::warning (this, tr ("<%1> Caen Scaler").arg (module_->getName ()), tr ("Configuring failed. Check base address."), QMessageBox::Ok);
            return;
        }

        monitorTimer->start ();
        btnStartStop->setText (tr ("Stop Monitor"));
        disconnect (btnStartStop, SIGNAL(clicked()), this, SLOT(startMonitor()));
        connect (btnStartStop, SIGNAL(clicked()), this, SLOT(stopMonitor()));

        updateMonitor();
    }
}

void Caen820UI::stopMonitor () {
    monitorTimer->stop();
    btnStartStop->setText (tr ("Start Monitor"));
    disconnect (btnStartStop, SIGNAL(clicked()), this, SLOT(stopMonitor()));
    connect (btnStartStop, SIGNAL(clicked()), this, SLOT(startMonitor()));
}

void Caen820UI::updateMonitor () {
    if (!module_->getInterface ()->isOpen ())
        stopMonitor ();

    QVector<uint32_t> data = module_->acquireMonitor ();

    for (int i = 0; i < 32; ++i) {
        lblChMon [i]->setText (tr ("%1").arg (data [i]));
    }
}

void Caen820UI::resetCounters () {
    if (module_->getInterface () && module_->getInterface ()->isOpen ())
        module_->dataClear ();
}
