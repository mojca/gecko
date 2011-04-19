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

    void createSettings (QGridLayout *);

protected slots:
    void userProcess();

public slots:
    void fractionChanged (double);
    void delayChanged (int);
    void thresholdChanged (int);
    void holdoffChanged (int);

private:
    DspCfdConfig *conf;
    QDoubleSpinBox *fractionSpinner_;
    QSpinBox *delaySpinner_;
    QSpinBox *thresholdSpinner_;
    QSpinBox *holdoffSpinner_;

};

#endif // DSPCFDPLUGIN_H
