#ifndef DSPPILEUPCORRECTIONPLUGIN_H
#define DSPPILEUPCORRECTIONPLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include "baseplugin.h"
#include "samdsp.h"

class BasePlugin;
class QSpinBox;
class QCheckBox;

struct DspPileUpCorrectionPluginConfig
{
    int signalsLeft;
    int signalsRight;
    int samplesLeft;
    int samplesRight;
};

class DspPileUpCorrectionPlugin : public virtual BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    DspPileUpCorrectionPluginConfig conf;

    QSpinBox* signalsLeftSpinner;
    QSpinBox* signalsRightSpinner;
    QSpinBox* samplesLeftSpinner;
    QSpinBox* samplesRightSpinner;

public:
    DspPileUpCorrectionPlugin(int _id, QString _name);
    static AbstractPlugin *create (int id, const QString &name, const Attributes &attrs) {
        Q_UNUSED (attrs);
        return new DspPileUpCorrectionPlugin (id, name);
    }
    virtual void userProcess();

    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
    void signalsLeftChanged(int);
    void signalsRightChanged(int);
    void samplesLeftChanged(int);
    void samplesRightChanged(int);

};

#endif // DSPPILEUPCORRECTIONPLUGIN_H
