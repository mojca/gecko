#ifndef DSPEXTRACTSIGNALPLUGIN_H
#define DSPEXTRACTSIGNALPLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include "baseplugin.h"
#include <samdsp.h>

class BasePlugin;
class QSpinBox;
class QCheckBox;

struct DspExtractSignalPluginConfig
{
    int width;
    int offset;
    bool invert;
};

class DspExtractSignalPlugin : public virtual BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    DspExtractSignalPluginConfig conf;

    QSpinBox* widthSpinner;
    QSpinBox* offsetSpinner;
    QCheckBox* invertBox;

    QVector<double> signal;
    QVector<double> baseline_out;

public:
    DspExtractSignalPlugin(int _id, QString _name);
    static AbstractPlugin *create (int id, const QString &name, const Attributes &attrs) {
        Q_UNUSED (attrs);
        return new DspExtractSignalPlugin (id, name);
    }
    virtual void userProcess();

    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
    void widthChanged();
    void offsetChanged();
    void invertChanged(bool);

};


#endif // DSPEXTRACTSIGNALPLUGIN_H
