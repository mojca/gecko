#ifndef DSPAMPSPECPLUGIN_H
#define DSPAMPSPECPLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include <QPushButton>
#include <QLineEdit>
#include "baseplugin.h"
#include "samdsp.h"

class BasePlugin;
class QSpinBox;

struct DspAmpSpecPluginConfig
{
    int width;
    int pointsForBaseline;
};

class DspAmpSpecPlugin : public virtual BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    DspAmpSpecPluginConfig conf;

    QSpinBox* widthSpinner;
    QSpinBox* baselineSpinner;

    QPushButton* resetButton;

    QLineEdit* lowClip;
    QLineEdit* hiClip;

    QVector<double>* outData;

    double estimateForBaseline;
    double estimateForAmplitude;

    unsigned int nofLowClip;
    unsigned int nofHiClip;

public:
    DspAmpSpecPlugin(int _id, QString _name);
    static AbstractPlugin *create (int _id, const QString &_name, const Attributes &_attrs) {
        Q_UNUSED (_attrs);
        return new DspAmpSpecPlugin (_id, _name);
    }

    virtual void userProcess();

    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
    void widthChanged();
    void baselineChanged();
    void resetSpectra();
};

#endif // DSPAMPSPECPLUGIN_H
