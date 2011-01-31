#ifndef CAEN820UI_H
#define CAEN820UI_H

#include "caen820module.h"
#include "baseui.h"

#include <vector>

class QCheckBox;
class QComboBox;
class QSpinBox;

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

private:
    Caen820Module *module_;

    QCheckBox *boxShortData;
    QCheckBox *boxHdrEnable;
    QCheckBox *clearMeb;
    QCheckBox *boxAutoRst;

    std::vector<QCheckBox *> boxChEn;

    QSpinBox *sbDwellTime;
    QComboBox *cbAcqMode;
};

#endif // CAEN820UI_H
