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

#ifndef MESYTECMADC32UI_H
#define MESYTECMADC32UI_H

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

#include "mesytec_madc_32_v2.h"
#include "baseui.h"

class MesytecMadc32Module;

class MesytecMadc32UI : public virtual BaseUI
{
    Q_OBJECT

public:
    MesytecMadc32UI(MesytecMadc32Module* _module);
    ~MesytecMadc32UI();

    void applySettings();

protected:
    MesytecMadc32Module* module;

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
    QPushButton* singleShotPreviewButton;
    QWidget previewWindow;
    plot2d* previewCh[MADC32V2_NUM_CHANNELS];
    QLabel* energyValueDisplay[MADC32V2_NUM_CHANNELS];
    QLabel* timestampDisplay[MADC32V2_NUM_CHANNELS];
    QLabel* resolutionDisplay[MADC32V2_NUM_CHANNELS];
    QLabel* flagDisplay[MADC32V2_NUM_CHANNELS];
    QLabel* moduleIdDisplay[MADC32V2_NUM_CHANNELS];
    QVector<double> previewData[MADC32V2_NUM_CHANNELS];
    QTimer* previewTimer;

public slots:
    void uiInput(QString _name);
    void clicked_start_button();
    void clicked_stop_button();
    void clicked_reset_button();
    void clicked_fifo_reset_button();
    void clicked_readout_reset_button();
    void clicked_configure_button();
    void clicked_previewButton();
    void clicked_startStopPreviewButton();
    void clicked_singleshot_button();
    void clicked_update_firmware_button();
    void timeout_previewTimer();
    void updatePreview();
};

#endif // MESYTECMADC32UI_H
