#ifndef DSPCALFILTERPLUGIN_H
#define DSPCALFILTERPLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include <QDoubleSpinBox>
#include "baseplugin.h"
#include "samdsp.h"

class BasePlugin;

struct DspCalFilterPluginConfig
{
    int width;
    int shift;
    double attenuation;
};

class DspCalFilterPlugin : public virtual BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    DspCalFilterPluginConfig conf;

    QSpinBox* widthSpinner;
    QSpinBox* shiftSpinner;
    QDoubleSpinBox* attenuationSpinner;

public:
    DspCalFilterPlugin(int _id, QString _name);
    static AbstractPlugin *create (int _id, const QString &_name, const Attributes &_attrs) {
        Q_UNUSED (_attrs);
        return new DspCalFilterPlugin (_id, _name);
    }

    virtual void userProcess();

    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
    void shiftChanged();
    void widthChanged();
    void attenuationChanged();

};


#endif // DSPCALFILTERPLUGIN_H
