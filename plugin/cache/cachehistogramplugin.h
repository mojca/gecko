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

#ifndef CACHEHISTOGRAMPLUGIN_H
#define CACHEHISTOGRAMPLUGIN_H

#include <QWidget>
#include <vector>

#include "basecacheplugin.h"

class QComboBox;
class BasePlugin;
class QLabel;

struct CacheHistogramPluginConfig
{
    double inputWeight;
    bool normalize;
    bool autoreset;
    bool autosave;
    double xmin, xmax;
    int nofBins, ymax;
    int autosaveInt;
    int autoresetInt;

    CacheHistogramPluginConfig()
        : inputWeight(1.), normalize(false), autoreset(false), autosave(true),
        xmin(0.),xmax(4095.),nofBins(4096),ymax(99),autosaveInt(60),autoresetInt(60)
    {}
};

class CacheHistogramPlugin : public virtual BaseCachePlugin
{
    Q_OBJECT

protected:
    QVector<double> cache;

    double binWidth;

    CacheHistogramPluginConfig conf;
    void recalculateBinWidth();

    QDoubleSpinBox* xminSpinner;
    QDoubleSpinBox* xmaxSpinner;
    QSpinBox* ymaxSpinner;
    QComboBox* nofBinsBox;

    QCheckBox* autosaveCheck;
    QCheckBox* autoresetCheck;

    QSpinBox* autosaveSpinner;
    QSpinBox* autoresetSpinner;

    QTimer* writeToFileTimer;
    QTimer* resetTimer;

    QLabel* numCountsLabel;

    bool writeToFile;
    int fileCount;

    uint64_t nofCounts;

    virtual void createSettings(QGridLayout*);

public:
    CacheHistogramPlugin(int _id, QString _name);
    static AbstractPlugin *create (int _id, const QString &_name, const Attributes &_attrs) {
        Q_UNUSED (_attrs);
        BaseCachePlugin* bcp = new CacheHistogramPlugin (_id, _name);
        return dynamic_cast<BasePlugin*>(bcp);
    }

    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);
    virtual void userProcess();

    virtual void runStartingEvent();

public slots:
    void xminChanged(double);
    void xmaxChanged(double);
    void ymaxChanged(int);
    void nofBinsChanged(int);
    virtual void normalizeChanged(bool);
    virtual void inputWeightChanged(double);
    void autoresetChanged(bool);
    void autosaveChanged(bool);
    void autoresetIntChanged(int);
    void autosaveIntChanged(int);

    void scheduleWriteToFile();
    void scheduleResetHistogram();
    void updateVisuals();
};

#endif // CACHEHISTOGRAMPLUGIN_H
