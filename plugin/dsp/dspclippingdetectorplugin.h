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

#ifndef DSPCLIPPINGDETECTORPLUGIN_H
#define DSPCLIPPINGDETECTORPLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include "baseplugin.h"
#include <samdsp.h>

class BasePlugin;
class QSpinBox;

struct DspClippingDetectorPluginConfig
{
    int low;
    int high;
};

class DspClippingDetectorPlugin : public virtual BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    DspClippingDetectorPluginConfig conf;

    QSpinBox* lowSpinner;
    QSpinBox* highSpinner;

    QVector<double> clip;

public:
    DspClippingDetectorPlugin(int _id, QString _name);
    static AbstractPlugin *create (int _id, const QString &_name, const Attributes &_attrs) {
        Q_UNUSED (_attrs);
        return new DspClippingDetectorPlugin (_id, _name);
    }

    virtual void userProcess();

    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
    void lowChanged();
    void highChanged();

};


#endif // DSPCLIPPINGDETECTORPLUGIN_H
