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

#ifndef DSPTRIGGERLMAXPLUGIN_H
#define DSPTRIGGERLMAXPLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include <QCheckBox>
#include "baseplugin.h"
#include <samdsp.h>

class BasePlugin;
class QSpinBox;
class QDoubleSpinBox;

struct DspTriggerLMAXPluginConfig
{
    double threshold;
    int holdoff;
    bool positive;
};

class DspTriggerLMAXPlugin : public BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    DspTriggerLMAXPluginConfig conf;

    QDoubleSpinBox* thresholdSpinner;
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
