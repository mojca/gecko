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

#ifndef CAEN820UI_H
#define CAEN820UI_H

#include "caen820module.h"
#include "baseui.h"

#include <QVector>

class QCheckBox;
class QComboBox;
class QSpinBox;
class QLabel;
class QPushButton;
class QTimer;

class Caen820UI : public BaseUI
{
    Q_OBJECT
public:
    explicit Caen820UI(Caen820Module *m);

    void createUI ();
    void applySettings ();

private slots:
    void updateChannels (int);
    void updateHdrEn (bool);
    void updateShortData (bool);
    void updateAutoRst (bool);
    void updateDwellTime (int);
    void updateAcqMode (int);

    void startMonitor ();
    void stopMonitor ();
    void updateMonitor ();
    void resetCounters ();

private:
    Caen820Module *module_;

    QCheckBox *boxShortData;
    QCheckBox *boxHdrEnable;
    QCheckBox *clearMeb;
    QCheckBox *boxAutoRst;

    QVector<QCheckBox *> boxChEn;

    QSpinBox *sbDwellTime;
    QComboBox *cbAcqMode;

    QVector<QLabel *> lblChMon;
    QPushButton *btnStartStop;
    QPushButton *btnCounterReset;
    QTimer *monitorTimer;
};

#endif // CAEN820UI_H
