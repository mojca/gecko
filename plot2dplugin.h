#ifndef PLOT2DPLUGIN_H
#define PLOT2DPLUGIN_H

#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QFile>
#include <QFileDialog>
#include <QSpinBox>
#include <QTextStream>
#include <QTimer>
#include <QDateTime>
#include <iostream>
#include <vector>

#include "baseplugin.h"
#include "plot2d.h"

class BasePlugin;

class Plot2DPlugin : public virtual BasePlugin
{
    Q_OBJECT

protected:

    virtual void createSettings(QGridLayout*);

    plot2d* plot;
    QPushButton* previewButton;
    QCheckBox* useExternalBox;
    QCheckBox* zoomExtendsBox;
    QSpinBox* xminSpinner;
    QSpinBox* yminSpinner;
    QSpinBox* xmaxSpinner;
    QSpinBox* ymaxSpinner;
    QSpinBox* updateSpeedSpinner;

    QTimer* halfSecondTimer;
    int msecsToTimeout;

    std::vector<std::vector<double> > outData;

public:
    Plot2DPlugin(int _id, QString _name);
    static AbstractPlugin *create (int _id, const QString &_name, const Attributes &_attrs) {
        Q_UNUSED (_attrs);
        return new Plot2DPlugin (_id, _name);
    }
    ~Plot2DPlugin();

    virtual void userProcess();
    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
    void previewButtonClicked();
    void resetData(uint,uint);
    void boundsChanged();
    void setTimerTimeout(int);

};

#endif // PLOT2DPLUGIN_H
