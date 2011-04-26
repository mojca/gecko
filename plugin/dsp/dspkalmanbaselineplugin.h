#ifndef DSPKALMANBASELINEPLUGIN_H
#define DSPKALMANBASELINEPLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include "baseplugin.h"
#include "../../samdsp/samdsp.h"

class BasePlugin;
class QDoubleSpinBox;
class QCheckBox;

struct DspKalmanBaselinePluginConfig
{
    double err;
    double errI;
    double delta;
};

class DspKalmanBaselinePlugin : public virtual BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    DspKalmanBaselinePluginConfig conf;

    QDoubleSpinBox* errSpinner;
    QDoubleSpinBox* errISpinner;
    QDoubleSpinBox* deltaSpinner;

    QVector<double> outData;

public:
    DspKalmanBaselinePlugin(int _id, QString _name);
    static AbstractPlugin *create (int id, const QString &name, const Attributes &attrs) {
        Q_UNUSED (attrs);
        return new DspKalmanBaselinePlugin (id, name);
    }
    virtual void userProcess();

    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
    void errChanged(double);
    void errIChanged(double);
    void deltaChanged(double);

};

#endif // DSPKALMANBASELINEPLUGIN_H
