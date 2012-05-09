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

#ifndef SIS3100UI_H
#define SIS3100UI_H

#include <QGroupBox>
#include <QLabel>
#include <QListView>
#include <QGridLayout>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QString>

#include "sis3100module.h"
#include "baseui.h"

class Sis3100Module;

class Sis3100UI : public BaseUI
{
    Q_OBJECT
public:
    enum AddrMode {A24, A32};
    enum DataMode {D16, D32};

public:
    Sis3100UI(Sis3100Module* _module);
    ~Sis3100UI() {}
    virtual void createUI();
    virtual void applySettings() {}

private:
    void moduleOpened ();
    void moduleClosed ();

    friend class Sis3100Module;

public slots:
    void openCloseButtonClicked();
    void readButtonClicked();
    void writeButtonClicked();
    void resetButtonClicked();
    void outputText(QString);
    void addrModeChanged(int);
    void dataModeChanged(int);

signals:
    void deviceOpened();
    void deviceClosed();

private:

    QWidget* createButtons();
    QWidget* createStatusView();
    QWidget* createVmeControl();

    bool deviceOpen;
    QPushButton *opencloseButton;
    QPushButton *resetButton;

    QPushButton *readButton;
    QPushButton *writeButton;

    QComboBox *addrModeSpinner;
    QComboBox *dataModeSpinner;

    QLabel *addrLabel;
    QLabel *dataLabel;

    QLineEdit *addrEdit;
    QLineEdit *dataEdit;

    QTextEdit *statusViewTextEdit;

    QWidget* vmeControl;

    Sis3100Module *module;
    QString name;

    AddrMode addrmode;
    DataMode datamode;
};

#endif // SIS3100UI_H
