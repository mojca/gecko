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

#ifndef DSPCFDPLUGIN_H
#define DSPCFDPLUGIN_H

#include "baseplugin.h"

class DspCfdConfig;
class QSpinBox;
class QCheckBox;
class QDoubleSpinBox;

class DspCfdPlugin : public BasePlugin
{
    Q_OBJECT
public:
    explicit DspCfdPlugin(int _id, QString _name);
    static AbstractPlugin *create (int _id, const QString &_name, const Attributes &) {
        return new DspCfdPlugin (_id, _name);
    }

    void createSettings (QGridLayout *);

    void saveSettings (QSettings *);
    void applySettings (QSettings *);

protected slots:
    void userProcess();

public slots:
    void fractionChanged (double);
    void negativeChanged (bool);
    void thresholdChanged (int);
    void holdoffChanged (int);
    void baselineChanged (int);

private:
    DspCfdConfig *conf;
    QDoubleSpinBox *fractionSpinner_;
    QCheckBox *negativeBox_;
    QSpinBox *thresholdSpinner_;
    QSpinBox *holdoffSpinner_;
    QSpinBox *baselineSpinner_;

};

#endif // DSPCFDPLUGIN_H
