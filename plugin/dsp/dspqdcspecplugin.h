/*
Copyright 2011 Bastian Loeher, Roland Wirth

This file is part of GECKO.

GECKO is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

GECKO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
#include <samdsp.h>

class BasePlugin;
class QSpinBox;

struct DspQdcSpecPluginConfig
{
    int width;
    int pointsForBaseline;
    int min;
    int max;
    int nofBins;

    DspQdcSpecPluginConfig() : width(20),
        pointsForBaseline(10),
        min(0),
        max(100),
        nofBins(4096) {}
};


class DspQdcSpecPlugin : public BasePlugin
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

    QVector<double> outData;

    double estimateForBaseline;
    double estimateForAmplitude;

    unsigned int nofLowClip;
    unsigned int nofHiClip;

    bool scheduleResize;

public:
    DspQdcSpecPlugin(int _id, QString _name);
    static AbstractPlugin *create (int _id, const QString &_name, const Attributes &_attrs) {
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
