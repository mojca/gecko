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

#ifndef RAWWRITESIS3302v1410PLUGIN_H
#define RAWWRITESIS3302v1410PLUGIN_H

#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <iostream>
#include <vector>

#include "baseplugin.h"

class BasePlugin;

class RawWriteSis3302v1410Plugin : public BasePlugin
{
    Q_OBJECT

protected:
    QString filePath;
    QString fileName;
    QString prefix;

    QLineEdit* filePathLineEdit;
    QPushButton* filePathButton;
    QCheckBox* intervalModeCheckBox;
    QSpinBox* intervalSpinBox;
    QSpinBox* nofEventsSpinBox;
    QLabel* writingDataLabel;

    virtual void createSettings(QGridLayout*);

    bool intervalMode;
    int interval_minutes;
    int interval_number;
    int nof_events;

    int nof_events_written;
    QTime last_interval_time;
    QTime next_interval_time;
    QTime now;

    bool settings_changed;

    QFile* file;

public:
    RawWriteSis3302v1410Plugin(int _id, QString _name);

    static AbstractPlugin *create (int id, const QString &name, const Attributes &attrs) {
        Q_UNUSED (attrs);
        return new RawWriteSis3302v1410Plugin (id, name);
    }

    QString getFilePath() { return filePath; }

    void setFilePath(QString _filePath);

    virtual void userProcess();
    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
    void filePathButtonClicked();
    void settingsChanged();
};

#endif // RAWWRITESIS3302v1410PLUGIN_H
