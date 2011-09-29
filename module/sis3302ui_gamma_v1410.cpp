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

#include "sis3302ui_gamma_v1410.h"
#include "sis3302module_gamma_v1410.h"
#include <iostream>

Sis3302V1410UI::Sis3302V1410UI(Sis3302V1410Module* _module)
    : module(_module), uif(this,&tabs), applyingSettings(false),
      previewRunning(false)
{
    createUI();
    createPreviewUI();

    previewTimer = new QTimer();
    previewTimer->setInterval(100);
    previewTimer->setSingleShot(true);

    std::cout << "Instantiated" << _module->MODULE_NAME << "UI" << std::endl;
}

Sis3302V1410UI::~Sis3302V1410UI(){}

void Sis3302V1410UI::createUI()
{
    QGridLayout* l = new QGridLayout;
    l->setMargin(0);
    l->setVerticalSpacing(0);

    int nt = 0; // current tab number
    int ng = 0; // current group number

    // TAB ACQUISITION
    tn.append("Acq"); uif.addTab(tn[nt]);

    gn.append("Basic Setup"); uif.addGroupToTab(tn[nt],gn[ng],"","v");
    uif.addPopupToGroup(tn[nt],gn[ng],"Mode","acMode",
                        (QStringList()
                         << "Single Event"
                         << "Multi Event"
                         << "MCA Mode"));
    uif.addSpinnerToGroup(tn[nt],gn[ng],"Number of Events","nof_events",1,10000);
    uif.addLineEditReadOnlyToGroup(tn[nt],gn[ng],"Module ID","module_id_lineedit","unknown");
    uif.addLineEditReadOnlyToGroup(tn[nt],gn[ng],"Firmware","firmware_lineedit","unknown");
    uif.addButtonToGroup(tn[nt],gn[ng],"Update","update_firmware_button");
    uif.getWidgets()->find("nof_events").value()->setEnabled(false);

    gn.append("Advanced Setup"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    uif.addPopupToGroup(tn[nt],gn[ng],"Vme Access Mode","vmeMode",
                        (QStringList()
                         << "Single Reads"
                         << "DMA 32bit"
                         << "FIFO Reads"
                         << "Block Transfer 32bit"
                         << "Block Transfer 64bit"
                         << "VME2E accelerated mode"));
    uif.addLineEditToGroup(tn[nt],gn[ng],"Header ID","header_id","3302");

    gn.append("Control"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    uif.addUnnamedGroupToGroup(tn[nt],gn[ng],"b0_");
    uif.addButtonToGroup(tn[nt],gn[ng]+"b0_","Trigger","trigger_button");
    uif.addButtonToGroup(tn[nt],gn[ng]+"b0_","Stop","stop_button");
    uif.addUnnamedGroupToGroup(tn[nt],gn[ng],"b1_");
    uif.addButtonToGroup(tn[nt],gn[ng]+"b1_","Arm","arm_button");
    uif.addButtonToGroup(tn[nt],gn[ng]+"b1_","Disarm","disarm_button");
    uif.addUnnamedGroupToGroup(tn[nt],gn[ng],"b2_");
    uif.addButtonToGroup(tn[nt],gn[ng]+"b2_","Reset","reset_button");
    uif.addButtonToGroup(tn[nt],gn[ng]+"b2_","Clear Timestamp","clear_button");
    uif.addUnnamedGroupToGroup(tn[nt],gn[ng],"b3_");
    uif.addButtonToGroup(tn[nt],gn[ng]+"b3_","Configure","configure_button");
    uif.addButtonToGroup(tn[nt],gn[ng]+"b3_","Single Shot","singleshot_button");
    uif.getWidgets()->find("stop_button").value()->setEnabled(false);

    // TAB INPUT/OUTPUT Config
    tn.append("I/O"); nt++; uif.addTab(tn[nt]);
    gn.append("LEMO Input"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    uif.addPopupToGroup(tn[nt],gn[ng],"Mode","lemo_in_mode",(QStringList()
                               << "Veto" << "Gate"));
    for(int i = 0; i<3; ++i) {
        uif.addCheckBoxToGroup(tn[nt],gn[ng],tr("Enable Input %1").arg(i),
                               tr("enable_lemo_in_%1").arg(i));
    }
    gn.append("LEMO Output"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    uif.addPopupToGroup(tn[nt],gn[ng],"Mode","lemo_out_mode",(QStringList()
                               << "Trigger, Busy, Arm"
                               << "Trigger, Veto, Arm"
                               << "Neighbor-, Trigger, Neighbor+"
                               << "Neighbor-, Veto, Neighbor+"));
    gn.append("Feedback"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    uif.addCheckBoxToGroup(tn[nt],gn[ng],"Send internal trigger to external input","send_int_trg_to_ext_as_or");

    // TABs TRIGGER

    int ch = 0;
    int nofChPerTab = 2;
    int nofTabs = module->NOF_CHANNELS/nofChPerTab;

    for(int t=0; t<nofTabs; t++) {
        tn.append(tr("T%1-%2").arg(ch).arg(ch+nofChPerTab)); nt++; uif.addTab(tn[nt]);

        for(int i=0; i<1; i++)
        {
            QString un = tr("noname_%1").arg(i);
            gn.append(un); ng++; uif.addUnnamedGroupToTab(tn[nt],gn[ng]);
            for(int j=0; j<nofChPerTab; j++)
            {
                gn.append(tr("Channel %1").arg(ch)); ng++; uif.addGroupToGroup(tn[nt],un,gn[ng],tr("enable_ch_%1").arg(ch));
                uif.addPopupToGroup(tn[nt],un+gn[ng],"Decimation",tr("trg_decim_mode_%1").arg(ch),(QStringList()
                                          << "OFF"
                                          << "2"
                                          << "4"
                                          << "8"));
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Threshold",tr("trigger_threshold_%1").arg(ch),-65536,65535); // 17 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Sum Gap Time",tr("trigger_sumg_length_%1").arg(ch),1,1023);
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Peak Time",tr("trigger_peak_length_%1").arg(ch),1,1023);
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Pulse Length",tr("trigger_pulse_length_%1").arg(ch),0,0xff);  // 8 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Int. Gate",tr("trigger_int_gate_length_%1").arg(ch),0,0xff);  // 8 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Int. Delay",tr("trigger_int_trg_delay_%1").arg(ch),0,0xff);  // 8 bits
                uif.addCheckBoxToGroup(tn[nt],un+gn[ng],"Ext. Gate",tr("enable_ext_gate_%1").arg(ch));
                uif.addCheckBoxToGroup(tn[nt],un+gn[ng],"Ext. Trigger",tr("enable_ext_trg_%1").arg(ch));
                uif.addCheckBoxToGroup(tn[nt],un+gn[ng],"Int. Gate",tr("enable_int_gate_%1").arg(ch));
                uif.addCheckBoxToGroup(tn[nt],un+gn[ng],"Int. Trigger",tr("enable_int_trg_%1").arg(ch));
                uif.addCheckBoxToGroup(tn[nt],un+gn[ng],"Disable Output",tr("disable_trg_out_%1").arg(ch));
                uif.addCheckBoxToGroup(tn[nt],un+gn[ng],"Invert Input",tr("enable_input_invert_%1").arg(ch));
                ch++;
            }
        }
    }


    // TABs ENERGY
    ch = 0;
    nofChPerTab = 2;
    nofTabs = module->NOF_ADC_GROUPS/nofChPerTab;

    for(int t=0; t<nofTabs; ++t) {
        tn.append(tr("E%1-%2").arg(ch).arg(ch+nofChPerTab)); nt++; uif.addTab(tn[nt]);

        for(int i=0; i<1; i++)
        {
            QString un = tr("noname_%1").arg(i);
            gn.append(un); ng++; uif.addUnnamedGroupToTab(tn[nt],gn[ng]);
            for(int j=0; j<nofChPerTab; j++)
            {
                gn.append(tr("Channel %1/%2").arg(ch*2).arg(ch*2+1)); ng++; uif.addGroupToGroup(tn[nt],un,gn[ng]);
                uif.addPopupToGroup(tn[nt],un+gn[ng],"Average",tr("energy_decim_mode_%1").arg(ch),(QStringList()
                                          << "OFF"
                                          << "2"
                                          << "4"
                                          << "8"));
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Sum Gap Time",tr("energy_sumg_length_%1").arg(ch),1,255);
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Peak Time",tr("energy_peak_length_%1").arg(ch),1,1023);
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"E Gate Length",tr("energy_gate_length_%1").arg(ch),0,0xffff);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"T Gate Length",tr("trigger_gate_length_%1").arg(ch),0,0x65535);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Pretrigger",tr("trigger_pretrigger_delay_%1").arg(ch),0,1023);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"RAW Length",tr("raw_sample_length_%1").arg(ch),0,65532);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Energy Length",tr("energy_sample_length_%1").arg(ch),0,510);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Start Index 1",tr("energy_sample_start_idx_1_%1").arg(ch),0,0xffff);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Start Index 2",tr("energy_sample_start_idx_2_%1").arg(ch),0,0xffff);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Start Index 3",tr("energy_sample_start_idx_3_%1").arg(ch),0,0xffff);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Start Index RAW",tr("raw_sample_start_idx_%1").arg(ch),0,0xffff);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Tau",tr("energy_tau_%1").arg(ch),0,0xff);  // 8 bits
                uif.addCheckBoxToGroup(tn[nt],un+gn[ng],"Extra Filter",tr("enable_energy_extra_filter_%1").arg(ch));
                QSpinBox* s = (QSpinBox*)(uif.getWidgets()->find(tr("raw_sample_length_%1").arg(ch)).value());
                s->setSingleStep(4);
                s = (QSpinBox*)(uif.getWidgets()->find(tr("energy_sample_length_%1").arg(ch)).value());
                s->setSingleStep(2);
                ch++;
            }
        }
    }

#ifdef unused
    // TAB EVENT
    tn.append("Event"); nt++; uif.addTab(tn[nt]);

    gn.append("Delay and Length"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    uif.addSpinnerToGroup(tn[nt],gn[ng],"Start Delay","start_delay",0,0xffffff); // 24 bits
    uif.addSpinnerToGroup(tn[nt],gn[ng],"Stop Delay","stop_delay",0,0xffffff); // 24 bits
    uif.addSpinnerToGroup(tn[nt],gn[ng],"Number of Events","nof_events",0,512 /*0x0fffff*/); // 20 bits actually, but only 512 can be stored
    uif.addCheckBoxToGroup(tn[nt],gn[ng],"Event Length Stop Mode","event_length_as_stop");
    uif.addSpinnerToGroup(tn[nt],gn[ng],"Event Length","event_length",0,0x1ffffff); // 25 bits

    gn.append("Advanced"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    uif.addCheckBoxToGroup(tn[nt],gn[ng],"Page Wrap","enable_page_wrap");
    uif.addPopupToGroup(tn[nt],gn[ng],"Page Wrap Size","wrapSize",(QStringList() << "64" << "128" << "256" << "512" << "1k" << "4k" << "16k" << "64k" << "256k" << "1M" << "4M" << "16M"));
    uif.addPopupToGroup(tn[nt],gn[ng],"Averaging Mode","avgMode",(QStringList() << "1" << "2" << "4" << "8" << "16" << "32" << "64" << "128"));
#endif

    // TAB DAC SETUP
    tn.append("DAC setup"); nt++; uif.addTab(tn[nt]);

    gn.append("Offsets"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    for(int ch=0; ch<8; ch++)
    {
        uif.addSpinnerToGroup(tn[nt],gn[ng],tr("DAC offset %1").arg(ch),tr("dac_offset_%1").arg(ch),0,0xffff);
    }

    // TAB Clock and IRQ
    tn.append("Clock && IRQ"); nt++; uif.addTab(tn[nt]);

    gn.append("Clock"); ng++; uif.addGroupToTab(tn[nt],gn[ng]);
    uif.addPopupToGroup(tn[nt],gn[ng],"Clock source","clockSource",(QStringList()
             << "Internal 100 MHz"
             << "Internal 50 MHz"
             << "Internal 25 MHz"
             << "Internal 10 MHz"
             << "Internal 1 MHz"
             << "100 kHz clock"
             << "external clock (from LEMO)"
             << "Real 100 MHz internal"));

    gn.append("Interrupt Setup"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    uif.addCheckBoxToGroup(tn[nt],gn[ng],"VME enable IRQ","enable_irq");
    uif.addPopupToGroup(tn[nt],gn[ng],"IRQ Mode","irqMode",(QStringList() << "RORA" << "ROAK"));
    uif.addPopupToGroup(tn[nt],gn[ng],"IRQ Source","irqSource",(QStringList()
             << "End of Address Threshold (Edge)"
             << "End of Address Threshold (Level)"));
    uif.addSpinnerToGroup(tn[nt],gn[ng],"IRQ level","irq_level",0,7);
    uif.addHexSpinnerToGroup(tn[nt],gn[ng],"IRQ vector","irq_vector",0,0xff);


    //###

    l->addWidget(dynamic_cast<QWidget*>(&tabs));

    QWidget* bottomButtons = new QWidget(this);
    {
    QHBoxLayout* l = new QHBoxLayout();
    QPushButton* previewButton = new QPushButton("Preview");
    connect(previewButton,SIGNAL(clicked()),this,SLOT(clicked_previewButton()));
    l->addWidget(previewButton);
    singleShotPreviewButton = new QPushButton("Singleshot");
    startStopPreviewButton = new QPushButton("Start");
    startStopPreviewButton->setCheckable(true);
    connect(singleShotPreviewButton,SIGNAL(clicked()),this,SLOT(clicked_singleshot_button()));
    connect(startStopPreviewButton,SIGNAL(clicked()),this,SLOT(clicked_startStopPreviewButton()));
    l->addWidget(singleShotPreviewButton);
    l->addWidget(startStopPreviewButton);
    bottomButtons->setLayout(l);
    }
    l->addWidget(bottomButtons);

    this->setLayout(l);
    connect(uif.getSignalMapper(),SIGNAL(mapped(QString)),this,SLOT(uiInput(QString)));

//    QList<QWidget*> li = this->findChildren<QWidget*>();
//    foreach(QWidget* w, li)
//    {
//        printf("%s\n",w->objectName().toStdString().c_str());
//    }
}

void Sis3302V1410UI::createPreviewUI()
{
    int nofCh = Sis3302V1410Module::NOF_CHANNELS;
    int nofCols = nofCh/sqrt(nofCh);
    int nofRows = nofCh/nofCols;


    QTabWidget* t = new QTabWidget();
    QGridLayout* l = new QGridLayout();

    // RAW displays
    int ch = 0;
    QWidget* rw = new QWidget();
    {
        QGridLayout* l = new QGridLayout();
        for(int r = 0; r < nofRows; ++r) {
            for(int c = 0; c < nofCols; ++c) {
                previewCh[ch] = new plot2d(this,QSize(240,200),ch);
                previewCh[ch]->addChannel(0,"adc",previewData[ch],QColor(Qt::blue),Channel::line,1);
                l->addWidget(previewCh[ch],r,c,1,1);
                ++ch;
            }
        }
        rw->setLayout(l);
    }
    // Energy displays
    ch = 0;
    QWidget* ew = new QWidget();
    {
        QGridLayout* l = new QGridLayout();
        for(int r = 0; r < nofRows; ++r) {
            for(int c = 0; c < nofCols; ++c) {
                previewEnergy[ch] = new plot2d(this,QSize(240,200),ch);
                previewEnergy[ch]->addChannel(0,"adc",previewEnergyData[ch],QColor(Qt::red),Channel::line,1);
                l->addWidget(previewEnergy[ch],r,c,1,1);
                ++ch;
            }
        }
        ew->setLayout(l);
    }
    // Value displays
    ch = 0;
    QWidget* vw = new QWidget();
    {
        QGridLayout* l = new QGridLayout();
        for(int r = 0; r < nofRows; ++r) {
            for(int c = 0; c < nofCols; ++c) {
                QGroupBox* b = new QGroupBox(tr("Ch: %1").arg(ch));
                {
                    QGridLayout* lb = new QGridLayout();
                    headerIdDisplay[ch] = new QLabel();
                    energyMinValueDisplay[ch] = new QLabel();
                    energyMaxValueDisplay[ch] = new QLabel();
                    timestampDisplay[ch] = new QLabel();
                    fastTriggerCounterDisplay[ch] = new QLabel();
                    flagDisplay[ch] = new QLabel();

                    lb->addWidget(new QLabel("Header ID:"),0,0,1,1);
                    lb->addWidget(headerIdDisplay[ch],0,1,1,1);
                    lb->addWidget(new QLabel("Flags:"),1,0,1,1);
                    lb->addWidget(flagDisplay[ch],1,1,1,1);
                    lb->addWidget(new QLabel("Trigger counter:"),2,0,1,1);
                    lb->addWidget(fastTriggerCounterDisplay[ch],2,1,1,1);
                    lb->addWidget(new QLabel("Timestamp:"),3,0,1,1);
                    lb->addWidget(timestampDisplay[ch],3,1,1,1);
                    lb->addWidget(new QLabel("Energy min:"),4,0,1,1);
                    lb->addWidget(energyMinValueDisplay[ch],4,1,1,1);
                    lb->addWidget(new QLabel("Energy max:"),5,0,1,1);
                    lb->addWidget(energyMaxValueDisplay[ch],5,1,1,1);

                    b->setLayout(lb);
                }
                l->addWidget(b,r,c,1,1);
                ++ch;
            }
        }
        vw->setLayout(l);
    }

    ew->setEnabled(false);
    t->addTab(rw,"RAW signals");
    t->addTab(ew,"Energy");
    t->addTab(vw,"Values");
    l->addWidget(t);

    previewWindow.setLayout(l);
    previewWindow.setWindowTitle("Signal preview");
    previewWindow.resize(640,480);
}

// Slot handling

void Sis3302V1410UI::uiInput(QString _name)
{
    if(applyingSettings == true) return;

    QGroupBox* gb = findChild<QGroupBox*>(_name);
    if(gb != 0)
    {
        if(_name.startsWith("enable_ch")) {
            int ch = _name.right(1).toInt();
            if(gb->isChecked()) module->conf.enable_ch[ch] = true;
            else module->conf.enable_ch[ch] = false;
            printf("Changed enable_ch %d\n",ch);
        }
    }

    QCheckBox* cb = findChild<QCheckBox*>(_name);
    if(cb != 0)
    {
        if(_name == "enable_irq") {
            if(cb->isChecked()) module->conf.enable_irq = true;
            else module->conf.enable_irq = false;
        }
        if(_name == "send_int_trg_to_ext_as_or") {
            module->conf.send_int_trg_to_ext_as_or = cb->isChecked();
        }
        if(_name.startsWith("enable_lemo_in_")) {
            int ch = _name.right(1).toInt();
            module->conf.enable_lemo_in[ch] = cb->isChecked();
        }
        if(_name.startsWith("enable_ext_gate_")) {
            int ch = _name.right(1).toInt();
            module->conf.enable_ext_gate[ch] = cb->isChecked();
        }
        if(_name.startsWith("enable_ext_trg_")) {
            int ch = _name.right(1).toInt();
            module->conf.enable_ext_trg[ch] = cb->isChecked();
        }
        if(_name.startsWith("enable_int_gate_")) {
            int ch = _name.right(1).toInt();
            module->conf.enable_int_gate[ch] = cb->isChecked();
        }
        if(_name.startsWith("enable_int_trg_")) {
            int ch = _name.right(1).toInt();
            module->conf.enable_int_trg[ch] = cb->isChecked();
        }
        if(_name.startsWith("disable_trg_out_")) {
            int ch = _name.right(1).toInt();
            module->conf.disable_trg_out[ch] = cb->isChecked();
        }
        if(_name.startsWith("enable_input_invert_")) {
            int ch = _name.right(1).toInt();
            module->conf.enable_input_invert[ch] = cb->isChecked();
        }
        if(_name.startsWith("enable_energy_extra_filter_")) {
            int ch = _name.right(1).toInt();
            module->conf.enable_energy_extra_filter[ch] = cb->isChecked();
        }
        //QMessageBox::information(this,"uiInput","You changed the checkbox "+_name);
    }

    QComboBox* cbb = findChild<QComboBox*>(_name);
    if(cbb != 0)
    {
        if(_name == "acMode") {
            module->conf.acMode = static_cast<Sis3302V1410config::AcMode>(cbb->currentIndex());
            if(module->conf.acMode == Sis3302V1410config::multiEvent) {
                uif.getWidgets()->find("nof_events").value()->setEnabled(true);
            } else {
                uif.getWidgets()->find("nof_events").value()->setEnabled(false);
            }
        }
        if(_name == "vmeMode") module->conf.vmeMode = static_cast<Sis3302V1410config::VmeMode>(cbb->currentIndex());
        if(_name == "clockSource") module->conf.clockSource = static_cast<Sis3302V1410config::ClockSource>(cbb->currentIndex());
        if(_name == "irqSource") module->conf.irqSource = static_cast<Sis3302V1410config::IrqSource>(cbb->currentIndex());
        if(_name == "irqMode") module->conf.irqMode = static_cast<Sis3302V1410config::IrqMode>(cbb->currentIndex());
        if(_name == "lemo_in_mode") module->conf.lemo_in_mode = static_cast<Sis3302V1410config::LemoInMode>(cbb->currentIndex());
        if(_name == "lemo_out_mode") module->conf.lemo_out_mode = static_cast<Sis3302V1410config::LemoOutMode>(cbb->currentIndex());
        if(_name.startsWith("trg_decim_mode_")) {
            int ch = _name.right(1).toInt();
            module->conf.trigger_decim_mode[ch] = static_cast<Sis3302V1410config::TrgDecimMode>(cbb->currentIndex());
        }
        if(_name.startsWith("energy_decim_mode_")) {
            int ch = _name.right(1).toInt();
            module->conf.energy_decim_mode[ch] = static_cast<Sis3302V1410config::EnDecimMode>(cbb->currentIndex());
        }
        //QMessageBox::information(this,"uiInput","You changed the combobox "+_name);
    }
    QSpinBox* sb = findChild<QSpinBox*>(_name);
    if(sb != 0)
    {
        if(_name == "irq_level") module->conf.irq_level = sb->value();
        if(_name == "irq_vector"){
            module->conf.irq_vector = sb->value();
        }
        if(_name == "nof_events"){
            module->conf.nof_events = sb->value();
        }
        if(_name.startsWith("trigger_pulse_length_")) {
            int ch = _name.right(1).toInt();
            module->conf.trigger_pulse_length[ch] = sb->value();
        }
        if(_name.startsWith("trigger_sumg_length_")) {
            int ch = _name.right(1).toInt();
            module->conf.trigger_sumg_length[ch] = sb->value();
        }
        if(_name.startsWith("trigger_peak_length_")) {
            int ch = _name.right(1).toInt();
            module->conf.trigger_peak_length[ch] = sb->value();
        }
        if(_name.startsWith("trigger_threshold_")) {
            int ch = _name.right(1).toInt();
            module->conf.trigger_threshold[ch] = sb->value();
        }
        if(_name.startsWith("dac_offset_")) {
            int ch = _name.right(1).toInt();
            module->conf.dac_offset[ch] = sb->value();
        }
        if(_name.startsWith("trigger_gate_length_")) {
            int ch = _name.right(1).toInt();
            module->conf.trigger_gate_length[ch] = sb->value();
        }
        if(_name.startsWith("trigger_pretrigger_delay_")) {
            int ch = _name.right(1).toInt();
            module->conf.trigger_pretrigger_delay[ch] = sb->value();
        }
        if(_name.startsWith("trigger_int_gate_length_")) {
            int ch = _name.right(1).toInt();
            module->conf.trigger_int_gate_length[ch] = sb->value();
        }
        if(_name.startsWith("trigger_int_trg_delay_")) {
            int ch = _name.right(1).toInt();
            module->conf.trigger_int_trg_delay[ch] = sb->value();
        }
        if(_name.startsWith("energy_sumg_length_")) {
            int ch = _name.right(1).toInt();
            module->conf.energy_sumg_length[ch] = sb->value();
        }
        if(_name.startsWith("energy_peak_length_")) {
            int ch = _name.right(1).toInt();
            module->conf.energy_peak_length[ch] = sb->value();
        }
        if(_name.startsWith("energy_gate_length_")) {
            int ch = _name.right(1).toInt();
            module->conf.energy_gate_length[ch] = sb->value();
        }
        if(_name.startsWith("energy_sample_length_")) {
            int ch = _name.right(1).toInt();
            module->conf.energy_sample_length[ch] = sb->value();
            module->updateEndAddrThresholds();
        }
        if(_name.startsWith("raw_sample_length_")) {
            int ch = _name.right(1).toInt();
            module->conf.raw_sample_length[ch] = sb->value();
            module->conf.header_id[ch] = sb->value();
            module->updateEndAddrThresholds();
        }
        if(_name.startsWith("energy_sample_start_idx_1_")) {
            int ch = _name.right(1).toInt();
            module->conf.energy_sample_start_idx[ch][0] = sb->value();
        }
        if(_name.startsWith("energy_sample_start_idx_2_")) {
            int ch = _name.right(1).toInt();
            module->conf.energy_sample_start_idx[ch][1] = sb->value();
        }
        if(_name.startsWith("energy_sample_start_idx_3_")) {
            int ch = _name.right(1).toInt();
            module->conf.energy_sample_start_idx[ch][2] = sb->value();
        }
        if(_name.startsWith("raw_sample_start_idx_")) {
            int ch = _name.right(1).toInt();
            module->conf.raw_data_sample_start_idx[ch] = sb->value();
        }
        if(_name.startsWith("energy_tau_")) {
            int ch = _name.right(1).toInt();
            module->conf.energy_tau[ch] = sb->value();
        }
    }
    QPushButton* pb = findChild<QPushButton*>(_name);
    if(pb != 0)
    {
        if(_name == "trigger_button") clicked_start_button();
        //if(_name == "stop_button") clicked_stop_button();
        if(_name == "arm_button") clicked_arm_button();
        if(_name == "disarm_button") clicked_disarm_button();
        if(_name == "reset_button") clicked_reset_button();
        if(_name == "clear_button") clicked_clear_button();
        if(_name == "configure_button") clicked_configure_button();
        if(_name == "singleshot_button") clicked_singleshot_button();
        if(_name == "update_firmware_button") clicked_update_firmware_button();
    }
}

void Sis3302V1410UI::clicked_start_button()
{
    module->trigger();
}

void Sis3302V1410UI::clicked_arm_button()
{
    module->arm(1);
}

void Sis3302V1410UI::clicked_disarm_button()
{
    module->disarm();
}

void Sis3302V1410UI::clicked_reset_button()
{
    module->reset();
}

void Sis3302V1410UI::clicked_clear_button()
{
    module->timestamp_clear();
}

void Sis3302V1410UI::clicked_configure_button()
{
    module->configure();
}

void Sis3302V1410UI::clicked_singleshot_button()
{
    if(!module->getInterface()) {
        return;
    }
    if(!module->getInterface()->isOpen()){
        if(0 != module->getInterface()->open()) {
            QMessageBox::warning (this, tr ("<%1> SIS3302 ADC").arg (module->MODULE_NAME), tr ("Could not open interface"), QMessageBox::Ok);
            return;
        }
    }
    module->singleShot();

    updatePreview();

    if(previewRunning) {
        previewTimer->start();
    }
}

void Sis3302V1410UI::clicked_update_firmware_button()
{
    module->updateModuleInfo();
    QLineEdit* m = (QLineEdit*) uif.getWidgets()->find("module_id_lineedit").value();
    QLineEdit* f = (QLineEdit*) uif.getWidgets()->find("firmware_lineedit").value();
    f->setText(tr("%1.%2").arg(module->conf.firmware_major_rev,2,16).arg(module->conf.firmware_minor_rev,2,16));
    m->setText(tr("0x%1").arg(module->conf.module_id,4,16));
}

void Sis3302V1410UI::clicked_previewButton()
{
    if(previewWindow.isHidden())
    {
        previewWindow.show();
    }
    else
    {
        previewWindow.hide();
    }
}

void Sis3302V1410UI::clicked_startStopPreviewButton()
{
    if(previewRunning) {
        // Stopping
        startStopPreviewButton->setText("Start");
        previewTimer->stop();
        previewRunning = false;
    } else {
        // Starting
        startStopPreviewButton->setText("Stop");
        if(previewWindow.isHidden()) {
            previewWindow.show();
        }
        previewRunning = true;
        previewTimer->start();
        connect(previewTimer,SIGNAL(timeout()),this,SLOT(timeout_previewTimer()));
    }
}

void Sis3302V1410UI::updatePreview()
{
    //std::cout << "Sis3302V1410UI::updatePreview" << std::endl << std::flush;
    for(int ch = 0; ch < NOF_CHANNELS; ++ch) {
        if(module->conf.enable_ch[ch]) {
            // Values
            headerIdDisplay[ch]->setText(tr("0x%1").arg(module->currentHeader[ch],4,16));
            QString flagString = "";
            if(module->currentPileupFlag[ch]) flagString += "Pileup ";
            if(module->currentRetriggerFlag[ch]) flagString += "Retrigger";
            flagDisplay[ch]->setText(flagString);
            timestampDisplay[ch]->setText(tr("0x%1").arg(module->currentTimestamp[ch],16,16));
            energyMinValueDisplay[ch]->setText(tr("%1").arg(module->currentEnergyFirstValue[ch]));
            energyMaxValueDisplay[ch]->setText(tr("%1").arg(module->currentEnergyMaxValue[ch]));
            fastTriggerCounterDisplay[ch]->setText(tr("%1").arg(module->currentTriggerCounter[ch]));

            // RAW data
            previewData[ch].resize(module->conf.raw_sample_length[ch/2]);
            //printf("Channel size: %d\n",previewData[ch].size());
            for(int i = 0; i < previewData[ch].size(); ++i) {
                previewData[ch][i] = module->currentRawBuffer[ch][i];
                //printf("%d,%d: %f\n",ch,i,previewData[ch][i]);
            }
            previewCh[ch]->getChannelById(0)->setData(previewData[ch]);
            previewCh[ch]->update();

            // Energy data, only if there was no pileup
            if(module->currentPileupFlag[ch] == false) {
                previewEnergyData[ch].resize(module->conf.energy_sample_length[ch/2]);
                if(previewEnergyData[ch].size() > 510) previewEnergyData[ch].resize(510);

                //printf("Channel size: %d\n",previewEnergyData[ch].size());
                for(int i = 0; i < previewEnergyData[ch].size(); ++i) {
                    previewEnergyData[ch][i] = module->currentEnergyBuffer[ch][i]
                            - module->currentEnergyFirstValue[ch];
                    //printf("%d,%d: %f\n",ch,i,previewEnergyData[ch][i]);
                }
            } else {
                previewEnergyData[ch].clear();
            }
            previewEnergy[ch]->resetBoundaries(0);
            previewEnergy[ch]->getChannelById(0)->setData(previewEnergyData[ch]);
            previewEnergy[ch]->update();
        }
    }
}

void Sis3302V1410UI::timeout_previewTimer() {
    if(previewRunning) {
        clicked_singleshot_button();
    }
}

// Settings handling

void Sis3302V1410UI::applySettings()
{
    applyingSettings = true;

    QList<QGroupBox*> gbs = findChildren<QGroupBox*>();
    if(!gbs.empty())
    {
        QList<QGroupBox*>::const_iterator it = gbs.begin();
        while(it != gbs.end())
        {
            QGroupBox* w = (*it);
            for(int ch=0; ch<NOF_CHANNELS; ch++) {
                if(w->objectName() == tr("enable_ch_%1").arg(ch)) w->setChecked(module->conf.enable_ch[ch]);
            }
            it++;
        }
    }
    QList<QCheckBox*> cbs = findChildren<QCheckBox*>();
    if(!cbs.empty())
    {
        QList<QCheckBox*>::const_iterator it = cbs.begin();
        while(it != cbs.end())
        {
            QCheckBox* w = (*it);

            if(w->objectName() == "enable_irq") w->setChecked(module->conf.enable_irq);
            if(w->objectName() == "send_int_trg_to_ext_as_or") w->setChecked(module->conf.send_int_trg_to_ext_as_or);

            for(int ch=0; ch<NOF_CHANNELS; ch++) {
                if(w->objectName() == tr("enable_lemo_in_%1").arg(ch)) w->setChecked(module->conf.enable_lemo_in[ch]);
                if(w->objectName() == tr("enable_ext_gate_%1").arg(ch)) w->setChecked(module->conf.enable_ext_gate[ch]);
                if(w->objectName() == tr("enable_ext_trg_%1").arg(ch)) w->setChecked(module->conf.enable_ext_trg[ch]);
                if(w->objectName() == tr("enable_int_gate_%1").arg(ch)) w->setChecked(module->conf.enable_int_gate[ch]);
                if(w->objectName() == tr("enable_int_trg_%1").arg(ch)) w->setChecked(module->conf.enable_int_trg[ch]);
                if(w->objectName() == tr("disable_trg_out_%1").arg(ch)) w->setChecked(module->conf.disable_trg_out[ch]);
                if(w->objectName() == tr("enable_input_invert_%1").arg(ch)) w->setChecked(module->conf.enable_input_invert[ch]);
                if(w->objectName() == tr("enable_energy_extra_filter_%1").arg(ch)) w->setChecked(module->conf.enable_energy_extra_filter[ch]);
            }
            it++;
        }
    }
    QList<QComboBox*> cbbs = findChildren<QComboBox*>();
    if(!cbbs.empty())
    {
        QList<QComboBox*>::const_iterator it = cbbs.begin();
        while(it != cbbs.end())
        {
            QComboBox* w = (*it);
            //printf("Found combobox with the name %s\n",w->objectName().toStdString().c_str());
            if(w->objectName() == "acMode") {
                w->setCurrentIndex(module->conf.acMode);
                if(module->conf.acMode == Sis3302V1410config::multiEvent) {
                    uif.getWidgets()->find("nof_events").value()->setEnabled(true);
                }
            }
            if(w->objectName() == "clockSource") w->setCurrentIndex(module->conf.clockSource);
            if(w->objectName() == "irqSource") w->setCurrentIndex(module->conf.irqSource);
            if(w->objectName() == "vmeMode") w->setCurrentIndex(module->conf.vmeMode);
            if(w->objectName() == "lemo_in_mode") w->setCurrentIndex(module->conf.lemo_in_mode);
            if(w->objectName() == "lemo_out_mode") w->setCurrentIndex(module->conf.lemo_out_mode);
            if(w->objectName() == "irqMode") w->setCurrentIndex(module->conf.irqMode);
            for(int ch=0; ch<NOF_CHANNELS; ch++)
            {
                if(w->objectName() == tr("trg_decim_mode_%1").arg(ch)) w->setCurrentIndex(module->conf.trigger_decim_mode[ch]);
                if(ch < 4 && w->objectName() == tr("energy_decim_mode_%1").arg(ch)) w->setCurrentIndex(module->conf.energy_decim_mode[ch]);
            }
            it++;
        }
    }
    QList<QSpinBox*> csb = findChildren<QSpinBox*>();
    if(!csb.empty())
    {
        QList<QSpinBox*>::const_iterator it = csb.begin();
        while(it != csb.end())
        {
            QSpinBox* w = (*it);
            //printf("Found spinbox with the name %s\n",w->objectName().toStdString().c_str());
            if(w->objectName() == "irq_level") w->setValue(module->conf.irq_level);
            if(w->objectName() == "irq_vector") w->setValue(module->conf.irq_vector);
            if(w->objectName() == "nof_events") w->setValue(module->conf.nof_events);

            for(int ch=0; ch<NOF_CHANNELS; ch++)
            {
                if(w->objectName() == tr("trigger_pulse_length_%1").arg(ch)) w->setValue(module->conf.trigger_pulse_length[ch]);
                if(w->objectName() == tr("trigger_sumg_length_%1").arg(ch)) w->setValue(module->conf.trigger_sumg_length[ch]);
                if(w->objectName() == tr("trigger_peak_length_%1").arg(ch)) w->setValue(module->conf.trigger_peak_length[ch]);
                if(w->objectName() == tr("trigger_threshold_%1").arg(ch)) w->setValue(module->conf.trigger_threshold[ch]);
                if(w->objectName() == tr("trigger_gate_length_%1").arg(ch)) w->setValue(module->conf.trigger_gate_length[ch]);
                if(w->objectName() == tr("trigger_pretrigger_delay_%1").arg(ch)) w->setValue(module->conf.trigger_pretrigger_delay[ch]);
                if(w->objectName() == tr("trigger_int_gate_length_%1").arg(ch)) w->setValue(module->conf.trigger_int_gate_length[ch]);
                if(w->objectName() == tr("trigger_int_trg_delay_%1").arg(ch)) w->setValue(module->conf.trigger_int_trg_delay[ch]);
                if(w->objectName() == tr("energy_sumg_length_%1").arg(ch)) w->setValue(module->conf.energy_sumg_length[ch]);
                if(w->objectName() == tr("energy_peak_length_%1").arg(ch)) w->setValue(module->conf.energy_peak_length[ch]);
                if(w->objectName() == tr("energy_gate_length_%1").arg(ch)) w->setValue(module->conf.energy_gate_length[ch]);
                if(w->objectName() == tr("energy_sample_length_%1").arg(ch)) w->setValue(module->conf.energy_sample_length[ch]);
                if(w->objectName() == tr("raw_sample_length_%1").arg(ch)) w->setValue(module->conf.raw_sample_length[ch]);
                if(w->objectName() == tr("energy_sample_start_idx_1_%1").arg(ch)) w->setValue(module->conf.energy_sample_start_idx[ch][0]);
                if(w->objectName() == tr("energy_sample_start_idx_2_%1").arg(ch)) w->setValue(module->conf.energy_sample_start_idx[ch][1]);
                if(w->objectName() == tr("energy_sample_start_idx_3_%1").arg(ch)) w->setValue(module->conf.energy_sample_start_idx[ch][2]);
                if(w->objectName() == tr("raw_sample_start_idx_%1").arg(ch)) w->setValue(module->conf.raw_data_sample_start_idx[ch]);
                if(w->objectName() == tr("energy_tau_%1").arg(ch)) w->setValue(module->conf.energy_tau[ch]);
                if(w->objectName() == tr("dac_offset_%1").arg(ch)) w->setValue(module->conf.dac_offset[ch]);
            }
            it++;
        }
    }
    applyingSettings = false;
}





