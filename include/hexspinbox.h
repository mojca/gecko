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

#ifndef HEXSPINBOX_H
#define HEXSPINBOX_H

#include <QSpinBox>
/*! The HexSpinBox widget displays a hexadecimal value inside a QSpinBox widget.
 *  It provides the means for validating hexadecimal input and conversion from and to
 *  hex strings.
 */
class HexSpinBox : public QSpinBox {
public:
    HexSpinBox (QWidget *p)
    : QSpinBox (p)
    , v_ (new QRegExpValidator (QRegExp ("[0-9a-fA-F]+"), this))
    {}
protected:
    virtual QString textFromValue (int value) const {
        return QString::number (value, 16).toUpper ();
    }

    virtual int valueFromText (const QString &text) const {
        QString in = text;
        if (in.startsWith (prefix ()))
            in.remove(0, prefix ().length ());
        if (in.endsWith (suffix ()))
            in.chop (suffix ().length ());

        return in.toInt (NULL, 16);
    }

    virtual QValidator::State validate (QString &input, int &pos) const {
        QString in = input;
        if (in.startsWith (prefix ()))
            in.remove(0, prefix ().length ());
        if (in.endsWith (suffix ()))
            in.chop (suffix ().length ());

        QValidator::State state = v_->validate (in, pos);
        pos = input.length () - suffix ().length ();
        return state;
    }
private:
    QRegExpValidator *v_;
};

#endif // HEXSPINBOX_H
