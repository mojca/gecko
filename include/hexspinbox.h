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
