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

#ifndef SIS3302UI_GAMMA_V1410_H
#define SIS3302UI_GAMMA_V1410_H

#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QMap>
#include <QMessageBox>
#include <QMutex>
#include <QPushButton>
#include <QSignalMapper>
#include <QSpinBox>
#include <QStringList>
#include <QTabWidget>
#include <QTimer>
#include "plot2d.h"
#include "hexspinbox.h"
#include "geckoui.h"

#include "sis3302module_gamma_v1410.h"
#include "baseui.h"

class Sis3302V1410Module;

class Sis3302V1410UI : public virtual BaseUI
{
    Q_OBJECT

public:
    static const uint8_t NOF_CHANNELS = 8;

    Sis3302V1410UI(Sis3302V1410Module* _module);
    ~Sis3302V1410UI();

    void applySettings();

protected:
    Sis3302V1410Module* module;

    // UI
    QTabWidget tabs;    // Tabs widget
    GeckoUiFactory uif; // UI generator factory
    QStringList tn; // Tab names
    QStringList gn; // Group names
    QStringList wn; // WidgetNames

    void createUI(); // Has to be implemented

    bool applyingSettings;
    bool previewRunning;

    // Preview window
    void createPreviewUI();
    QPushButton* startStopPreviewButton;
    QWidget previewWindow;
    plot2d* previewCh[NOF_CHANNELS];
    plot2d* previewEnergy[NOF_CHANNELS];
    QVector<double> previewData[NOF_CHANNELS];
    QVector<double> previewEnergyData[NOF_CHANNELS];
    QTimer* previewTimer;

public slots:
    void uiInput(QString _name);
    void clicked_start_button();
    void clicked_arm_button();
    void clicked_disarm_button();
    void clicked_reset_button();
    void clicked_clear_button();
    void clicked_configure_button();
    void clicked_previewButton();
    void clicked_startStopPreviewButton();
    void clicked_singleshot_button();
    void clicked_update_firmware_button();
    void timeout_previewTimer();
    void updatePreview();
};

#endif // SIS3302UI_GAMMA_V1410_H
