#include "caen820ui.h"

#include <QGridLayout>
#include <QTabWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QSignalMapper>

Caen820UI::Caen820UI(Caen820Module *mod)
: BaseUI (mod)
, module_ (mod)
{
    createUI ();
    applySettings ();
}

void Caen820UI::createUI () {
    QGroupBox *settingsbox = new QGroupBox (tr ("%1 Settings").arg (module_->getName ()));
    QGridLayout *l = new QGridLayout (settingsbox);

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
