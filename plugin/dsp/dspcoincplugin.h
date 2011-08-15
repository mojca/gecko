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

#ifndef DSPCOINCPLUGIN_H
#define DSPCOINCPLUGIN_H

#include "baseplugin.h"

struct ConfigDspCoinc;

class QComboBox;
class QSpinBox;
class QCheckBox;
class QLabel;

class DspCoincPlugin : public BasePlugin {
    Q_OBJECT
public:
    static AbstractPlugin* create (int id, const QString& name, const Attributes& attrs) {
        return new DspCoincPlugin (id, name, attrs);
    }

    static AttributeMap attributeMap ();

    AttributeMap getAttributeMap () const { return attributeMap (); }
    Attributes getAttributes () const { return attrs_; }

    void saveSettings (QSettings *s);
    void applySettings (QSettings *s);

    void runStartingEvent();

public slots:
    void userProcess ();

    void gateOpenerChanged (int);
    void delayChanged (int);
    void widthChanged (int);
    void timestampChanged (bool);

    void updateCoincData ();

protected:
    void createSettings (QGridLayout *);

private:
    DspCoincPlugin (int id, QString name, Attributes attrs);

private:
    Attributes attrs_;
    ConfigDspCoinc *conf_;

    int ntriggers_;
    int ndata_;

    QComboBox *boxGateOpener_;
    QSpinBox  *sbDelay_;
    QSpinBox  *sbWidth_;
    QCheckBox *cbTimestamps_;
    QLabel    *lblCoinc;

    uint64_t nCoinc;
    uint64_t nNoCoinc;
};

#endif // DSPCOINCPLUGIN_H
