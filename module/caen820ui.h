#ifndef CAEN820UI_H
#define CAEN820UI_H

#include "caen820module.h"
#include "baseui.h"

#include <QVector>

class QCheckBox;
class QComboBox;
class QSpinBox;
class QLabel;
class QPushButton;
class QTimer;

class Caen820UI : public BaseUI
{
    Q_OBJECT
public:
    explicit Caen820UI(Caen820Module *m);

    void createUI ();
    void applySettings ();

private slots:
    void updateChannels (int);
    void updateHdrEn (bool);
    void updateShortData (bool);
    void updateAutoRst (bool);
    void updateDwellTime (int);
    void updateAcqMode (int);

    void startMonitor ();
    void stopMonitor ();
    void updateMonitor ();
    void resetCounters ();

private:
    Caen820Module *module_;

    QCheckBox *boxShortData;
    QCheckBox *boxHdrEnable;
    QCheckBox *clearMeb;
    QCheckBox *boxAutoRst;

    QVector<QCheckBox *> boxChEn;

    QSpinBox *sbDwellTime;
    QComboBox *cbAcqMode;

    QVector<QLabel *> lblChMon;
    QPushButton *btnStartStop;
    QPushButton *btnCounterReset;
    QTimer *monitorTimer;
};

#endif // CAEN820UI_H
