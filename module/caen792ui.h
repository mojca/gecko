#ifndef CAEN792UI_H
#define CAEN792UI_H

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

#include "baseui.h"

class Caen792Module;

class Caen792UI : public virtual BaseUI
{
	Q_OBJECT

public:
    Caen792UI(Caen792Module* _module, bool _isqdc);
	~Caen792UI();

	void applySettings();

public slots:
        void dataResetClicked();
	void evcntResetClicked(){;}
	void softResetClicked(){;}
	void fastClearClicked(){;}
	void incrEventClicked(){;}
	void incrOffsetClicked(){;}
	void testConversionClicked();
	void statusUpdateClicked();
	void configureClicked();
	void infoUpdateClicked();

	void irqLevelChanged();
	void irqVectorChanged();
	void nofEventsChanged();
	void settings1Changed();
	void settings2Changed();
	void crateNoChanged();
	void ipedChanged();
	void fclrChanged();
        void slideconstChanged();

private:
	Caen792Module* module;
    bool isqdc;

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
	QSpinBox* ipedSpinner;
	QSpinBox* fclrSpinner;
        QSpinBox* slideconstSpinner;

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
	QCheckBox* hiresThrBox;
	QCheckBox* sldEnableBox;
	QCheckBox* autoIncrBox;
	QCheckBox* sldSubEnableBox;
	QCheckBox* allTriggerBox;
	QCheckBox* emptyProgBox;
	QCheckBox* offlineBox;
    QCheckBox* stopModeBox;

	QTextEdit* romInfoEdit;
	QLineEdit* firmwareEdit;

        bool blockSlots;
};

#endif // CAEN792UI_H
