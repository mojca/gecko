#ifndef CAEN785UI_H
#define CAEN785UI_H

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
#include <QPushButton>
#include <QTabWidget>
#include <QTextEdit>

#include "caen785module.h"
#include "baseui.h"

class Caen785Module;

class Caen785UI : public virtual BaseUI
{
    Q_OBJECT

public:
    Caen785UI(Caen785Module* _module);
    ~Caen785UI();

    void applySettings();

public slots:
    void dataResetClicked(){;}
    void evcntResetClicked(){;}
    void softResetClicked(){;}
    void fastClearClicked(){;}
    void incrEventClicked(){;}
    void incrOffsetClicked(){;}
    void testConversionClicked();
    void statusUpdateClicked();
    void configureClicked();
    void infoUpdateClicked(){;}

    void irqLevelChanged();
    void irqVectorChanged();
    void nofEventsChanged();
    void settings1Changed();
    void settings2Changed();
    void crateNoChanged();
    void slideConstChanged();

protected:
    Caen785Module* module;

    void createUI();

    QWidget* createButtons();
    QWidget* createTabs();

    QWidget* createDevCtrlTab();
    QWidget* createSettingsTab();
    QWidget* createThresholdsTab(){return 0;}
    QWidget* createIrqTab();
    QWidget* createInfoTab();

    QWidget* createDeviceControls();
    QWidget* createSettings1Controls();
    QWidget* createSettings2Controls();
    QWidget* createSettings3Controls();
    QWidget* createThresholdsControls();
    QWidget* createInterruptControls();
    QWidget* createInfoControls();

    QLineEdit* baseAddressEdit;
    QLineEdit* status1Edit;
    QLineEdit* status2Edit;
    QLineEdit* evCntrEdit;
    QLineEdit* irqVectorEdit;

    QSpinBox* irqLevelSpinner;
    QSpinBox* nofEventSpinner;
    QSpinBox* crateNumberSpinner;
    QSpinBox* slidingScaleSpinner;

    QSpinBox* nofTestConversionBox;

    QPushButton* dataResetButton;
    QPushButton* evcntResetButton;
    QPushButton* softResetButton;
    QPushButton* fastClearButton;
    QPushButton* incrEventButton;
    QPushButton* incrOffsetButton;
    QPushButton* testConversionButton;
    QPushButton* statusUpdateButton;
    QPushButton* configureButton;
    QPushButton* infoUpdateButton;

    QCheckBox* blockEndBox;
    QCheckBox* progResetBox;
    QCheckBox* berrEnableBox;
    QCheckBox* align64Box;

    QCheckBox* ovRangeBox;
    QCheckBox* lowThrBox;
    QCheckBox* sldEnableBox;
    QCheckBox* autoIncrBox;
    QCheckBox* sldSubEnableBox;
    QCheckBox* allTriggerBox;
    QCheckBox* emptyProgBox;
    QCheckBox* offlineBox;
    QCheckBox* highThrResBox;

    QTextEdit* romInfoEdit;
    QLineEdit* firmwareEdit;

    bool blockSlots;
};

#endif // CAEN785UI_H
