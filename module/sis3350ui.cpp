#include "sis3350ui.h"
#include <QMessageBox>

Sis3350UI::Sis3350UI(Sis3350Module* _module)
: module (_module)
, name (_module->getName ())
{
    createUI();

    freeRunnerTimer = new QTimer();
    connect(freeRunnerTimer,SIGNAL(timeout()),this,SLOT(timerTimeout()));
    freeRunnerTimer->setInterval(50);

    timerArmMutex.lock();
    timerArmed = false;
    timerArmMutex.unlock();

    std::cout << "Instantiated Sis3350 UI" << std::endl;
}

Sis3350UI::~Sis3350UI()
{
    viewport->close();
    delete viewport;
    freeRunnerTimer->stop();
    delete freeRunnerTimer;
}

void Sis3350UI::createUI()
{
    QGridLayout* l = new QGridLayout;
    QGridLayout* boxL = new QGridLayout;

    QGroupBox* box = new QGroupBox;
    box->setTitle(name + " Settings");

    // Module specific code here

    QWidget* buttons = createButtons();
    QWidget* tabs = createTabs();

    boxL->addWidget(buttons,0,0,1,1);
    boxL->addWidget(tabs,1,0,1,1);

    viewport = new Viewport(0,module->rblt_data,4,module->conf.sample_length);
    viewport->setWindowTitle(tr("sis3350 Preview"));

    // End

    l->addWidget(box,0,0,1,1);
    box->setLayout(boxL);
    this->setLayout(l);
}

QWidget* Sis3350UI::createButtons()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout();

    QLabel* modeLabel = new QLabel(tr("Mode:"));
    modeBox = new QComboBox();
    modeBox->addItem(tr("Ringbuffer Async"));
    modeBox->addItem(tr("Ringbuffer Sync"));
    modeBox->addItem(tr("Direct Mem Gate Async"));
    modeBox->addItem(tr("Direct Mem Gate Sync"));
    modeBox->addItem(tr("Direct Memory Stop"));
    modeBox->addItem(tr("Direct Memory Start"));
    connect(modeBox,SIGNAL(currentIndexChanged(int)),this,SLOT(modeChanged(int)));

    QLabel* addressLabel = new QLabel(tr("Base Address:"));
    baseAddressEdit = new QLineEdit(tr("%1").arg(module->conf.base_addr,8,16,QChar('0')));
    baseAddressEdit->setReadOnly(true);

    autoUpdateCheckbox = new QCheckBox(tr("Auto update"));
    singleShot = new QPushButton(tr("Update"));
    connect(singleShot,SIGNAL(clicked()),this,SLOT(singleShotClicked()));
    freeRunningButton = new QPushButton(tr("Free Running"));
    freeRunningButton->setCheckable (true);
    connect(freeRunningButton,SIGNAL(clicked()),this,SLOT(freeRunningButtonClicked()));
    previewButton = new QPushButton(tr("Preview..."));
    connect(previewButton,SIGNAL(clicked()),this,SLOT(previewButtonClicked()));

    l->addWidget(modeLabel,0,0,1,1);
    l->addWidget(modeBox,0,1,1,1);
    l->addWidget(addressLabel,0,2,1,1);
    l->addWidget(baseAddressEdit,0,3,1,1);
    l->addWidget(singleShot,1,0,1,1);
    l->addWidget(freeRunningButton,1,1,1,1);
    l->addWidget(previewButton,1,2,1,1);
    l->addWidget(autoUpdateCheckbox,1,3,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createTabs()
{
    QTabWidget* tabs = new QTabWidget();

    tabs->addTab(createDevCtrlTab(),tr("Dev Ctrl"));
    tabs->addTab(createTriggerTab(),tr("Trigger"));
    tabs->addTab(createGainTab(),tr("Gain"));
    tabs->addTab(createRunTab(),tr("Run"));
    tabs->addTab(createClockTab(),tr("Clock"));
    tabs->addTab(createIrqTab(),tr("Irq"));

    return tabs;
}

QWidget* Sis3350UI::createTriggerTab()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout;

    l->addWidget(createTriggerEnableControls(),0,0,1,1);
    l->addWidget(createTriggerThresholdControls(),1,0,1,1);
    l->addWidget(createTriggerFIRControls(),2,0,1,1);
    l->addWidget(createTriggerEdgeControls(),3,0,1,1);
    l->addWidget(createTriggerPeakControls(),4,0,1,1);
    l->addWidget(createTriggerGateControls(),5,0,1,1);
    l->addWidget(createTriggerSourceControls(),6,0,1,1);

    trgOutCheckbox = new QCheckBox (tr ("Enable TRG OUT LEMO connector"), box);
    connect (trgOutCheckbox, SIGNAL(toggled(bool)), SLOT(trgOutClicked(bool)));
    l->addWidget(trgOutCheckbox, 7, 0, 1, 1);

    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createGainTab()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout;

    l->addWidget(createInputGainControls(),0,0,1,1);
    l->addWidget(createInputOffsetControls(),1,0,1,1);
    l->addWidget(createPretriggerControls(),2,0,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createRunTab()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout;

    l->addWidget(createRunControls(),0,0,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createDevCtrlTab()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout;

    l->addWidget(createDeviceControls(),0,0,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createClockTab()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout;

    l->addWidget(createClockSelect(),0,0,1,1);
    l->addWidget(createClockControls(),1,0,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createIrqTab()
{
    QWidget* box = new QWidget();
    QGridLayout* l = new QGridLayout;

    l->addWidget(createInterruptSourceControls(),0,0,1,1);

    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createInputGainControls()
{
    // Gain spinners
    QWidget *box = new QWidget(this);
    QSignalMapper *mapper = new QSignalMapper();
    QHBoxLayout *l = new QHBoxLayout();
    l->setMargin(0);

    gains = new QList<QSpinBox*>();
    QLabel *label = new QLabel(tr("Gain:"),this);

    l->addWidget(label);

    for(int i=0; i<4; i++)
    {
        QSpinBox *gain = new QSpinBox(this);
        gain->setMinimum(0);
        gain->setMaximum(255);
        gain->setValue(module->conf.variable_gain[i]);
        gains->append(gain);
        connect(gain,SIGNAL(valueChanged(int)),mapper,SLOT(map()));
        mapper->setMapping(gain,i);
        l->addWidget(gain);
    }
    connect(mapper,SIGNAL(mapped(int)),this,SLOT(gainChanged(int)));
    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createInputOffsetControls()
{
    // Trigger Offset spinners
    QWidget *box = new QWidget(this);
    QSignalMapper *mapper = new QSignalMapper();
    QHBoxLayout *l = new QHBoxLayout();
    l->setMargin(0);

    offsets = new QList<QSpinBox*>();
    QLabel *label = new QLabel(tr("Offset:"),this);

    l->addWidget(label);

    for(int i=0; i<4; i++)
    {
        QSpinBox *off = new QSpinBox(this);
        off->setMinimum(0);
        off->setMaximum(65535);
        off->setAccelerated(true);
        off->setSingleStep(10);
        off->setValue(module->conf.adc_offset[i]);
        connect(off,SIGNAL(valueChanged(int)),mapper,SLOT(map()));
        mapper->setMapping(off,i);
        l->addWidget(off);
        offsets->append(off);
    }
    connect(mapper,SIGNAL(mapped(int)),this,SLOT(offChanged(int)));
    box->setLayout(l);
    return box;
}

//QWidget* Sis3350UI::createTemplate()
//{
//    QWidget *box = new QWidget(this);
//    QSignalMapper *mapper = new QSignalMapper();
//    QHBoxLayout *l = new QHBoxLayout();
//    l->setMargin(0);
//
//
//
//    box->setLayout(l);
//    return box;
//}

QWidget* Sis3350UI::createTriggerSourceControls()
{
    QWidget *box = new QWidget(this);
    QHBoxLayout *l = new QHBoxLayout();
    l->setMargin(0);



    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createInterruptSourceControls()
{
    QWidget *box = new QWidget(this);
    QHBoxLayout *l = new QHBoxLayout();
    l->setMargin(0);

    QLabel *irqLabel = new QLabel(tr("IRQ source:"));
    interruptSourceBox = new QComboBox();
    interruptSourceBox->addItem(tr("Reached End Address Threshold (level sensitive)"));
    interruptSourceBox->addItem(tr("Reached End Address Threshold (edge sensitive)"));
    interruptSourceBox->addItem(tr("End of Event"));
    interruptSourceBox->addItem(tr("End of Multi Event"));
    connect(interruptSourceBox,SIGNAL(currentIndexChanged(int)),this,SLOT(interruptSourceChanged(int)));

    l->addWidget(irqLabel);
    l->addWidget(interruptSourceBox);

    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createDeviceControls()
{
    QWidget *box = new QWidget(this);
    QVBoxLayout *l = new QVBoxLayout();
    l->setMargin(0);

    armButton = new QPushButton(tr("Arm"));
    disarmButton = new QPushButton(tr("Disarm"));
    resetButton = new QPushButton(tr("Reset"));
    triggerButton = new QPushButton(tr("Trigger"));
    timestampClearButton = new QPushButton(tr("Clear Timestamp"));

    connect(armButton,SIGNAL(clicked()),this,SLOT(armClicked()));
    connect(disarmButton,SIGNAL(clicked()),this,SLOT(disarmClicked()));
    connect(resetButton,SIGNAL(clicked()),this,SLOT(resetClicked()));
    connect(triggerButton,SIGNAL(clicked()),this,SLOT(triggerClicked()));
    connect(timestampClearButton,SIGNAL(clicked()),this,SLOT(timestampClearClicked()));

    l->addWidget(armButton);
    l->addWidget(disarmButton);
    l->addWidget(resetButton);
    l->addWidget(triggerButton);
    l->addWidget(timestampClearButton);
    l->addStretch(10);


    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createClockSelect()
{
    QWidget *box = new QWidget(this);
    QHBoxLayout *l = new QHBoxLayout();
    l->setMargin(0);

    QLabel *clockLabel = new QLabel(tr("Clock source:"));
    clockSourceBox = new QComboBox();
    clockSourceBox->addItem(tr("Internal 100 MHz"));
    clockSourceBox->addItem(tr("Internal Synthesizer"));
    clockSourceBox->addItem(tr("External BNC"));
    clockSourceBox->addItem(tr("External LVDS"));
    connect(clockSourceBox,SIGNAL(currentIndexChanged(int)),this,SLOT(clockSourceChanged(int)));

    l->addWidget(clockLabel);
    l->addWidget(clockSourceBox);

    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createClockControls()
{
    // Clock spinners
    // Clock frequency f = 25 MHz * clock1 / 2**clock2
    QWidget *box = new QWidget(this);
    QHBoxLayout *l = new QHBoxLayout();
    l->setMargin(0);

    clock1Spinner = new QSpinBox(this);
    clock2Spinner = new QSpinBox(this);
    QLabel *clockLabel = new QLabel(tr("Clock:"));
    clockFrequencyLabel = new QLabel(tr("500 MHz"));

    clock1Spinner->setRange(7,20);
    clock2Spinner->setRange(0,3);
    clock1Spinner->setValue(module->conf.clock1);    // default = 20 -> 500 MHz
    clock2Spinner->setValue(module->conf.clock2);

    connect(clock1Spinner,SIGNAL(valueChanged(int)),this,SLOT(clockChanged()));
    connect(clock2Spinner,SIGNAL(valueChanged(int)),this,SLOT(clockChanged()));

    l->addWidget(clockLabel);
    l->addWidget(clock1Spinner);
    l->addWidget(clock2Spinner);
    l->addWidget(clockFrequencyLabel);

    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createRunControls()
{
    QWidget *box = new QWidget(this);
    QGridLayout *l = new QGridLayout();
    l->setMargin(0);

    // Number of Events setup spinner
    QLabel *nofEventLabel = new QLabel(tr("Number of Acquisitions:"));
    nofEventSpinner = new QSpinBox(this);
    nofEventSpinner->setMinimum(1);
    nofEventSpinner->setMaximum(100000);
    nofEventSpinner->setSingleStep(1);
    nofEventSpinner->setValue(module->conf.nof_reads);
    connect(nofEventSpinner,SIGNAL(valueChanged(int)),this,SLOT(nofEventsChanged()));

    // Number of Multi Events setup spinner
    QLabel *nofMultiEventLabel = new QLabel(tr("Multi events:"));
    nofMultiEventSpinner = new QSpinBox(this);
    nofMultiEventSpinner->setMinimum(1);
    nofMultiEventSpinner->setMaximum(100000);
    nofMultiEventSpinner->setSingleStep(1);
    nofMultiEventSpinner->setValue(module->conf.multievent_max_nof_events);
    connect(nofMultiEventSpinner,SIGNAL(valueChanged(int)),this,SLOT(nofMultiEventsChanged()));

    // Sample Length setup spinner
    QLabel *sampleLengthLabel = new QLabel(tr("Samples"));
    sampleLengthSpinner = new QSpinBox(this);
    sampleLengthSpinner->setMinimum(0);
    sampleLengthSpinner->setMaximum(16000);
    sampleLengthSpinner->setAccelerated(true);
    sampleLengthSpinner->setSingleStep(8);
    sampleLengthSpinner->setValue(module->conf.sample_length);
    connect(sampleLengthSpinner,SIGNAL(editingFinished()),this,SLOT(sampleLengthChanged()));

    l->addWidget(nofEventLabel,0,0,1,1);
    l->addWidget(nofEventSpinner,0,1,1,1);
    l->addWidget(nofMultiEventLabel,1,0,1,1);
    l->addWidget(nofMultiEventSpinner,1,1,1,1);
    l->addWidget(sampleLengthLabel,2,0,1,1);
    l->addWidget(sampleLengthSpinner,2,1,1,1);


    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createPretriggerControls()
{
    // Pretrigger setup spinner
    QWidget *box = new QWidget(this);
    QHBoxLayout *l = new QHBoxLayout();
    l->setMargin(0);

    QLabel *label = new QLabel(tr("Pretrigger:"),this);
    l->addWidget(label);

    pretriggerDelaySpinner = new QSpinBox(this);
    pretriggerDelaySpinner->setMinimum(0);
    pretriggerDelaySpinner->setMaximum(0x1FFF);
    pretriggerDelaySpinner->setAccelerated(true);
    pretriggerDelaySpinner->setSingleStep(1);
    pretriggerDelaySpinner->setValue(module->conf.pre_delay);
    connect(pretriggerDelaySpinner,SIGNAL(valueChanged(int)),this,SLOT(pretriggerChanged()));

    l->addWidget(pretriggerDelaySpinner);
    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createTriggerEnableControls()
{
    // Trigger Enable Checkboxes
    QWidget *box = new QWidget(this);
    QSignalMapper *mapper = new QSignalMapper();
    QHBoxLayout *l = new QHBoxLayout();
    l->setMargin(0);

    triggerEnable = new QList<QCheckBox*>();
    QLabel *label = new QLabel(tr("Enable:"),this);

    l->addWidget(label);

    for(int i=0; i<4; i++)
    {
        QCheckBox *trgEnable = new QCheckBox(this);
        if(module->conf.trigger_enable[i])
        {
            trgEnable->setChecked(true);
        }
        triggerEnable->append(trgEnable);
        connect(trgEnable,SIGNAL(stateChanged(int)),mapper,SLOT(map()));
        mapper->setMapping(trgEnable,i);
        l->addWidget(trgEnable);
    }
    connect(mapper,SIGNAL(mapped(int)),this,SLOT(enableChanged(int)));
    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createTriggerPeakControls()
{
    // Trigger Peak spinners
    QWidget *box = new QWidget(this);
    QSignalMapper *mapper = new QSignalMapper();
    QHBoxLayout *l = new QHBoxLayout();
    l->setMargin(0);

    peaks = new QList<QSpinBox*>();
    QLabel *label = new QLabel(tr("FIR Width:"),this);

    l->addWidget(label);

    for(int i=0; i<4; i++)
    {
        QSpinBox *peak = new QSpinBox(this);
        peak->setMinimum(0);
        peak->setMaximum(0x1F);
        peak->setAccelerated(true);
        peak->setSingleStep(1);
        peak->setValue(module->conf.trigger_peak_length[i]);
        peaks->append(peak);
        connect(peak,SIGNAL(valueChanged(int)),mapper,SLOT(map()));
        mapper->setMapping(peak,i);
        l->addWidget(peak);
    }
    connect(mapper,SIGNAL(mapped(int)),this,SLOT(peakChanged(int)));
    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createTriggerGateControls()
{
    // Trigger Gate spinners
    QWidget *box = new QWidget(this);
    QSignalMapper *mapper = new QSignalMapper();
    QHBoxLayout *l = new QHBoxLayout();
    l->setMargin(0);

    gates = new QList<QSpinBox*>();
    QLabel *label = new QLabel(tr("Gap width:"),this);

    l->addWidget(label);

    for(int i=0; i<4; i++)
    {
        QSpinBox *gate = new QSpinBox(this);
        gate->setMinimum(0);
        gate->setMaximum(0x1F);
        gate->setAccelerated(true);
        gate->setSingleStep(1);
        gate->setValue(module->conf.trigger_gap_length[i]);
        gates->append(gate);
        connect(gate,SIGNAL(valueChanged(int)),mapper,SLOT(map()));
        mapper->setMapping(gate,i);
        l->addWidget(gate);
    }
    connect(mapper,SIGNAL(mapped(int)),this,SLOT(gateChanged(int)));
    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createTriggerEdgeControls()
{
    // Trigger edge controls
    QWidget *box = new QWidget(this);
    QSignalMapper *mapper = new QSignalMapper();
    QHBoxLayout *l = new QHBoxLayout();
    l->setMargin(0);

    triggerGt = new QList<QCheckBox*>();
    QLabel *label = new QLabel(tr("Pos. edge:"),this);

    l->addWidget(label);

    for(int i=0; i<4; i++)
    {
        QCheckBox *trgGt = new QCheckBox(this);
        if(module->conf.trigger_gt[i])
        {
            trgGt->setChecked(true);
        }
        triggerGt->append(trgGt);
        connect(trgGt,SIGNAL(stateChanged(int)),mapper,SLOT(map()));
        mapper->setMapping(trgGt,i);
        l->addWidget(trgGt);
    }
    connect(mapper,SIGNAL(mapped(int)),this,SLOT(gtChanged(int)));
    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createTriggerThresholdControls()
{
    // Trigger Threshold Controls
    QWidget *box = new QWidget(this);
    QSignalMapper *mapper = new QSignalMapper();
    QHBoxLayout *l = new QHBoxLayout();
    l->setMargin(0);

    thresholds = new QList<QSpinBox*>();
    QLabel *label = new QLabel(tr("Thr:"),this);

    l->addWidget(label);

    for(int i=0; i<4; i++)
    {
        QSpinBox *thr = new QSpinBox(this);
        thr->setMinimum(0);
        thr->setMaximum(0xFFF);
        thr->setAccelerated(true);
        thr->setSingleStep(10);
        thr->setValue(module->conf.trigger_threshold[i]);
        thresholds->append(thr);
        connect(thr,SIGNAL(valueChanged(int)),mapper,SLOT(map()));
        mapper->setMapping(thr,i);
        l->addWidget(thr);
    }
    connect(mapper,SIGNAL(mapped(int)),this,SLOT(thrChanged(int)));
    box->setLayout(l);
    return box;
}

QWidget* Sis3350UI::createTriggerFIRControls()
{
    // Trigger Enable FIR mode
    QWidget *box = new QWidget(this);
    QSignalMapper *mapper = new QSignalMapper();
    QHBoxLayout *l = new QHBoxLayout();
    l->setMargin(0);

    triggerFir = new QList<QCheckBox*>();
    QLabel *label = new QLabel(tr("FIR mode:"),this);

    l->addWidget(label);

    for(int i=0; i<4; i++)
    {
        QCheckBox *trgFir = new QCheckBox(this);
        if(module->conf.trigger_fir[i])
        {
            trgFir->setChecked(true);
        }
        triggerFir->append(trgFir);
        connect(trgFir,SIGNAL(stateChanged(int)),mapper,SLOT(map()));
        mapper->setMapping(trgFir,i);
        l->addWidget(trgFir);
    }
    connect(mapper,SIGNAL(mapped(int)),this,SLOT(firChanged(int)));
    box->setLayout(l);
    return box;
}


void Sis3350UI::applySettings()
{
    baseAddressEdit->setText(tr("%1").arg(module->conf.base_addr,8,16,QChar('0')));
    modeBox->setCurrentIndex(module->conf.acMode);
    nofEventSpinner->setValue(module->conf.nof_reads);
    nofMultiEventSpinner->setValue(module->conf.multievent_max_nof_events);
    pretriggerDelaySpinner->setValue(module->conf.pre_delay);
    sampleLengthSpinner->setValue(module->conf.sample_length);
    viewport->changeSampleLength(module->conf.sample_length);
    clock1Spinner->setValue(module->conf.clock1);
    clock2Spinner->setValue(module->conf.clock2);

    clockSourceBox->setCurrentIndex(module->conf.clockSource);
    interruptSourceBox->setCurrentIndex(module->conf.irqSource);

    trgOutCheckbox->setChecked (module->conf.trigger_enable_lemo_out);

    for(int ch=0; ch<4; ch++)
    {
        module->conf.trigger_fir[ch]    ? triggerFir->at(ch)->setChecked(true)    : triggerFir->at(ch)->setChecked(false);
        module->conf.trigger_enable[ch] ? triggerEnable->at(ch)->setChecked(true) : triggerEnable->at(ch)->setChecked(false);
        module->conf.trigger_gt[ch]     ? triggerGt->at(ch)->setChecked(true)     : triggerGt->at(ch)->setChecked(false);

        offsets->at(ch)->setValue(module->conf.adc_offset[ch]);
        thresholds->at(ch)->setValue(module->conf.trigger_threshold[ch]);
        gates->at(ch)->setValue(module->conf.trigger_gap_length[ch]);
        peaks->at(ch)->setValue(module->conf.trigger_peak_length[ch]);
        gains->at(ch)->setValue(module->conf.variable_gain[ch]);
    }
}

void Sis3350UI::armClicked()
{
    if(module->isIfaceOpen())
        module->arm();
}

void Sis3350UI::disarmClicked()
{
    if(module->isIfaceOpen())
        module->disarm();
}

void Sis3350UI::resetClicked()
{
    if(module->isIfaceOpen())
        module->reset();
}

void Sis3350UI::triggerClicked()
{
    if(module->isIfaceOpen())
        module->trigger();
}

void Sis3350UI::timestampClearClicked()
{
    if(module->isIfaceOpen())
        module->timestampClear();
}

void Sis3350UI::singleShotClicked()
{
    timerArmMutex.lock();
    if(timerArmed && freeRunnerTimer->isActive())
    {
        timerArmed = false;
    }
    timerArmMutex.unlock();

    if(module->getInterface())
    {
        if (!module->getInterface()->isOpen())
            if (0 != module->getInterface()->open ()) {
                QMessageBox::warning (this, tr ("<%1> SIS3350 ADC").arg (name), tr ("Could not open interface"), QMessageBox::Ok);
                return;
            }

        // Only do one acquisition and do not save files
        module->conf.nof_reads = 1;
        unsigned int tempPollcount = module->conf.pollcount;
        module->conf.pollcount = 1000;

        // Schedule readout
        module->singleShot();

        // Reset values
        module->conf.pollcount = tempPollcount;

        viewport->update();
    }
}

void Sis3350UI::timerTimeout()
{    
    singleShotClicked();
}

void Sis3350UI::armTimer()
{
    timerArmMutex.lock();
    timerArmed = true;
    timerArmMutex.unlock();
}

void Sis3350UI::freeRunningButtonClicked()
{
    if(module->getInterface())
    {
        static bool isRunning = false;

        if(isRunning == false)
        {
            if (!module->getInterface()->isOpen())
                if (0 != module->getInterface()->open ()) {
                    QMessageBox::warning (this, tr ("<%1> SIS3350 ADC").arg (name), tr ("Could not open interface"), QMessageBox::Ok);
                    return;
                }

            isRunning = true;
            timerArmMutex.lock();
            timerArmed = true;
            timerArmMutex.unlock();
            freeRunnerTimer->start();
            freeRunningButton->setChecked(true);
        }
        else
        {
            isRunning = false;
            freeRunnerTimer->stop();
            freeRunningButton->setChecked(false);
        }
    }
}

void Sis3350UI::previewButtonClicked()
{
    if(viewport->isHidden())
    {
        viewport->show();
    }
    else
    {
        viewport->hide();
    }
}

void Sis3350UI::modeChanged(int newMode)
{
    module->conf.acMode = (Sis3350config::AcMode)newMode;

    // UI restrictions
    switch(module->conf.acMode)
    {
    case Sis3350config::ringBufferAsync:
        // No external trigger, no multi event
        // no trigger delay
        // No vme key
        // End of acquisition only address thr
        // + Prebuffer
        // + Ringbuffer sample length
        pretriggerDelaySpinner->setEnabled(true);
        pretriggerDelaySpinner->setMaximum(0x1FFF);
        nofEventSpinner->setEnabled(false);
        nofMultiEventSpinner->setEnabled(false);
        sampleLengthSpinner->setEnabled(true);
        sampleLengthSpinner->setMaximum(16000);
        break;
    case Sis3350config::ringBufferSync:
        // no trigger delay
        // + Prebuffer
        // + Ringbuffer sample length
        pretriggerDelaySpinner->setEnabled(true);
        pretriggerDelaySpinner->setMaximum(0x1FFF);
        nofEventSpinner->setEnabled(true);
        nofMultiEventSpinner->setEnabled(true);
        sampleLengthSpinner->setEnabled(true);
        sampleLengthSpinner->setMaximum(16000);
        break;
    case Sis3350config::directMemGateAsync:
        // No external trigger
        // No FIR trigger
        // no trigger delay
        // + Prebuffer
        // + Max Length
        // + Gate extend length
        pretriggerDelaySpinner->setEnabled(true);
        nofEventSpinner->setEnabled(false);
        nofMultiEventSpinner->setEnabled(false);
        sampleLengthSpinner->setEnabled(false);
        break;
    case Sis3350config::directMemGateSync:
        // No FIR trigger
        // no trigger delay
        // + Prebuffer
        // + Max Length
        // + Gate extend length
        pretriggerDelaySpinner->setEnabled(true);
        nofEventSpinner->setEnabled(true);
        nofMultiEventSpinner->setEnabled(true);
        sampleLengthSpinner->setEnabled(false);
        break;
    case Sis3350config::directMemStop:
        // No FIR trigger
        // + Prebuffer
        pretriggerDelaySpinner->setEnabled(true);
        nofEventSpinner->setEnabled(true);
        nofMultiEventSpinner->setEnabled(true);
        sampleLengthSpinner->setEnabled(false);
        break;
    case Sis3350config::directMemStart:
        // No FIR trigger
        pretriggerDelaySpinner->setEnabled(true);
        pretriggerDelaySpinner->setMaximum(16000);
        nofEventSpinner->setEnabled(true);
        nofMultiEventSpinner->setEnabled(true);
        sampleLengthSpinner->setEnabled(true);
        sampleLengthSpinner->setMaximum(65000000);
        break;
    default:
        break;
    }
}

void Sis3350UI::clockSourceChanged(int newSource)
{
    module->conf.clockSource = (Sis3350config::ClockSource)newSource;

    if(module->conf.clockSource == Sis3350config::intVar)
    {
        clock1Spinner->setEnabled(true);
        clock2Spinner->setEnabled(true);
    }
    else
    {
        clock1Spinner->setEnabled(false);
        clock2Spinner->setEnabled(false);
    }
}

void Sis3350UI::interruptSourceChanged(int newSource)
{
    module->conf.irqSource = (Sis3350config::IrqSource)newSource;
}

void Sis3350UI::gainChanged(int ch)
{
    int newValue = gains->at(ch)->value();
    module->conf.variable_gain[ch] = newValue;
    this->singleShotClicked();
}

void Sis3350UI::offChanged(int ch)
{
    int newValue = offsets->at(ch)->value();
    module->conf.adc_offset[ch] = newValue;
    this->singleShotClicked();
}

void Sis3350UI::thrChanged(int ch)
{
    int newValue = thresholds->at(ch)->value();
    module->conf.trigger_threshold[ch] = newValue;
    this->singleShotClicked();
}

void Sis3350UI::gateChanged(int ch)
{
    int newValue = gates->at(ch)->value();
    module->conf.trigger_gap_length[ch] = newValue;
    this->singleShotClicked();
}

void Sis3350UI::peakChanged(int ch)
{
    int newValue = peaks->at(ch)->value();
    module->conf.trigger_peak_length[ch] = newValue;
    this->singleShotClicked();
}

void Sis3350UI::enableChanged(int ch)
{
    bool newValue = triggerEnable->at(ch)->isChecked();
    module->conf.trigger_enable[ch] = newValue;
    this->singleShotClicked();
}

void Sis3350UI::firChanged(int ch)
{
    bool newValue = triggerFir->at(ch)->isChecked();
    module->conf.trigger_fir[ch] = newValue;
    this->singleShotClicked();
}

void Sis3350UI::gtChanged(int ch)
{
    bool newValue = triggerGt->at(ch)->isChecked();
    module->conf.trigger_gt[ch] = newValue;
    this->singleShotClicked();
}

void Sis3350UI::nofEventsChanged()
{
    int newValue = nofEventSpinner->value();
    module->conf.nof_reads = newValue;
    //printf("Changed nof_reads to %d\n",module->conf.nof_reads);
}

void Sis3350UI::nofMultiEventsChanged()
{
    int newValue = nofMultiEventSpinner->value();
    module->conf.multievent_max_nof_events = newValue;
    if(module->conf.multievent_max_nof_events > 1) module->conf.acq_multi_event = true;
    else module->conf.acq_multi_event = false;
    //printf("Changed nof_multievents to %d\n",module->conf.multievent_max_nof_events);
}

void Sis3350UI::pretriggerChanged()
{
    int newValue = pretriggerDelaySpinner->value();
    module->conf.pre_delay = newValue;
    module->conf.direct_mem_pretrigger_length = newValue;
    this->singleShotClicked();
}

void Sis3350UI::postProcessChanged()
{
}

void Sis3350UI::clockChanged()
{
    char buf[100];
    int newValue = clock1Spinner->value();
    module->conf.clock1 = newValue;
    int tmp = 25*newValue;
    newValue = clock2Spinner->value();
    module->conf.clock2 = newValue;
    switch(newValue)
    {
        case 1:
            tmp /= 2; break;
        case 2:
            tmp /= 4; break;
        case 3:
            tmp /= 8; break;
    }
    sprintf(buf,"%d MHz",tmp);
    this->clockFrequencyLabel->setText(tr(buf));
    this->singleShotClicked();
}

void Sis3350UI::sampleLengthChanged()
{
    int newValue = sampleLengthSpinner->value();
    newValue &= ~(0x7);
    if(newValue < 16) newValue = 16;
    sampleLengthSpinner->setValue(newValue);
    module->conf.sample_length = newValue;
    module->conf.direct_mem_sample_length = newValue;
    viewport->changeSampleLength(newValue);
    this->singleShotClicked();
}

void Sis3350UI::trgOutClicked (bool enabled) {
    module->conf.trigger_enable_lemo_out = enabled;
}
