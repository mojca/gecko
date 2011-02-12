#ifndef DSPADCPLUGIN_H
#define DSPADCPLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include "baseplugin.h"
#include "../../samdsp/samdsp.h"

class BasePlugin;
class QSpinBox;
class QCheckBox;

struct DspAdcPluginConfig
{
};

class DspAdcPlugin : public virtual BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    DspAdcPluginConfig conf;

    vector<double> outData;

public:
    DspAdcPlugin(int _id, QString _name);
    static AbstractPlugin *create (int id, const QString &name, const Attributes &attrs) {
        Q_UNUSED (attrs);
        return new DspAdcPlugin (id, name);
    }
    virtual void userProcess();

    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:

};

#endif // DSPADCPLUGIN_H
