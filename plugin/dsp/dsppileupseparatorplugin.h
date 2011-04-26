#ifndef DSPPILEUPSEPARATORPLUGIN_H
#define DSPPILEUPSEPARATORPLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include "baseplugin.h"
#include "../../samdsp/samdsp.h"

class BasePlugin;
class QSpinBox;
class QCheckBox;

struct DspPileupSeparatorPluginConfig
{
    int left;
    int right;
    bool invert;
};

class DspPileupSeparatorPlugin : public BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    DspPileupSeparatorPluginConfig conf;

    QSpinBox* leftSpinner;
    QSpinBox* rightSpinner;
    QCheckBox* invertBox;

public:
    DspPileupSeparatorPlugin(int _id, QString _name);
    static AbstractPlugin *create (int id, const QString &name, const Attributes &attrs) {
        Q_UNUSED (attrs);
        return new DspPileupSeparatorPlugin (id, name);
    }
    virtual void userProcess();

    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
    void leftChanged();
    void rightChanged();
    void invertChanged(bool);


};

#endif // DSPPILEUPSEPARATORPLUGIN_H
