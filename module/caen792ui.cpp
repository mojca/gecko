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

#include "caen792ui.h"
#include "caen792module.h"

Caen792UI::Caen792UI(Caen792Module* _module, bool _isqdc)
    : BaseUI()
    , module (_module)
    , isqdc (_isqdc)
{
    blockSlots = false;
    createUI();

    std::cout << "Instantiated Caen792 UI" << std::endl;
}

Caen792UI::~Caen792UI()
{

}

void Caen792UI::createUI()
{
    QGridLayout* l = new QGridLayout;
    QGridLayout* boxL = new QGridLayout;

    QGroupBox* box = new QGroupBox;
    box->setTitle(module->getName () + " Settings");

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

QWidget* Caen792UI::createButtons()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout();

    statusUpdateButton = new QPushButton(tr("Update"));
    connect(statusUpdateButton,SIGNAL(clicked()),this,SLOT(statusUpdateClicked()));

    configureButton = new QPushButton(tr("Configure"));
    connect(configureButton,SIGNAL(clicked()),this,SLOT(configureClicked()));

    QLabel* status1Label = new QLabel(tr("Status1:"));
    status1Edit = new QLineEdit(tr("0x%1").arg(module->getStatus1 (),8,16,QChar('0')));
    status1Edit->setReadOnly(true);
    QLabel* status2Label = new QLabel(tr("Status2:"));
    status2Edit = new QLineEdit(tr("0x%1").arg(module->getStatus2 (),8,16,QChar('0')));
    status2Edit->setReadOnly(true);

    QLabel* addressLabel = new QLabel(tr("Base Address:"));
    baseAddressEdit = new QLineEdit(tr("0x%1").arg(module->getConfig ()->base_addr,8,16,QChar('0')));
    baseAddressEdit->setReadOnly(true);

    QLabel* evcntrLabel = new QLabel(tr("Event Counter:"));
    evCntrEdit = new QLineEdit(tr("%1").arg(module->getEventCount (),16,10,QChar('0')));
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

QWidget* Caen792UI::createTabs()
{
    QTabWidget* tabs = new QTabWidget();

    tabs->addTab(createDevCtrlTab(),tr("Dev Ctrl"));
    tabs->addTab(createSettingsTab(),tr("Settings"));
    tabs->addTab(createThresholdsTab(),tr("Thresholds"));
    tabs->addTab(createIrqTab(),tr("IRQ"));
    tabs->addTab(createInfoTab(),tr("Info"));

    return tabs;
}

QWidget* Caen792UI::createDevCtrlTab()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout;

    l->addWidget(createDeviceControls(),0,0,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Caen792UI::createSettingsTab()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout;

    l->addWidget(createSettings1Controls(),0,0,1,1);
    l->addWidget(createSettings2Controls(),1,0,1,1);
    l->addWidget(createSettings3Controls(),2,0,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Caen792UI::createInfoTab()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout;

    l->addWidget(createInfoControls(),0,0,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Caen792UI::createThresholdsTab()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout;

    l->addWidget(createThresholdsControls(),0,0,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Caen792UI::createIrqTab()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout;

    l->addWidget(createInterruptControls(),0,0,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Caen792UI::createDeviceControls()
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

QWidget* Caen792UI::createThresholdsControls()
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

QWidget* Caen792UI::createInterruptControls()
{
    QWidget *box = new QWidget(this);
    QGridLayout *l = new QGridLayout();
    l->setMargin(0);

    QLabel *irqVectorLabel = new QLabel(tr("IRQ vector:"));
    irqVectorEdit = new QLineEdit(tr("0x%1").arg(module->getConfig ()->irq_vector,8,16,QChar('0')));

    QLabel *irqLevelLabel = new QLabel(tr("IRQ level:"));
    irqLevelSpinner = new QSpinBox();
    irqLevelSpinner->setMaximum(7);
    irqLevelSpinner->setValue(module->getConfig ()->irq_level);

    QLabel *nofEventLabel = new QLabel(tr("Events for IRQ:"));
    nofEventSpinner = new QSpinBox();
    nofEventSpinner->setMaximum(512);
    nofEventSpinner->setValue(module->getConfig ()->ev_trg);

    l->addWidget(irqLevelLabel,0,0,1,1);
    l->addWidget(irqLevelSpinner,0,1,1,1);
    l->addWidget(irqVectorLabel,1,0,1,1);
    l->addWidget(irqVectorEdit,1,1,1,1);
    l->addWidget(nofEventLabel,2,0,1,1);
    l->addWidget(nofEventSpinner,2,1,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Caen792UI::createInfoControls()
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
    l->addWidget(infoUpdateButton);

    box->setLayout(l);
    return box;
}

QWidget* Caen792UI::createSettings1Controls()
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

QWidget* Caen792UI::createSettings2Controls()
{
    QWidget *box = new QWidget(this);
    QGridLayout *l = new QGridLayout();
    l->setMargin(0);

    ovRangeBox = new QCheckBox(tr(     "Suppress Over Range"));
    lowThrBox = new QCheckBox(tr(      "Suppress Low Thr"));
    sldEnableBox = new QCheckBox(tr(   "Use Sliding Scale"));
    autoIncrBox = new QCheckBox(tr(    "Auto Increment"));
    sldSubEnableBox = new QCheckBox(tr("Slide Subtract En"));
    allTriggerBox = new QCheckBox(tr(  "All triggers En"));
    emptyProgBox = new QCheckBox(tr(   "Write empty events"));
    offlineBox = new QCheckBox(tr(     "Offline"));
    hiresThrBox = new QCheckBox (tr (  "High resolution thresholds"));

    connect(ovRangeBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));
    connect(lowThrBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));
    connect(sldEnableBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));
    connect(autoIncrBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));
    connect(sldSubEnableBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));
    connect(allTriggerBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));
    connect(emptyProgBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));
    connect(offlineBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));
    connect(hiresThrBox,SIGNAL(toggled(bool)),this,SLOT(settings2Changed()));

    l->addWidget(ovRangeBox,     0,0,1,1);
    l->addWidget(lowThrBox,      0,1,1,1);
    l->addWidget(offlineBox,     0,2,1,1);
    l->addWidget(autoIncrBox,    1,0,1,1);
    l->addWidget(allTriggerBox,  1,1,1,1);
    l->addWidget(emptyProgBox,   1,2,1,1);
    l->addWidget(sldEnableBox,   2,0,1,1);
    l->addWidget(sldSubEnableBox,2,1,1,1);
    l->addWidget(hiresThrBox,    2,2,1,1);

    if (!isqdc) {
        stopModeBox = new QCheckBox (tr ("Common Stop"));
        connect (stopModeBox, SIGNAL(toggled(bool)), SLOT(settings2Changed()));
        l->addWidget(stopModeBox, 3,0,1,1);
    }

    box->setLayout(l);
    return box;
}

QWidget* Caen792UI::createSettings3Controls()
{
    QWidget *box = new QWidget(this);
    QGridLayout *l = new QGridLayout();
    l->setMargin(0);

    QLabel *crateNoLabel = new QLabel(tr("Crate number:"));
    crateNumberSpinner = new QSpinBox();
    crateNumberSpinner->setMaximum(255);
    crateNumberSpinner->setValue(module->getConfig ()->cratenumber);

    QLabel *ipedLabel = new QLabel (tr (isqdc ? "Pedestal Current:" : "Full Scale Range:"));
    ipedSpinner = new QSpinBox ();
    ipedSpinner->setMaximum (255);
    ipedSpinner->setValue (isqdc ? module->getConfig ()->i_ped : module->getConfig ()->fsr);

    QLabel *fclrLabel = new QLabel (tr ("Fastclear Window:"));
    fclrSpinner = new QSpinBox ();
    fclrSpinner->setMaximum (1023);
    fclrSpinner->setValue (module->getConfig ()->fastclear);

    QLabel *slideconstLabel = new QLabel(tr("Sliding Constant:"));
    slideconstSpinner = new QSpinBox();
    slideconstSpinner->setMaximum(255);
    slideconstSpinner->setValue(module->getConfig ()->slideconst);

    connect (crateNumberSpinner, SIGNAL(valueChanged(int)), SLOT(crateNoChanged()));
    connect (ipedSpinner, SIGNAL(valueChanged(int)), SLOT(ipedChanged()));
    connect (fclrSpinner, SIGNAL(valueChanged(int)), SLOT(fclrChanged()));
    connect (slideconstSpinner, SIGNAL(valueChanged(int)), SLOT(slideconstChanged()));

    l->addWidget(crateNoLabel,0,0,1,1);
    l->addWidget(crateNumberSpinner,0,1,1,1);
    l->addWidget(ipedLabel,1,0,1,1);
    l->addWidget(ipedSpinner,1,1,1,1);
    l->addWidget(fclrLabel,2,0,1,1);
    l->addWidget(fclrSpinner,2,1,1,1);
    l->addWidget(slideconstLabel,3,0,1,1);
    l->addWidget(slideconstSpinner,3,1,1,1);

    box->setLayout(l);
    return box;
}

void Caen792UI::thresholdsChanged() {
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

void Caen792UI::settings1Changed()
{
    if (blockSlots) return;
    module->getConfig ()->block_end     = blockEndBox->isChecked();
    module->getConfig ()->align64       = align64Box->isChecked();
    module->getConfig ()->berr_enable   = berrEnableBox->isChecked();
    module->getConfig ()->program_reset = progResetBox->isChecked();
}

void Caen792UI::settings2Changed()
{
    if (blockSlots) return;
    module->getConfig ()->overRangeSuppressionEnabled = ovRangeBox->isChecked();
    module->getConfig ()->zeroSuppressionEnabled      = lowThrBox->isChecked();
    module->getConfig ()->zeroSuppressionThr	  = hiresThrBox->isChecked();
    module->getConfig ()->slidingScaleEnabled         = sldEnableBox->isChecked();
    module->getConfig ()->autoIncrementEnabled        = autoIncrBox->isChecked();
    module->getConfig ()->slideSubEnabled             = sldSubEnableBox->isChecked();
    module->getConfig ()->alwaysIncrementEventCounter = allTriggerBox->isChecked();
    module->getConfig ()->emptyEventWriteEnabled      = emptyProgBox->isChecked();
    module->getConfig ()->offline                     = offlineBox->isChecked();

    if (!isqdc)
        module->getConfig()->stop_mode                = stopModeBox->isChecked ();
}

void Caen792UI::crateNoChanged()
{
    module->getConfig ()->cratenumber = crateNumberSpinner->value();
}

void Caen792UI::nofEventsChanged()
{
    module->getConfig ()->ev_trg = nofEventSpinner->value();
}

void Caen792UI::ipedChanged () {
    if (isqdc)
        module->getConfig ()->i_ped = ipedSpinner->value ();
    else
        module->getConfig ()->fsr = ipedSpinner->value ();
}

void Caen792UI::fclrChanged () {
    module->getConfig ()->fastclear = fclrSpinner->value ();
}

void Caen792UI::slideconstChanged () {
    module->getConfig ()->slideconst = slideconstSpinner->value ();
}

void Caen792UI::irqLevelChanged()
{
    module->getConfig ()->irq_level = irqLevelSpinner->value();
}

void Caen792UI::irqVectorChanged()
{
    module->getConfig ()->irq_vector = irqVectorEdit->text().toShort();
}

void Caen792UI::infoUpdateClicked() {
    uint16_t fw = module->getInfo ();
    firmwareEdit->setText (QString ("%1.%2").arg ((fw >> 8) & 0xFF, 2, 16, QChar ('0')).arg (fw & 0xFF, 2, 16, QChar ('0')));
}

void Caen792UI::applySettings()
{
    blockSlots = true;
    baseAddressEdit->setText(tr("%1").arg(module->getConfig ()->base_addr,8,16,QChar('0')));

    blockEndBox->setChecked(module->getConfig ()->block_end);
    align64Box->setChecked(module->getConfig ()->align64);
    berrEnableBox->setChecked(module->getConfig ()->berr_enable);
    progResetBox->setChecked(module->getConfig ()->program_reset);

    ovRangeBox->setChecked(module->getConfig ()->overRangeSuppressionEnabled);
    lowThrBox->setChecked(module->getConfig ()->zeroSuppressionEnabled);
    hiresThrBox->setChecked(module->getConfig ()->zeroSuppressionThr);
    sldEnableBox->setChecked(module->getConfig ()->slidingScaleEnabled);
    autoIncrBox->setChecked(module->getConfig ()->autoIncrementEnabled);
    sldSubEnableBox->setChecked(module->getConfig ()->slideSubEnabled);
    allTriggerBox->setChecked(module->getConfig ()->alwaysIncrementEventCounter);
    emptyProgBox->setChecked(module->getConfig ()->emptyEventWriteEnabled);
    offlineBox->setChecked(module->getConfig ()->offline);

    if (!isqdc)
        stopModeBox->setChecked(module->getConfig ()->stop_mode);

    crateNumberSpinner->setValue(module->getConfig ()->cratenumber);
    if (isqdc)
        ipedSpinner->setValue(module->getConfig ()->i_ped);
    else
        ipedSpinner->setValue(module->getConfig ()->fsr);
    fclrSpinner->setValue(module->getConfig ()->fastclear);
    slideconstSpinner->setValue(module->getConfig ()->slideconst);
    irqLevelSpinner->setValue(module->getConfig ()->irq_level);
    nofEventSpinner->setValue(module->getConfig ()->ev_trg);

    for(int ch = 0; ch < 32; ++ch) {
        //printf("Settings value: ch:%d, val:%d, kill:%d\n",ch,
        //       module->getConfig()->thresholds[ch],
        //       (int)module->getConfig()->killChannel[ch]);
        thresholdSpinner[ch]->setValue(module->getConfig()->thresholds[ch]);
        if(module->getConfig()->killChannel[ch] == false) killChannelBox[ch]->setChecked(true);
        else killChannelBox[ch]->setChecked(false);
    }

    irqVectorEdit->setText(tr("%1").arg(module->getConfig ()->irq_vector,8,16,QChar('0')));
    blockSlots = false;
}

void Caen792UI::statusUpdateClicked()
{
    module->readStatus ();
    status1Edit->setText(tr("0x%1").arg(module->getStatus1 (),8,16,QChar('0')));
    status2Edit->setText(tr("0x%1").arg(module->getStatus2 (),8,16,QChar('0')));
    evCntrEdit->setText(tr("%1").arg(module->getEventCount (),16,10,QChar('0')));
}

void Caen792UI::configureClicked()
{
    module->reset();
    module->configure();
}

void Caen792UI::testConversionClicked()
{
    for(int i = 0; i < nofTestConversionBox->value(); i++)
    {
        uint32_t data[34];
        uint32_t rd;
        module->singleShot(data,&rd);
    }
}

void Caen792UI::dataResetClicked() { module->dataReset ();}
