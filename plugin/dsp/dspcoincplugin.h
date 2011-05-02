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
