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

#ifndef DSPAMPSPECPLUGIN_H
#define DSPAMPSPECPLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include <QPushButton>
#include <QLineEdit>
#include "baseplugin.h"
#include <samdsp.h>

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
