#ifndef CAEN1290UI_H
#define CAEN1290UI_H

#include "baseui.h"

class Caen1290Module;
class QWidget;
class QCheckBox;
class QComboBox;
class QSpinBox;
class QSignalMapper;

class Caen1290UI : public BaseUI
{
    Q_OBJECT
public:
    Caen1290UI (Caen1290Module *);

    virtual void applySettings ();

public slots:
    void updateCtlFlag (int);

    void updateAcqMode (int);
    void updateWinWidth (int);
    void updateWinOffset (int);
    void updateSwMargin (int);
    void updateRejMargin (int);
    void updateSubTrg (bool);

    void updateEdgeDet (int);
    void updateEdgeLsb (int);
    void updateDeadTime (int);

    void updateEnCh (int);
    void updateChAdj (int);
    void updateRcAdj (int);
    void updateCoarseOffset (int);
    void updateFineOffset (int);

    void checkAllChans (int);

protected:
    virtual void createUI ();

private:
    QWidget *createDeviceCfg ();
    QWidget *createAcqCfg ();
    QWidget *createEventCfg ();
    QWidget *createAdjustCfg ();

    QWidget *createEnChPane (QWidget *parent, QSignalMapper *map, int num=32, int offset=0);
    QWidget *createChAdjPane (QWidget *parent, QSignalMapper *map, int num=32, int offset=0);
private:
    Caen1290Module *module_;

    QCheckBox *boxBerrEn;
    QCheckBox *boxTerm;
    QCheckBox *boxTermSw;
    QCheckBox *boxAlign64;
    QCheckBox *boxCompEn;
    QCheckBox *boxEvFifoEn;
    QCheckBox *boxExTimeTagEn;

    QComboBox *cbAcqMode;
    QSpinBox  *sbWinWidth;
    QSpinBox  *sbWinOffset;
    QSpinBox  *sbSwMargin;
    QSpinBox  *sbRejMargin;
    QCheckBox *boxSubTrig;

    QComboBox *cbEdgeDetect;
    QComboBox *cbEdgeLsb;
    QComboBox *cbHitDeadTime;

    std::vector<QCheckBox *> boxEnCh;
    std::vector<QSpinBox *>  sbChAdj;
    QCheckBox *boxRcAdj [4][12];

    QSpinBox  *sbGOCoarse;
    QSpinBox  *sbGOFine;

};

#endif // CAEN1290UI_H
