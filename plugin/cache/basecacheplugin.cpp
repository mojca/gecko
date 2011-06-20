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

#include "basecacheplugin.h"
#include "runmanager.h"
#include "pluginconnectorqueued.h"

#include <QGridLayout>
#include <QLabel>
#include <QSettings>

BaseCachePlugin::BaseCachePlugin(int _id, QString _name, QWidget* _parent)
        : BasePlugin(_id, _name, _parent),
        msecsToTimeout(500),
        scheduleReset(true)
{
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"in"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::out,"fileOut"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::out,"out"));

    //std::cout << "Instantiated BaseCachePlugin" << std::endl;
}

void BaseCachePlugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        previewButton = new QPushButton(tr("Show..."));
        resetButton = new QPushButton(tr("Reset"));
        fileNameButton = new QPushButton(tr("..."));

        fileNameEdit = new QLineEdit(conf.fileName);
        fileNameEdit->setReadOnly(true);

        normalizeCheck = new QCheckBox(tr("Normalize"));
        normalizeCheck->setChecked(false);

        useFileCheck = new QCheckBox(tr("Use file"));
        useFileCheck->setChecked(false);

        useInputWeightCheck = new QCheckBox(tr("Use input weight"));
        useInputWeightCheck->setChecked(true);

        plot = new plot2d(0,QSize(320,240),0);
        plot->setWindowTitle(getName());

        QLabel* updateSpeedLabel = new QLabel(tr("Update Speed"));
        QLabel* inputWeightLabel = new QLabel(tr("Input Weight"));
        QLabel* fileNameLabel     = new QLabel(tr("File name:"));

        updateSpeedSpinner = new QSpinBox();
        updateSpeedSpinner->setMinimum(100);
        updateSpeedSpinner->setMaximum(5000);
        updateSpeedSpinner->setSingleStep(100);
        updateSpeedSpinner->setValue(msecsToTimeout);

        inputWeightSpinner = new QDoubleSpinBox();
        inputWeightSpinner->setMinimum(0.001);
        inputWeightSpinner->setMaximum(1.0);
        inputWeightSpinner->setSingleStep(0.001);
        inputWeightSpinner->setDecimals(3);

        connect(previewButton,SIGNAL(clicked()),this,SLOT(previewButtonClicked()));
        connect(resetButton,SIGNAL(clicked()),this,SLOT(resetButtonClicked()));
        connect(updateSpeedSpinner,SIGNAL(valueChanged(int)),this,SLOT(setTimerTimeout(int)));
        connect(inputWeightSpinner,SIGNAL(valueChanged(double)),this,SLOT(inputWeightChanged(double)));
        connect(normalizeCheck,SIGNAL(toggled(bool)), this,SLOT(normalizeChanged(bool)));
        connect(useFileCheck,SIGNAL(toggled(bool)), this,SLOT(useFileChanged(bool)));
        connect(useInputWeightCheck,SIGNAL(toggled(bool)), this,SLOT(useInputWeightChanged(bool)));
        connect(fileNameButton,SIGNAL(clicked()),this,SLOT(fileNameButtonClicked()));

        cl->addWidget(normalizeCheck,1,0,1,2);
	cl->addWidget(useInputWeightCheck,1,2,1,2);

        cl->addWidget(previewButton,0,0,1,2);
        cl->addWidget(resetButton,  0,2,1,2);

        cl->addWidget(inputWeightLabel  ,2,0,1,1);
        cl->addWidget(inputWeightSpinner,2,1,1,1);
        cl->addWidget(updateSpeedLabel  ,2,2,1,1);
        cl->addWidget(updateSpeedSpinner,2,3,1,1);

        QWidget* container2 = new QWidget();
        {
            QGridLayout* cl2 = new QGridLayout;

            cl2->addWidget(fileNameLabel,     0,0,1,1);
            cl2->addWidget(fileNameEdit,      0,1,1,1);
            cl2->addWidget(fileNameButton,    0,2,1,1);
            cl2->addWidget(useFileCheck,      0,3,1,1);

            cl->addWidget(container2,3,0,1,4);
            container2->setLayout(cl2);
        }

        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

BaseCachePlugin::~BaseCachePlugin()
{
    plot->close();
    delete plot;
    plot = NULL;
}

void BaseCachePlugin::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
        set = "inputWeight";    if(settings->contains(set)) conf.inputWeight = settings->value(set).toDouble();
        set = "normalize";      if(settings->contains(set)) conf.normalize = settings->value(set).toBool();
        set = "useStoredData";  if(settings->contains(set)) conf.useStoredData = settings->value(set).toBool();
        set = "useInputWeight"; if(settings->contains(set)) conf.useInputWeight = settings->value(set).toBool();
        set = "fileName";       if(settings->contains(set)) conf.fileName = settings->value(set).toString();
        set = "plotGeometry";   if(settings->contains(set)) plot->restoreGeometry(settings->value(set).toByteArray());
        set = "plotVisible";    if(settings->contains(set)) if(settings->value(set).toBool()) plot->show();
    settings->endGroup();

    inputWeightSpinner->setValue(conf.inputWeight);
    normalizeCheck->setChecked(conf.normalize);
    useFileCheck->setChecked(conf.useStoredData);
    useInputWeightCheck->setChecked(conf.useInputWeight);
    fileNameEdit->setText(conf.fileName);
}

void BaseCachePlugin::saveSettings(QSettings* settings)
{
    if(settings == NULL)
    {
        std::cout << getName().toStdString() << ": no settings file" << std::endl;
        return;
    }
    else
    {
        std::cout << getName().toStdString() << " saving settings...";
        settings->beginGroup(getName());
            settings->setValue("inputWeight",conf.inputWeight);
            settings->setValue("normalize",conf.normalize);
            settings->setValue("useStoredData",conf.useStoredData);
            settings->setValue("useInputWeight",conf.useInputWeight);
            settings->setValue("fileName",conf.fileName);
            settings->setValue("plotGeometry",plot->saveGeometry());
            plot->isVisible() ? settings->setValue("plotVisible",true) : settings->setValue("plotVisible",false);
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void BaseCachePlugin::previewButtonClicked()
{
    if(plot->isHidden())
    {
        plot->show();
    }
    else
    {
        plot->hide();
    }
}

void BaseCachePlugin::resetButtonClicked()
{
    resetData();
}

void BaseCachePlugin::fileNameButtonClicked()
{
    setFileName(QFileDialog::getOpenFileName(this,"Load cache data...","","Data files (*.dat)"));
}

void BaseCachePlugin::setFileName(QString _fileName)
{
    if(_fileName.isEmpty()) return;
    conf.fileName = _fileName;
    fileNameEdit->setText(conf.fileName);
}

void BaseCachePlugin::normalizeChanged(bool newValue)
{
    conf.normalize = newValue;
}

void BaseCachePlugin::useFileChanged(bool newValue)
{
    conf.useStoredData = newValue;
    if(newValue == true) lastRead.setTime_t(0);
}

void BaseCachePlugin::useInputWeightChanged(bool newValue)
{
    conf.useInputWeight = newValue;
}

void BaseCachePlugin::inputWeightChanged(double newValue)
{
    conf.inputWeight = newValue;
}

void BaseCachePlugin::setTimerTimeout(int msecs)
{
    msecsToTimeout = msecs;
    halfSecondTimer->setInterval(msecsToTimeout);
}

void BaseCachePlugin::resetData()
{
    scheduleReset = true;
}
