#ifndef DSPCFDPLUGIN_H
#define DSPCFDPLUGIN_H

#include "baseplugin.h"

class DspCfdConfig;
class QSpinBox;
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
    void delayChanged (int);
    void thresholdChanged (int);
    void holdoffChanged (int);
    void baselineChanged (int);

private:
    DspCfdConfig *conf;
    QDoubleSpinBox *fractionSpinner_;
    QSpinBox *delaySpinner_;
    QSpinBox *thresholdSpinner_;
    QSpinBox *holdoffSpinner_;
    QSpinBox *baselineSpinner_;

};

#endif // DSPCFDPLUGIN_H
