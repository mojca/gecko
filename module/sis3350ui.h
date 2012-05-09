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

#ifndef SIS3350UI_H
#define SIS3350UI_H

#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QMutex>
#include <QPushButton>
#include <QSignalMapper>
#include <QTabWidget>
#include <QTimer>
#include "viewport.h"

#include "sis3350module.h"
#include "baseui.h"

class Sis3350Module;

class Sis3350UI : public BaseUI
{
    Q_OBJECT

public:
    Sis3350UI(Sis3350Module* _module);
    ~Sis3350UI();

    void applySettings();

public slots:
    void armClicked();
    void disarmClicked();
    void resetClicked();
    void triggerClicked();
    void timestampClearClicked();
    void singleShotClicked();
    void freeRunningButtonClicked();
    void previewButtonClicked();
    void gainChanged(int ch);
    void offChanged(int ch);
    void thrChanged(int ch);
    void gateChanged(int ch);
    void peakChanged(int ch);
    void enableChanged(int ch);
    void firChanged(int ch);
    void gtChanged(int ch);
    void nofEventsChanged();
    void sampleLengthChanged();
    void nofMultiEventsChanged();
    void pretriggerChanged();
    void postProcessChanged();
    void clockChanged();
    void clockSourceChanged(int);
    void interruptSourceChanged(int);
    void modeChanged(int);
    void timerTimeout();
    void armTimer();
    void trgOutClicked (bool);

protected:
    bool timerArmed;

    void createUI();

    QWidget* createButtons();
    QWidget* createTabs();

    QWidget* createDevCtrlTab();
    QWidget* createTriggerTab();
    QWidget* createGainTab();
    QWidget* createRunTab();
    QWidget* createClockTab();
    QWidget* createIrqTab();

    QWidget* createInputGainControls();
    QWidget* createInputOffsetControls();
    QWidget* createPretriggerControls();

    QWidget* createTriggerThresholdControls();
    QWidget* createTriggerEnableControls();
    QWidget* createTriggerFIRControls();
    QWidget* createTriggerEdgeControls();
    QWidget* createTriggerSourceControls();
    QWidget* createTriggerPeakControls();
    QWidget* createTriggerGateControls();

    QWidget* createDeviceControls();
    QWidget* createRunControls();
    QWidget* createClockSelect();
    QWidget* createClockControls();
    QWidget* createInterruptSourceControls();

    QComboBox* modeBox;
    QComboBox *clockSourceBox;
    QComboBox *interruptSourceBox;

    QCheckBox* trgOutCheckbox;
    QCheckBox* autoUpdateCheckbox;
    QPushButton* singleShot;
    QPushButton* freeRunningButton;
    QPushButton* previewButton;

    QPushButton* armButton;
    QPushButton* disarmButton;
    QPushButton* resetButton;
    QPushButton* triggerButton;
    QPushButton* timestampClearButton;

    QSignalMapper *mapper;

    QTimer *freeRunnerTimer;
    QMutex timerArmMutex;

    Viewport *viewport;

    QCheckBox *ppCheck;
    QList<QSpinBox*> *gains;
    QList<QSpinBox*> *thresholds;
    QList<QSpinBox*> *gates;
    QList<QSpinBox*> *peaks;
    QList<QSpinBox*> *offsets;
    QList<QCheckBox*> *triggerEnable;
    QList<QCheckBox*> *triggerFir;
    QList<QCheckBox*> *triggerGt;

    QSpinBox *nofEventSpinner;
    QSpinBox *nofMultiEventSpinner;
    QSpinBox *pretriggerDelaySpinner;
    QSpinBox *sampleLengthSpinner;
    QSpinBox *clock1Spinner;
    QSpinBox *clock2Spinner;

    QLabel *clockFrequencyLabel;
    QLineEdit *baseAddressEdit;

private:
    Sis3350Module *module;
    QString name;
};

#endif // SIS3350UI_H
