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

#ifndef BASECACHEPLUGIN_H
#define BASECACHEPLUGIN_H

#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTimer>
#include <QPushButton>
#include <QLineEdit>
#include <QDateTime>

#include "baseplugin.h"
#include "plot2d.h"

class BasePlugin;

struct BaseCachePluginConfig
{
    //! New updates to the cache will be weighted with this weight
    double inputWeight;
    //! The cache data will be normalized to a scale of [0:1]
    bool normalize;
    //! If this flag is set, the cache data are taken from a file on disk
    bool useStoredData;
    //! If this flag is set, cache data is completely updated on every refresh
    bool useInputWeight;
    //! This fileName is used to access the stored data
    QString fileName;
};

/*! Convenience base class for creating cache plugins.
 *  This class provides for easily creating cache plugins, e.g. histograms.
 *  \todo More Doc!
 */
class BaseCachePlugin : public BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    virtual void resetData();
    void setFileName(QString);

    int msecsToTimeout;
    bool scheduleReset;

    BaseCachePluginConfig conf;
    QDateTime lastRead;

    plot2d* plot;
    QPushButton* previewButton;
    QPushButton* resetButton;
    QPushButton* fileNameButton;

    QSpinBox* updateSpeedSpinner;
    QDoubleSpinBox* inputWeightSpinner;

    QCheckBox* normalizeCheck;
    QCheckBox* useFileCheck;
    QCheckBox* useInputWeightCheck;

    QLineEdit* fileNameEdit;

    QTimer* halfSecondTimer;

public:
    BaseCachePlugin(int _id, QString _name, QWidget* _parent = 0);
    virtual ~BaseCachePlugin();

    virtual void userProcess() = 0;
    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

    virtual AbstractPlugin::Group getPluginGroup () { return AbstractPlugin::GroupCache; }

public slots:
    void setTimerTimeout(int msecs);

    virtual void normalizeChanged(bool);
    virtual void useFileChanged(bool);
    virtual void useInputWeightChanged(bool);
    virtual void inputWeightChanged(double);
    virtual void previewButtonClicked();
    virtual void resetButtonClicked();
    virtual void fileNameButtonClicked();
};

#endif // BASECACHEPLUGIN_H
