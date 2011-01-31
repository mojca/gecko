#ifndef DSPQDCSPECPLUGIN_H
#define DSPQDCSPECPLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include <QPushButton>
#include <QLineEdit>
#include <QTimer>
#include "baseplugin.h"
#include "../../samdsp/samdsp.h"

class BasePlugin;
class QSpinBox;

struct DspQdcSpecPluginConfig
{
    int width;
    int pointsForBaseline;
    int min;
    int max;
    int nofBins;
};

class DspQdcSpecPlugin : public virtual BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    DspQdcSpecPluginConfig conf;

    QSpinBox* widthSpinner;
    QSpinBox* baselineSpinner;
    QSpinBox* nofBinsSpinner;
    QSpinBox* minValueSpinner;
    QSpinBox* maxValueSpinner;

    QPushButton* resetButton;

    QLineEdit* lowClip;
    QLineEdit* hiClip;

    QTimer* halfSecondTimer;

    vector<double> outData;

    double estimateForBaseline;
    double estimateForAmplitude;

    unsigned int nofLowClip;
    unsigned int nofHiClip;

    bool scheduleResize;

public:
    DspQdcSpecPlugin(int _id, QString _name);
    static BasePlugin *create (int _id, const QString &_name, const Attributes &_attrs) {
        Q_UNUSED (_attrs);
        return new DspQdcSpecPlugin (_id, _name);
    }

    virtual void userProcess();

    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
    void widthChanged();
    void baselineChanged();
    void resetSpectra();
    void minChanged();
    void maxChanged();
    void nofBinsChanged();
    void updateUI();

};

#endif // DSPQDCSPECPLUGIN_H
