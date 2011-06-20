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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QSignalMapper>
#include <QSpinBox>
#include <QList>
#include <QCheckBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QTabWidget>
#include <QTimer>
#include "sis3150control.h"
#include "sis3350control.h"
#include "sis3150usbui.h"
#include "sis3350ui.h"
#include "viewport.h"
#include "postprocesswindow.h"
#include "configmanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Sis3150control *sis3150;
    Sis3350control *sis;

public slots:
    void connectDevicesTo3150();

protected:
    void createUI();
    void setupPostProcessWindow();
    void closeEvent(QCloseEvent *ev);
    ConfigManager *c;


    QTabWidget *tabs;
    QTimer *freeRunnerTimer;

    Sis3150usbUI *sis3150usbUI;
    Sis3350UI    *sis3350UI;

    PostProcessWindow *ppw;


};

#endif // MAINWINDOW_H
