#ifndef DSPCLIPPINGDETECTORPLUGIN_H
#define DSPCLIPPINGDETECTORPLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include "baseplugin.h"
#include "../../samdsp/samdsp.h"

class BasePlugin;
class QSpinBox;

struct DspClippingDetectorPluginConfig
{
    int low;
    int high;
};

class DspClippingDetectorPlugin : public virtual BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    DspClippingDetectorPluginConfig conf;

    QSpinBox* lowSpinner;
    QSpinBox* highSpinner;

    QVector<double> clip;

public:
    DspClippingDetectorPlugin(int _id, QString _name);
    static AbstractPlugin *create (int _id, const QString &_name, const Attributes &_attrs) {
        Q_UNUSED (_attrs);
        return new DspClippingDetectorPlugin (_id, _name);
    }

    virtual void userProcess();

    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
    void lowChanged();
    void highChanged();

};


#endif // DSPCLIPPINGDETECTORPLUGIN_H
