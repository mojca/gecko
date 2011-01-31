#include "caen1290ui.h"
#include "caen1290module.h"
#include "caen_v1290.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QTabWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QPushButton>

#include <QVariant>
#include <QSignalMapper>

enum EdgeDet { EdPair = 0, EdTrail = 1, EdLead = 2, EdLeadTrail = 3 };
enum ResMode { Rm800 = 0, Rm200 = 1, Rm100 = 2, Rm25 = 3 };
enum DeadTime { Dt5 = 0, Dt10 = 1, Dt30 = 2, Dt100 = 3 };

Caen1290UI::Caen1290UI (Caen1290Module *mod)
: BaseUI (mod)
, module_ (mod)
{
    createUI ();
    applySettings ();
}

void Caen1290UI::createUI () {
    QGridLayout *l = new QGridLayout (this);

    QGroupBox *box = new QGroupBox (tr ("%1 Settings").arg (module_->getName ()), this);
    QGridLayout *boxl = new QGridLayout (box);
    QTabWidget *tabs = new QTabWidget (this);

    tabs->addTab (createDeviceCfg (), tr ("General"));
    tabs->addTab (createAcqCfg (), tr ("Acquisition Mode"));
    tabs->addTab (createEventCfg (), tr ("Event Detection"));
    tabs->addTab (createAdjustCfg (), tr ("Adjustments"));

    boxl->addWidget (tabs, 0, 0, 1, 1);
    box->setLayout (boxl);

    l->addWidget (box, 0, 0, 1, 1);
    setLayout (l);
}

QWidget *Caen1290UI::createDeviceCfg () {
    QWidget *pane = new QWidget (this);
    QGridLayout *l = new QGridLayout (pane);

    QGroupBox *grp = new QGroupBox (tr ("Board Configuration"), pane);
    QGridLayout *grpl = new QGridLayout (grp);

    boxBerrEn = new QCheckBox (tr ("BERR"), grp);
    boxAlign64 = new QCheckBox (tr ("ALIGN64"), grp);
    boxTermSw = new QCheckBox (tr ("Software TERM"), grp);
    boxTerm = new QCheckBox (tr ("TERM"), grp);
    boxCompEn = new QCheckBox (tr ("Compensation"), grp);
    boxEvFifoEn = new QCheckBox (tr ("Event FIFO"), grp);
    boxExTimeTagEn = new QCheckBox (tr ("Ext. Time Tag"), grp);

    grpl->addWidget (boxBerrEn,  0,0,1,1);
    grpl->addWidget (boxAlign64, 0,1,1,1);
    grpl->addWidget (boxTermSw,  1,0,1,1);
    grpl->addWidget (boxTerm,    1,1,1,1);
    grpl->addWidget (boxCompEn,  2,0,1,1);
    grpl->addWidget (boxEvFifoEn,2,1,1,1);
    grpl->addWidget (boxExTimeTagEn,3,0,1,2);

    QSignalMapper *map = new QSignalMapper (this);
    map->setMapping (boxBerrEn, CAEN1290_CTL_BERREN);
    map->setMapping (boxAlign64, CAEN1290_CTL_ALIGN64);
    map->setMapping (boxTermSw, CAEN1290_CTL_TERM_SW);
    map->setMapping (boxTerm, CAEN1290_CTL_TERM);
    map->setMapping (boxCompEn, CAEN1290_CTL_COMPEN);
    map->setMapping (boxEvFifoEn, CAEN1290_CTL_EVFIFOEN);
    map->setMapping (boxExTimeTagEn, CAEN1290_CTL_EXTRGTIMETAG);

    connect (boxBerrEn, SIGNAL(toggled (bool)), map, SLOT(map ()));
    connect (boxAlign64, SIGNAL(toggled (bool)), map, SLOT(map ()));
    connect (boxTermSw, SIGNAL(toggled (bool)), map, SLOT(map ()));
    connect (boxTerm, SIGNAL(toggled (bool)), map, SLOT(map ()));
    connect (boxCompEn, SIGNAL(toggled (bool)), map, SLOT(map ()));
    connect (boxEvFifoEn, SIGNAL(toggled (bool)), map, SLOT(map ()));
    connect (boxExTimeTagEn, SIGNAL(toggled (bool)), map, SLOT(map ()));

    connect (map, SIGNAL(mapped (int)), SLOT(updateCtlFlag (int)));

    grp->setLayout (grpl);   
    l->addWidget(grp,0,0,1,1);
    l->setRowStretch (1, 1);
    pane->setLayout (l);
    return pane;
}

QWidget *Caen1290UI::createAcqCfg () {
    int row = 0;
    QWidget *pane = new QWidget (this);
    QGridLayout *l = new QGridLayout (pane);

    QLabel *lbl = new QLabel (tr ("Acquisition Mode:"), pane);
    cbAcqMode = new QComboBox (pane);
    cbAcqMode->addItem (tr ("Triggered"), true);
    cbAcqMode->addItem (tr ("Continuous"), false);
    connect (cbAcqMode, SIGNAL(currentIndexChanged(int)), SLOT(updateAcqMode(int)));
    l->addWidget (lbl, row, 0, 1, 1);
    l->addWidget (cbAcqMode, row, 1, 1, 1);
    ++row;

    lbl = new QLabel (tr ("Window width:"), pane);
    sbWinWidth = new QSpinBox (pane);
    sbWinWidth->setMinimum (0);
    sbWinWidth->setMaximum ((1 << 13) - 1);
    connect (sbWinWidth, SIGNAL(valueChanged(int)), SLOT(updateWinWidth(int)));
    l->addWidget (lbl, row, 0, 1, 1);
    l->addWidget (sbWinWidth, row, 1, 1, 1);
    ++row;

    lbl = new QLabel (tr ("Window offset:"), pane);
    sbWinOffset = new QSpinBox (pane);
    sbWinOffset->setMinimum (0);
    sbWinOffset->setMaximum ((1 << 13) - 1);
    connect (sbWinOffset, SIGNAL(valueChanged(int)), SLOT(updateWinOffset(int)));
    l->addWidget (lbl, row, 0, 1, 1);
    l->addWidget (sbWinOffset, row, 1, 1, 1);
    ++row;

    lbl = new QLabel (tr ("Search margin:"), pane);
    sbSwMargin = new QSpinBox (pane);
    sbSwMargin->setMinimum (0);
    sbSwMargin->setMaximum ((1 << 13) - 1);
    connect (sbSwMargin, SIGNAL(valueChanged(int)), SLOT(updateSwMargin(int)));
    l->addWidget (lbl, row, 0, 1, 1);
    l->addWidget (sbSwMargin, row, 1, 1, 1);
    ++row;

    lbl = new QLabel (tr ("Reject margin:"), pane);
    sbRejMargin = new QSpinBox (pane);
    sbRejMargin->setMinimum (0);
    sbRejMargin->setMaximum ((1 << 13) - 1);
    connect (sbRejMargin, SIGNAL(valueChanged(int)), SLOT(updateRejMargin(int)));
    l->addWidget (lbl, row, 0, 1, 1);
    l->addWidget (sbRejMargin, row, 1, 1, 1);
    ++row;

    boxSubTrig = new QCheckBox (tr ("Subtract trigger time"), pane);
    connect (boxSubTrig, SIGNAL(toggled(bool)), SLOT(updateSubTrg(bool)));
    l->addWidget (boxSubTrig, row, 1, 1, 1);
    ++row;

    l->setRowStretch (row, 1);
    pane->setLayout (l);
    return pane;
}

QWidget *Caen1290UI::createEventCfg () {
    int row = 0;
    QLabel *lbl;
    QWidget *pane = new QWidget (this);
    QGridLayout *l = new QGridLayout (pane);

    lbl = new QLabel (tr ("Edge Detection Mode:"), pane);
    cbEdgeDetect = new QComboBox (pane);
    cbEdgeDetect->addItem (tr ("Leading Edge"), QVariant (EdLead));
    cbEdgeDetect->addItem (tr ("Trailing Edge"), QVariant (EdTrail));
    cbEdgeDetect->addItem (tr ("Both Edges"), QVariant (EdLeadTrail));
    connect (cbEdgeDetect, SIGNAL(currentIndexChanged(int)), SLOT(updateEdgeDet(int)));
    l->addWidget (lbl, row, 0, 1, 1);
    l->addWidget (cbEdgeDetect, row, 1, 1, 1);
    ++row;

    lbl = new QLabel (tr ("Edge Resolution:"), pane);
    cbEdgeLsb = new QComboBox (pane);
    if (module_->type_.hires ())
        cbEdgeLsb->addItem (tr ("25 ps"), QVariant (Rm25));
    cbEdgeLsb->addItem (tr ("100 ps"), QVariant (Rm100));
    cbEdgeLsb->addItem (tr ("200 ps"), QVariant (Rm200));
    cbEdgeLsb->addItem (tr ("800 ps"), QVariant (Rm800));
    connect (cbEdgeLsb, SIGNAL(currentIndexChanged(int)), SLOT(updateEdgeLsb(int)));
    l->addWidget (lbl, row, 0, 1, 1);
    l->addWidget (cbEdgeLsb, row, 1, 1, 1);
    ++row;

    lbl = new QLabel (tr ("Dead Time:"), pane);
    cbHitDeadTime = new QComboBox (pane);
    cbHitDeadTime->addItem (tr ("~5 ns"), QVariant (Dt5));
    cbHitDeadTime->addItem (tr ("~10 ns"), QVariant (Dt10));
    cbHitDeadTime->addItem (tr ("~30 ns"), QVariant (Dt30));
    cbHitDeadTime->addItem (tr ("~100 ns"), QVariant (Dt100));
    connect (cbHitDeadTime, SIGNAL(currentIndexChanged(int)), SLOT(updateDeadTime(int)));
    l->addWidget (lbl, row, 0, 1, 1);
    l->addWidget (cbHitDeadTime, row, 1, 1, 1);
    ++row;

    l->setRowStretch (row, 1);
    pane->setLayout (l);
    return pane;
}

QWidget *Caen1290UI::createAdjustCfg () {
    int row = 0;
    QWidget *pane = new QWidget (this);
    QGridLayout *l = new QGridLayout (pane);

    QGroupBox *grp = new QGroupBox (tr ("Enabled Channels"), pane);
    QGridLayout *grpl = new QGridLayout (grp);
    QSignalMapper *map = new QSignalMapper (this);

    boxEnCh.resize (module_->type_.channels ());
    if (module_->type_.channels () > 32) {
        QTabWidget *tabw = new QTabWidget (grp);
        char paneidx ('A');

        for (int chs = module_->type_.channels (), i = 0; chs > 0; chs -= 32, ++i, ++paneidx)
            tabw->addTab (createEnChPane (tabw, map, std::min (chs, 32), 32*i),
                          tr ("Bank %1").arg (QString (paneidx)));
        grpl->addWidget (tabw, 0, 0, 1, 3);
    } else {
        grpl->addWidget (createEnChPane (grp, map, module_->type_.channels ()), 0, 0, 1, 1);
    }

    QSignalMapper *checkmap = new QSignalMapper (this);
    QPushButton *checkallBtn = new QPushButton(tr ("Check &all"));
    QPushButton *uncheckallBtn = new QPushButton(tr ("&Uncheck all"));

    checkmap->setMapping (checkallBtn, 1);
    checkmap->setMapping (uncheckallBtn, 0);
    connect (checkallBtn, SIGNAL(clicked()), checkmap, SLOT(map()));
    connect (uncheckallBtn, SIGNAL(clicked()), checkmap, SLOT(map()));
    connect (checkmap, SIGNAL(mapped(int)), SLOT(checkAllChans(int)));

    grpl->addWidget (checkallBtn, 1, 1, 1, 1);
    grpl->addWidget (uncheckallBtn, 1, 2, 1, 1);
    grpl->setColumnStretch (0, 1);

    connect (map, SIGNAL(mapped (int)), SLOT(updateEnCh (int)));
    grp->setLayout (grpl);
    l->addWidget (grp, row, 0, 1, 2);
    ++row;

    grp = new QGroupBox (tr ("Channel adjust"), pane);
    grpl = new QGridLayout (grp);
    map = new QSignalMapper (this);

    sbChAdj.resize (module_->type_.channels ());
    if (module_->type_.channels () > 32) {
        QTabWidget *tabw = new QTabWidget (grp);
        char paneidx ('A');

        for (int chs = module_->type_.channels (), i = 0; chs > 0; chs -= 32, ++i, ++paneidx)
            tabw->addTab (createChAdjPane (tabw, map, std::min (chs, 32), 32*i),
                          tr ("Bank %1").arg (QString (paneidx)));
        grpl->addWidget (tabw, 0, 0, 1, 1);
    } else {
        grpl->addWidget (createChAdjPane (grp, map, module_->type_.channels ()), 0, 0, 1, 1);
    }


    connect (map, SIGNAL(mapped (int)), SLOT (updateChAdj (int)));
    grp->setLayout (grpl);
    l->addWidget (grp, row, 0, 1, 2);
    ++row;

    grp = new QGroupBox (tr ("RC adjust"), pane);
    grpl = new QGridLayout (grp);
    map = new QSignalMapper (this);

    for (int i = 0; i < 4; ++i) {
        grpl->addWidget (new QLabel (tr ("%1:").arg (i)), i, 0, 1, 1);
        for (int j = 0; j < 12; ++j) {
            boxRcAdj [i][j] = new QCheckBox (grp);
            map->setMapping(boxRcAdj [i][j], 12 * i + j);
            connect (boxRcAdj [i][j], SIGNAL(toggled(bool)), map, SLOT(map()));
            grpl->addWidget (boxRcAdj [i][j], i, j + 1 + j/3);
        }
        grpl->setColumnMinimumWidth (4, 5);
        grpl->setColumnMinimumWidth (8, 5);
        grpl->setColumnMinimumWidth (12, 5);
    }

    connect (map, SIGNAL (mapped (int)), SLOT (updateRcAdj (int)));
    grp->setLayout (grpl);
    l->addWidget (grp, row, 0, 1, 2);
    grp->setVisible(module_->type_.hires ());
    ++row;

    l->addWidget (new QLabel ("Global Offset (coarse):", this), row, 0, 1, 1);
    sbGOCoarse = new QSpinBox (this);
    sbGOCoarse->setMinimum (0);
    sbGOCoarse->setMaximum ((1 << 12) - 1);
    connect (sbGOCoarse, SIGNAL(valueChanged(int)), SLOT(updateCoarseOffset(int)));
    l->addWidget (sbGOCoarse, row++, 1, 1, 1);

    l->addWidget (new QLabel ("Global Offset (fine):", this), row, 0, 1, 1);
    sbGOFine = new QSpinBox (this);
    sbGOFine->setMinimum (0);
    sbGOFine->setMaximum ((1 << 6) - 1);
    connect (sbGOFine, SIGNAL(valueChanged(int)), SLOT(updateFineOffset(int)));
    l->addWidget (sbGOFine, row++, 1, 1, 1);

    l->setSizeConstraint (QLayout::SetMinAndMaxSize);
    pane->setLayout (l);

    QScrollArea *sa = new QScrollArea (this);
    sa->setWidget (pane);
    sa->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
    sa->setMinimumWidth (sa->sizeHint ().width () + sa->verticalScrollBar ()->sizeHint ().width ());
    return sa;
}

QWidget *Caen1290UI::createChAdjPane (QWidget *parent, QSignalMapper *map, int num, int offset) {
    QWidget *pane = new QWidget (parent);
    QGridLayout *l = new QGridLayout (pane);

    for (int i = 0; i < num; ++i) {
        sbChAdj [offset+i] = new QSpinBox ();
        sbChAdj [offset+i]->setMinimum (0);
        sbChAdj [offset+i]->setMaximum (0xFF);
        QLabel *lbl = new QLabel (tr ("%1:").arg (offset+i));
        map->setMapping (sbChAdj [offset+i], offset+i);
        connect (sbChAdj [offset+i], SIGNAL(valueChanged(int)), map, SLOT(map()));
        l->addWidget (lbl, i / 4, 2 * (i % 4), 1, 1);
        l->addWidget (sbChAdj [offset+i], i / 4, 2 * (i % 4) + 1, 1, 1);
    }
    pane->setLayout (l);
    return pane;
}

QWidget *Caen1290UI::createEnChPane (QWidget *parent, QSignalMapper *map, int num, int offset) {
    QWidget *pane = new QWidget (parent);
    QGridLayout *l = new QGridLayout (pane);

    for (int i = 0; i < num; ++i) {
        boxEnCh [offset+i] = new QCheckBox (tr ("%1").arg (offset+i));
        map->setMapping (boxEnCh [offset+i], offset+i);
        connect (boxEnCh [offset+i], SIGNAL(toggled(bool)), map, SLOT(map()));
        l->addWidget (boxEnCh [offset+i], i / 8, i % 8, 1, 1);
    }
    pane->setLayout (l);
    return pane;
}

void Caen1290UI::applySettings () {
    Caen1290Config *cfg = module_->conf_;

    boxBerrEn->setChecked (cfg->berr_enable);
    boxAlign64->setChecked (cfg->align64);
    boxTermSw->setChecked (cfg->term_sw);
    boxTerm->setChecked (cfg->term);
    boxCompEn->setChecked (cfg->comp_enable);
    boxEvFifoEn->setChecked (cfg->ev_fifo_enable);
    boxExTimeTagEn->setChecked (cfg->extrigtimetag_enable);

    for (int i = 0; i < cbAcqMode->count(); ++i) {
        if (cbAcqMode->itemData (i).toBool () == cfg->trig_match_mode) {
            cbAcqMode->setCurrentIndex (i);
            break;
        }
    }

    sbWinWidth->setValue (cfg->win_width);
    sbWinOffset->setValue (cfg->win_offset);
    sbSwMargin->setValue (cfg->sw_margin);
    sbRejMargin->setValue (cfg->rej_margin);
    boxSubTrig->setChecked (cfg->sub_trg);

    for (int i = 0; i < cbEdgeDetect->count(); ++i) {
        if (cbEdgeDetect->itemData (i).toInt () == cfg->edge_detect_mode) {
            cbEdgeDetect->setCurrentIndex (i);
            break;
        }
    }

    for (int i = 0; i < cbEdgeLsb->count(); ++i) {
        if (cbEdgeLsb->itemData (i).toInt () == cfg->edge_lsb_val) {
            cbEdgeLsb->setCurrentIndex (i);
            break;
        }
    }

    for (int i = 0; i < cbHitDeadTime->count(); ++i) {
        if (cbHitDeadTime->itemData (i).toInt () == cfg->hit_dead_time) {
            cbHitDeadTime->setCurrentIndex (i);
            break;
        }
    }

    for (int chans = module_->type_.channels (), j = 0; chans > 0; chans -= 16, ++j)
        for (int i = 0; i < std::min (chans, 16); ++i)
            boxEnCh [16*j+i]->setChecked (((cfg->enable_chan_bitmap [j] >> i) & 1) != 0);

    for (int i = 0; i < module_->type_.channels (); ++i)
        sbChAdj [i]->setValue (cfg->channel_adj [i]);

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 12; ++j)
            boxRcAdj [i][j]->setChecked (((cfg->rc_adj [i] >> j) & 1) != 0);

    sbGOCoarse->setValue (cfg->global_offset_coarse);
    sbGOFine->setValue (cfg->global_offset_fine);
}

void Caen1290UI::updateCtlFlag (int flag) {
    Caen1290Config *cfg = module_->conf_;

    switch (flag) {
    case CAEN1290_CTL_BERREN:   cfg->berr_enable = boxBerrEn->isChecked (); break;
    case CAEN1290_CTL_ALIGN64:  cfg->align64 = boxAlign64->isChecked (); break;
    case CAEN1290_CTL_TERM_SW:  cfg->term_sw = boxTermSw->isChecked (); break;
    case CAEN1290_CTL_TERM:     cfg->term = boxTerm->isChecked (); break;
    case CAEN1290_CTL_COMPEN:   cfg->comp_enable = boxCompEn->isChecked (); break;
    case CAEN1290_CTL_EVFIFOEN: cfg->ev_fifo_enable = boxEvFifoEn->isChecked (); break;
    case CAEN1290_CTL_EXTRGTIMETAG: cfg->extrigtimetag_enable = boxExTimeTagEn->isChecked (); break;
    }
}

void Caen1290UI::updateAcqMode (int idx) {
    module_->conf_->trig_match_mode = cbAcqMode->itemData (idx).toBool ();
}

void Caen1290UI::updateWinWidth (int val) {
    module_->conf_->win_width = val;
}

void Caen1290UI::updateWinOffset (int val) {
    module_->conf_->win_offset = val;
}

void Caen1290UI::updateSwMargin (int val) {
    module_->conf_->sw_margin = val;
}

void Caen1290UI::updateRejMargin (int val) {
    module_->conf_->rej_margin = val;
}

void Caen1290UI::updateSubTrg (bool val) {
    module_->conf_->sub_trg = val;
}

void Caen1290UI::updateEdgeDet (int idx) {
    module_->conf_->edge_detect_mode = cbEdgeDetect->itemData (idx).toInt ();
}

void Caen1290UI::updateEdgeLsb (int idx) {
    module_->conf_->edge_lsb_val = cbEdgeLsb->itemData (idx).toInt ();
}

void Caen1290UI::updateDeadTime (int idx) {
    module_->conf_->hit_dead_time = cbHitDeadTime->itemData (idx).toInt ();
}

void Caen1290UI::updateCoarseOffset (int val) {
    module_->conf_->global_offset_coarse = val;
}

void Caen1290UI::updateFineOffset (int val) {
    module_->conf_->global_offset_fine = val;
}

void Caen1290UI::updateEnCh (int idx) {
    if (boxEnCh [idx]->isChecked ())
        module_->conf_->enable_chan_bitmap [idx / 16] |= (1 << (idx % 16));
    else
        module_->conf_->enable_chan_bitmap [idx / 16] &= ~(1 << (idx % 16));
}

void Caen1290UI::updateChAdj (int idx) {
    module_->conf_->channel_adj [idx] = sbChAdj [idx]->value ();
}

void Caen1290UI::updateRcAdj (int idx) {
    int rc = idx / 12;
    int bit = idx % 12;

    if (boxRcAdj [rc][bit]->isChecked ())
        module_->conf_->rc_adj [rc] |= (1 << bit);
    else
        module_->conf_->rc_adj [rc] &= ~(1 << bit);
}

void Caen1290UI::checkAllChans (int check) {
    for (std::vector<QCheckBox*>::iterator i = boxEnCh.begin (); i != boxEnCh.end (); ++i)
        (*i)->setChecked (check != 0);
}
