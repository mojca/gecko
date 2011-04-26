#ifndef DSPTIMEFILTERPLUGIN_H
#define DSPTIMEFILTERPLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include "baseplugin.h"
#include "../../samdsp/samdsp.h"

class BasePlugin;
class QSpinBox;

struct DspTimeFilterPluginConfig
{
    int width;
    int spacing;
};

class DspTimeFilterPlugin : public virtual BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    DspTimeFilterPluginConfig conf;

    QSpinBox* widthSpinner;
    QSpinBox* spacingSpinner;

    std::vector<double>* curData;
    std::vector<double> outData;

public:
    DspTimeFilterPlugin(int _id, QString _name);
    static AbstractPlugin *create (int _id, const QString &_name, const Attributes &_attrs) {
        Q_UNUSED (_attrs);
        return new DspTimeFilterPlugin (_id, _name);
    }

    virtual void userProcess();

    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
    void spacingChanged();
    void widthChanged();

};

#endif // DSPTIMEFILTERPLUGIN_H
