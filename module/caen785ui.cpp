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

#include "caen785ui.h"

Caen785UI::Caen785UI(Caen785Module* _module)
{
    module = _module;
    blockSlots = false;
    createUI();

    std::cout << "Instantiated Caen785 UI" << std::endl;
}

Caen785UI::~Caen785UI()
{

}

void Caen785UI::createUI()
{
    QGridLayout* l = new QGridLayout;
    QGridLayout* boxL = new QGridLayout;

    QGroupBox* box = new QGroupBox;
    box->setTitle(module->getName() + " Settings");

    // Module specific code here

    QWidget* buttons = createButtons();
    QWidget* tabs = createTabs();

    boxL->addWidget(buttons,0,0,1,1);
    boxL->addWidget(tabs,1,0,1,1);

    // End

    l->addWidget(box,0,0,1,1);
    box->setLayout(boxL);
    this->setLayout(l);
}

QWidget* Caen785UI::createButtons()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout();

    statusUpdateButton = new QPushButton(tr("Update"));
    connect(statusUpdateButton,SIGNAL(clicked()),this,SLOT(statusUpdateClicked()));

    configureButton = new QPushButton(tr("Configure"));
    connect(configureButton,SIGNAL(clicked()),this,SLOT(configureClicked()));

    QLabel* status1Label = new QLabel(tr("Status1:"));
    status1Edit = new QLineEdit(tr("0x%1").arg(module->status1,8,16,QChar('0')));
    status1Edit->setReadOnly(true);
    QLabel* status2Label = new QLabel(tr("Status2:"));
    status2Edit = new QLineEdit(tr("0x%1").arg(module->status2,8,16,QChar('0')));
    status2Edit->setReadOnly(true);

    QLabel* addressLabel = new QLabel(tr("Base Address:"));
    baseAddressEdit = new QLineEdit(tr("0x%1").arg(module->conf.base_addr,8,16,QChar('0')));
    baseAddressEdit->setReadOnly(true);

    QLabel* evcntrLabel = new QLabel(tr("Event Counter:"));
    evCntrEdit = new QLineEdit(tr("%1").arg(module->evcntr,16,10,QChar('0')));
    evCntrEdit->setReadOnly(true);

    l->addWidget(status1Label,0,0,1,1);
    l->addWidget(status2Label,1,0,1,1);
    l->addWidget(status1Edit,0,1,1,1);
    l->addWidget(status2Edit,1,1,1,1);
    l->addWidget(statusUpdateButton,0,2,1,1);
    l->addWidget(configureButton,1,2,1,1);
    l->addWidget(addressLabel,0,3,1,1);
    l->addWidget(evcntrLabel,1,3,1,1);
    l->addWidget(baseAddressEdit,0,4,1,1);
    l->addWidget(evCntrEdit,1,4,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Caen785UI::createTabs()
{
    QTabWidget* tabs = new QTabWidget();

    tabs->addTab(createDevCtrlTab(),tr("Dev Ctrl"));
    tabs->addTab(createSettingsTab(),tr("Settings"));
    tabs->addTab(createThresholdsTab(),tr("Thresholds"));
    tabs->addTab(createIrqTab(),tr("IRQ"));
    tabs->addTab(createInfoTab(),tr("Info"));

    return tabs;
}

QWidget* Caen785UI::createDevCtrlTab()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout;

    l->addWidget(createDeviceControls(),0,0,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Caen785UI::createSettingsTab()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout;

    l->addWidget(createSettings1Controls(),0,0,1,1);
    l->addWidget(createSettings2Controls(),1,0,1,1);
    l->addWidget(createSettings3Controls(),2,0,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Caen785UI::createInfoTab()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout;

    l->addWidget(createInfoControls(),0,0,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Caen785UI::createThresholdsTab()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout;

    l->addWidget(createThresholdsControls(),0,0,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Caen785UI::createIrqTab()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout;

    l->addWidget(createInterruptControls(),0,0,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Caen785UI::createDeviceControls()
{
    QWidget *box = new QWidget(this);
    QGridLayout *l = new QGridLayout();
    l->setMargin(0);

    dataResetButton = new QPushButton(tr("Data Reset"));
    evcntResetButton = new QPushButton(tr("Event Counter Reset"));
    softResetButton = new QPushButton(tr("Soft Reset"));
    fastClearButton = new QPushButton(tr("Fast Clear"));
    incrEventButton = new QPushButton(tr("Increase ev. counter"));
    incrOffsetButton = new QPushButton(tr("Increase ev. offset"));
    testConversionButton = new QPushButton(tr("Test Conversion"));

    nofTestConversionBox = new QSpinBox();
    nofTestConversionBox->setValue(1);
    nofTestConversionBox->setMaximum(1000000);

    connect(dataResetButton,SIGNAL(clicked()),this,SLOT(dataResetClicked()));
    connect(evcntResetButton,SIGNAL(clicked()),this,SLOT(evcntResetClicked()));
    connect(softResetButton,SIGNAL(clicked()),this,SLOT(softResetClicked()));
    connect(fastClearButton,SIGNAL(clicked()),this,SLOT(fastClearClicked()));
    connect(incrEventButton,SIGNAL(clicked()),this,SLOT(incrEventClicked()));
    connect(incrOffsetButton,SIGNAL(clicked()),this,SLOT(incrOffsetClicked()));
    connect(testConversionButton,SIGNAL(clicked()),this,SLOT(testConversionClicked()));

    l->addWidget(dataResetButton,0,0,1,1);
    l->addWidget(evcntResetButton,0,1,1,1);
    l->addWidget(softResetButton,1,0,1,1);
    l->addWidget(fastClearButton,1,1,1,1);
    l->addWidget(incrEventButton,2,0,1,1);
    l->addWidget(incrOffsetButton,2,1,1,1);
    l->addWidget(testConversionButton,3,0,1,1);
    l->addWidget(nofTestConversionBox,3,1,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Caen785UI::createInterruptControls()
{
    QWidget *box = new QWidget(this);
    QGridLayout *l = new QGridLayout();
    l->setMargin(0);

    QLabel *irqVectorLabel = new QLabel(tr("IRQ vector:"));
    irqVectorEdit = new QLineEdit(tr("0x%1").arg(module->conf.irq_vector,8,16,QChar('0')));

    QLabel *irqLevelLabel = new QLabel(tr("IRQ level:"));
    irqLevelSpinner = new QSpinBox();
    irqLevelSpinner->setMaximum(7);
    irqLevelSpinner->setValue(module->conf.irq_level);

    QLabel *nofEventLabel = new QLabel(tr("Events for IRQ:"));
    nofEventSpinner = new QSpinBox();
    nofEventSpinner->setMaximum(512);
    nofEventSpinner->setValue(module->conf.nof_events);

    l->addWidget(irqLevelLabel,0,0,1,1);
    l->addWidget(irqLevelSpinner,0,1,1,1);
    l->addWidget(irqVectorLabel,1,0,1,1);
    l->addWidget(irqVectorEdit,1,1,1,1);
    l->addWidget(nofEventLabel,2,0,1,1);
    l->addWidget(nofEventSpinner,2,1,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Caen785UI::createInfoControls()
{
    QWidget *box = new QWidget(this);
    QVBoxLayout *l = new QVBoxLayout();
    l->setMargin(0);

    QLabel *fwLabel = new QLabel(tr("Firmware Info:"));
    firmwareEdit = new QLineEdit();
    firmwareEdit->setReadOnly(true);

    QLabel *romLabel = new QLabel(tr("Rom Info:"));
    romInfoEdit = new QTextEdit();
    romInfoEdit->setReadOnly(true);

    infoUpdateButton = new QPushButton(tr("Update info"));

    connect(infoUpdateButton,SIGNAL(clicked()),this,SLOT(infoUpdateClicked()));

    l->addWidget(fwLabel);
    l->addWidget(firmwareEdit);
    l->addWidget(romLabel);
    l->addWidget(romInfoEdit);

    box->setLayout(l);
    return box;
}

QWidget* Caen785UI::createThresholdsControls()
{
    QWidget *box = new QWidget(this);
    QGridLayout *l = new QGridLayout();
    l->setMargin(0);
    l->setSpacing(0);

    for(int ch = 0; ch < 32; ch+=2) {
        QWidget* w = new QWidget();
        {
            QHBoxLayout* h = new QHBoxLayout();
            h->setMargin(0);
            h->setSpacing(0);
            killChannelBox[ch] = new QCheckBox(tr("Ch %1:").arg(ch));
            thresholdSpinner[ch] = new QSpinBox();
            thresholdSpinner[ch]->setMaximum(0xffff);

            killChannelBox[ch+1] = new QCheckBox(tr("Ch %1:").arg(ch+1));
            thresholdSpinner[ch+1] = new QSpinBox();
            thresholdSpinner[ch+1]->setMaximum(0xffff);

            h->addWidget(killChannelBox[ch]);
            h->addWidget(thresholdSpinner[ch]);
            h->addWidget(killChannelBox[ch+1]);
            h->addWidget(thresholdSpinner[ch+1]);

            connect(killChannelBox[ch],SIGNAL(toggled(bool)),this,SLOT(thresholdsChanged()));
            connect(thresholdSpinner[ch],SIGNAL(valueChanged(int)),this,SLOT(thresholdsChanged()));
            connect(killChannelBox[ch+1],SIGNAL(toggled(bool)),this,SLOT(thresholdsChanged()));
            connect(thresholdSpinner[ch+1],SIGNAL(valueChanged(int)),this,SLOT(thresholdsChanged()));
            w->setLayout(h);
        }
        l->addWidget(w);
    }

    box->setLayout(l);
    return box;
}

QWidget* Caen785UI::createSettings1Controls()
{
    QWidget *box = new QWidget(this);
    QGridLayout *l = new QGridLayout();
    l->setMargin(0);

    blockEndBox = new QCheckBox(tr("Block End"));
    progResetBox = new QCheckBox(tr("Prog Reset"));
    berrEnableBox = new QCheckBox(tr("Berr En"));
    align64Box = new QCheckBox(tr("Align 64"));

    l->addWidget(blockEndBox,0,0,1,1);
    l->addWidget(berrEnableBox,0,1,1,1);
    l->addWidget(progResetBox,1,0,1,1);
    l->addWidget(align64Box,1,1,1,1);

    connect(blockEndBox,SIGNAL(toggled(bool)),this,SLOT(settings1Changed()));
    connect(berrEnableBox,SIGNAL(toggled(bool)),this,SLOT(settings1Changed()));
    connect(progResetBox,SIGNAL(toggled(bool)),this,SLOT(settings1Changed()));
    connect(align64Box,SIGNAL(toggled(bool)),this,SLOT(settings1Changed()));

    box->setLayout(l);
    return box;
}

QWidget* Caen785UI::createSettings2Controls()
{
    QWidget *box = new QWidget(this);
    QGridLayout *l = new QGridLayout();
    l->setMargin(0);

    ovRangeBox = new QCheckBox(tr(     "Over Range"));
    lowThrBox = new QCheckBox(tr(      "Enable Thresholds"));
    sldEnableBox = new QCheckBox(tr(   "Use Sliding Scale"));
    autoIncrBox = new QCheckBox(tr(    "Auto Increment"));
    sldSubEnableBox = new QCheckBox(tr("Slide Subtract En"));
    allTriggerBox = new QCheckBox(tr(  "All triggers En"));
    emptyProgBox = new QCheckBox(tr(   "Write empty events"));
    offlineBox = new QCheckBox(tr(     "Offline"));
    highThrResBox = new QCheckBox(tr(  "High resolution thresholds"));

    connect(ovRangeBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));
    connect(lowThrBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));
    connect(sldEnableBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));
    connect(autoIncrBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));
    connect(sldSubEnableBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));
    connect(allTriggerBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));
    connect(emptyProgBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));
    connect(offlineBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));
    connect(highThrResBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));

    l->addWidget(ovRangeBox,     0,0,1,1);
    l->addWidget(lowThrBox,      0,1,1,1);
    l->addWidget(offlineBox,     0,2,1,1);
    l->addWidget(autoIncrBox,    1,0,1,1);
    l->addWidget(allTriggerBox,  1,1,1,1);
    l->addWidget(emptyProgBox,   1,2,1,1);
    l->addWidget(sldEnableBox,   2,0,1,1);
    l->addWidget(sldSubEnableBox,2,1,1,1);
    l->addWidget(highThrResBox,  2,2,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Caen785UI::createSettings3Controls()
{
    QWidget *box = new QWidget(this);
    QGridLayout *l = new QGridLayout();
    l->setMargin(0);

    QLabel *crateNoLabel = new QLabel(tr("Crate number:"));
    crateNumberSpinner = new QSpinBox();
    crateNumberSpinner->setMaximum(255);
    crateNumberSpinner->setValue(module->conf.cratenumber);

    QLabel *slidingLabel = new QLabel(tr("Sliding Scale Value:"));
    slidingScaleSpinner = new QSpinBox();
    slidingScaleSpinner->setMaximum(255);
    slidingScaleSpinner->setValue(module->conf.slide_constant);

    l->addWidget(crateNoLabel,0,0,1,1);
    l->addWidget(crateNumberSpinner,0,1,1,1);
    l->addWidget(slidingLabel,1,0,1,1);
    l->addWidget(slidingScaleSpinner,1,1,1,1);

    box->setLayout(l);
    return box;
}

void Caen785UI::thresholdsChanged() {
    if (blockSlots) return;
    for(int ch = 0; ch < 32; ++ch) {
        module->getConfig()->thresholds[ch] = thresholdSpinner[ch]->value();
        if(killChannelBox[ch]->isChecked()) {
            module->getConfig()->killChannel[ch] = false;
        } else {
            module->getConfig()->killChannel[ch] = true;
        }
    }
}

void Caen785UI::settings1Changed()
{
    if (blockSlots) return;
    module->conf.block_end     = blockEndBox->isChecked();
    module->conf.align64       = align64Box->isChecked();
    module->conf.berr_enable   = berrEnableBox->isChecked();
    module->conf.program_reset = progResetBox->isChecked();
}

void Caen785UI::settings2Changed()
{
    if (blockSlots) return;
    module->conf.overRangeSuppressionEnabled = ovRangeBox->isChecked();
    module->conf.zeroSuppressionEnabled      = lowThrBox->isChecked();
    module->conf.slidingScaleEnabled         = sldEnableBox->isChecked();
    module->conf.autoIncrementEnabled        = autoIncrBox->isChecked();
    module->conf.slideSubEnabled             = sldSubEnableBox->isChecked();
    module->conf.alwaysIncrementEventCounter = allTriggerBox->isChecked();
    module->conf.emptyEventWriteEnabled      = emptyProgBox->isChecked();
    module->conf.offline                     = offlineBox->isChecked();
    module->conf.zeroSuppressionThr          = highThrResBox->isChecked();
}

void Caen785UI::crateNoChanged()
{
    module->conf.cratenumber = crateNumberSpinner->value();
}

void Caen785UI::slideConstChanged()
{
    module->conf.slide_constant = slidingScaleSpinner->value();
}

void Caen785UI::nofEventsChanged()
{
    module->conf.nof_events = nofEventSpinner->value();
}

void Caen785UI::irqLevelChanged()
{
    module->conf.irq_level = irqLevelSpinner->value();
}

void Caen785UI::irqVectorChanged()
{
    module->conf.irq_vector = irqVectorEdit->text().toShort();
}

void Caen785UI::applySettings()
{
    blockSlots = true;
    baseAddressEdit->setText(tr("%1").arg(module->conf.base_addr,8,16,QChar('0')));

    blockEndBox->setChecked(module->conf.block_end);
    align64Box->setChecked(module->conf.align64);
    berrEnableBox->setChecked(module->conf.berr_enable);
    progResetBox->setChecked(module->conf.program_reset);

    ovRangeBox->setChecked(module->conf.overRangeSuppressionEnabled);
    lowThrBox->setChecked(module->conf.zeroSuppressionEnabled);
    sldEnableBox->setChecked(module->conf.slidingScaleEnabled);
    autoIncrBox->setChecked(module->conf.autoIncrementEnabled);
    sldSubEnableBox->setChecked(module->conf.slideSubEnabled);
    allTriggerBox->setChecked(module->conf.alwaysIncrementEventCounter);
    emptyProgBox->setChecked(module->conf.emptyEventWriteEnabled);
    offlineBox->setChecked(module->conf.offline);
    highThrResBox->setChecked(module->conf.zeroSuppressionThr);

    crateNumberSpinner->setValue(module->conf.cratenumber);
    slidingScaleSpinner->setValue(module->conf.slide_constant);
    irqLevelSpinner->setValue(module->conf.irq_level);
    nofEventSpinner->setValue(module->conf.nof_events);
    slidingScaleSpinner->setValue(module->conf.slide_constant);

    for(int ch = 0; ch < 32; ++ch) {
        //printf("Settings value: ch:%d, val:%d, kill:%d\n",ch,
        //       module->getConfig()->thresholds[ch],
        //       (int)module->getConfig()->killChannel[ch]);
        thresholdSpinner[ch]->setValue(module->getConfig()->thresholds[ch]);
        if(module->getConfig()->killChannel[ch] == false) killChannelBox[ch]->setChecked(true);
        else killChannelBox[ch]->setChecked(false);
    }

    irqVectorEdit->setText(tr("%1").arg(module->conf.irq_vector,8,16,QChar('0')));
    blockSlots = false;
}

void Caen785UI::statusUpdateClicked()
{
    module->readStatus();
    status1Edit->setText(tr("0x%1").arg(module->status1,8,16,QChar('0')));
    status2Edit->setText(tr("0x%1").arg(module->status2,8,16,QChar('0')));
    evCntrEdit->setText(tr("%1").arg(module->evcntr,16,10,QChar('0')));
}

void Caen785UI::configureClicked()
{
    module->reset();
    module->configure();
}

void Caen785UI::testConversionClicked()
{
    for(int i = 0; i < nofTestConversionBox->value(); i++)
    {
        module->singleShot();
    }
}
