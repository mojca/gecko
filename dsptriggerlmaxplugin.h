#ifndef DSPTRIGGERLMAXPLUGIN_H
#define DSPTRIGGERLMAXPLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include <QCheckBox>
#include "baseplugin.h"
#include "../../samdsp/samdsp.h"

class BasePlugin;
class QSpinBox;

struct DspTriggerLMAXPluginConfig
{
    int threshold;
    int holdoff;
    bool positive;
};

class DspTriggerLMAXPlugin : public virtual BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    DspTriggerLMAXPluginConfig conf;

    QSpinBox* thresholdSpinner;
    QSpinBox* holdoffSpinner;
    QCheckBox* polarityBox;

    QVector<double> trigger;

public:
    DspTriggerLMAXPlugin(int _id, QString _name);
    static AbstractPlugin *create (int _id, const QString &_name, const Attributes &_attrs) {
        Q_UNUSED (_attrs);
        return new DspTriggerLMAXPlugin (_id, _name);
    }

    virtual void userProcess();

    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
    void thresholdChanged();
    void holdoffChanged();
    void polarityChanged(bool);
};

#endif // DSPTRIGGERLMAXPLUGIN_H
