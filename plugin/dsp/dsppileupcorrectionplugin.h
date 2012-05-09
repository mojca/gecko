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

#ifndef DSPPILEUPCORRECTIONPLUGIN_H
#define DSPPILEUPCORRECTIONPLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include "baseplugin.h"
#include <samdsp.h>

class BasePlugin;
class QSpinBox;
class QCheckBox;

struct DspPileUpCorrectionPluginConfig
{
    int signalsLeft;
    int signalsRight;
    int samplesLeft;
    int samplesRight;
};

class DspPileUpCorrectionPlugin : public BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    DspPileUpCorrectionPluginConfig conf;

    QSpinBox* signalsLeftSpinner;
    QSpinBox* signalsRightSpinner;
    QSpinBox* samplesLeftSpinner;
    QSpinBox* samplesRightSpinner;

public:
    DspPileUpCorrectionPlugin(int _id, QString _name);
    static AbstractPlugin *create (int id, const QString &name, const Attributes &attrs) {
        Q_UNUSED (attrs);
        return new DspPileUpCorrectionPlugin (id, name);
    }
    virtual void userProcess();

    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
    void signalsLeftChanged(int);
    void signalsRightChanged(int);
    void samplesLeftChanged(int);
    void samplesRightChanged(int);

};

#endif // DSPPILEUPCORRECTIONPLUGIN_H
