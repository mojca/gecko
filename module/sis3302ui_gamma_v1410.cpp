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
    uif.addLineEditReadOnlyToGroup(tn[nt],gn[ng],"Module ID","module_id","unknown");
    uif.addLineEditReadOnlyToGroup(tn[nt],gn[ng],"Firmware","firmware","unknown");

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
    uif.addButtonToGroup(tn[nt],gn[ng]+"b0_","Trigger","start_button");
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
                               << "Arm, Busy, Trigger"
                               << "Arm, Veto, Trigger"
                               << "Neighbor+, Trigger, Neighbor-"
                               << "Neighbor+, Veto, Neighbor-"));
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
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Gate Length",tr("trigger_gate_length_%1").arg(ch),0,0x65535);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Pretrigger",tr("trigger_pretrigger_delay_%1").arg(ch),0,1023);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Int. Gate",tr("trigger_int_gate_length_%1").arg(ch),0,0xff);  // 8 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Int. Delay",tr("trigger_int_trg_delay_%1").arg(ch),0,0xff);  // 8 bits
                uif.addCheckBoxToGroup(tn[nt],un+gn[ng],"Ext. Gate",tr("enable_ext_gate_%1").arg(ch));
                uif.addCheckBoxToGroup(tn[nt],un+gn[ng],"Ext. Trigger",tr("enable_ext_trg_%1").arg(ch));
                uif.addCheckBoxToGroup(tn[nt],un+gn[ng],"Int. Gate",tr("enable_int_gate_%1").arg(ch));
                uif.addCheckBoxToGroup(tn[nt],un+gn[ng],"Int. Trigger",tr("enable_int_trg_%1").arg(ch));
                uif.addCheckBoxToGroup(tn[nt],un+gn[ng],"Disable Output",tr("disable_trg_out_%1").arg(ch));
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
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Gate Length",tr("energy_gate_length_%1").arg(ch),0,0xffff);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"RAW Length",tr("energy_sample_length_%1").arg(ch),0,65532);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Energy Length",tr("raw_sample_length_%1").arg(ch),0,0x1ffff);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Start Index 1",tr("energy_sample_start_idx_1_%1").arg(ch),0,0xffff);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Start Index 2",tr("energy_sample_start_idx_2_%1").arg(ch),0,0xffff);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Start Index 3",tr("energy_sample_start_idx_3_%1").arg(ch),0,0xffff);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Start Index RAW",tr("raw_sample_start_idx_%1").arg(ch),0,0xffff);  // 16 bits
                uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Tau",tr("energy_tau_%1").arg(ch),0,0xff);  // 8 bits
                uif.addCheckBoxToGroup(tn[nt],un+gn[ng],"Extra Filter",tr("enable_energy_extra_filter_%1").arg(ch));
                uif.addCheckBoxToGroup(tn[nt],un+gn[ng],"Invert Input",tr("enable_input_invert_%1").arg(ch));
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
    startStopPreviewButton = new QPushButton("Start");
    connect(startStopPreviewButton,SIGNAL(clicked()),this,SLOT(clicked_startStopPreviewButton()));
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
    int ch = 0;
    QGridLayout* l = new QGridLayout();
    for(int r = 0; r < nofRows; ++r) {
        for(int c = 0; c < nofCols; ++c) {
            previewCh[ch] = new plot2d(this,QSize(240,200),ch);
            previewCh[ch]->addChannel(0,"adc",previewData[ch],QColor(Qt::blue),Channel::line,1);
            l->addWidget(previewCh[ch],r,c,1,1);
            ++ch;
        }
    }
    previewWindow.setLayout(l);
    previewWindow.resize(640,480);
}

// Slot handling

void Sis3302V1410UI::uiInput(QString _name)
{
    if(applyingSettings == true) return;

    QGroupBox* gb = findChild<QGroupBox*>(_name);
    if(gb != 0)
    {
        if(_name.startsWith("ch_enabled")) {
            int ch = _name.right(1).toInt();
            if(gb->isChecked()) module->conf.enable_ch[ch] = true;
            else module->conf.enable_ch[ch] = false;
            printf("Changed ch_enabled %d\n",ch);
        }
    }

    QCheckBox* cb = findChild<QCheckBox*>(_name);
    if(cb != 0)
    {
        if(_name == "enable_irq") {
            if(cb->isChecked()) module->conf.enable_irq = true;
            else module->conf.enable_irq = false;
        }
        if(_name == "enable_external_trg") {
            if(cb->isChecked()) module->conf.enable_external_trg = true;
            else module->conf.enable_external_trg = false;
        }
        //QMessageBox::information(this,"uiInput","You changed the checkbox "+_name);
    }

    QComboBox* cbb = findChild<QComboBox*>(_name);
    if(cbb != 0)
    {
        if(_name == "acMode") module->conf.acMode = static_cast<Sis3302V1410config::AcMode>(cbb->currentIndex());
        if(_name == "clockSource") module->conf.clockSource = static_cast<Sis3302V1410config::ClockSource>(cbb->currentIndex());
        if(_name == "irqSource") module->conf.irqSource = static_cast<Sis3302V1410config::IrqSource>(cbb->currentIndex());
        if(_name == "irqMode") module->conf.irqMode = static_cast<Sis3302V1410config::IrqMode>(cbb->currentIndex());
        //QMessageBox::information(this,"uiInput","You changed the combobox "+_name);
    }
    QSpinBox* sb = findChild<QSpinBox*>(_name);
    if(sb != 0)
    {
        if(_name == "irq_level") module->conf.irq_level = sb->value();
        if(_name == "irq_vector"){
            module->conf.irq_vector = sb->value();
        }
        if(_name.startsWith("trigger_pulse_length_")) {
            int ch = _name.right(1).toInt();
            module->conf.trigger_pulse_length[ch] = sb->value();
        }
        if(_name.startsWith("trigger_gap_length_")) {
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
    }
    QPushButton* pb = findChild<QPushButton*>(_name);
    if(pb != 0)
    {
        if(_name == "start_button") clicked_start_button();
        //if(_name == "stop_button") clicked_stop_button();
        if(_name == "arm_button") clicked_arm_button();
        if(_name == "disarm_button") clicked_disarm_button();
        if(_name == "reset_button") clicked_reset_button();
        if(_name == "clear_button") clicked_clear_button();
        if(_name == "configure_button") clicked_configure_button();
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
        startStopPreviewButton->setText("Start");
        previewRunning = false;
    } else {
        startStopPreviewButton->setText("Stop");
        previewRunning = true;
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
            for(int ch=0; ch<8; ch++)
            {
                if(w->objectName() == tr("ch_enabled%1").arg(ch)) w->setChecked(module->conf.enable_ch[ch]);
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
            if(w->objectName() == "enable_external_trg") w->setChecked(module->conf.enable_external_trg);

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
            if(w->objectName() == "acMode") w->setCurrentIndex(module->conf.acMode);
            if(w->objectName() == "clockSource") w->setCurrentIndex(module->conf.clockSource);
            if(w->objectName() == "irqSource") w->setCurrentIndex(module->conf.irqSource);
            if(w->objectName() == "irqMode") w->setCurrentIndex(module->conf.irqMode);
            for(int ch=0; ch<8; ch++)
            {
                //if(w->objectName() == tr("trgMode_%1").arg(ch)) w->setCurrentIndex(module->conf.trgMode[ch]);
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

            for(int ch=0; ch<8; ch++)
            {
                if(w->objectName() == tr("trigger_pulse_length_%1").arg(ch)) w->setValue(module->conf.trigger_pulse_length[ch]);
                if(w->objectName() == tr("trigger_gap_length_%1").arg(ch)) w->setValue(module->conf.trigger_sumg_length[ch]);
                if(w->objectName() == tr("trigger_peak_length_%1").arg(ch)) w->setValue(module->conf.trigger_peak_length[ch]);
                if(w->objectName() == tr("trigger_threshold_%1").arg(ch)) w->setValue(module->conf.trigger_threshold[ch]);
                if(w->objectName() == tr("dac_offset_%1").arg(ch)) w->setValue(module->conf.dac_offset[ch]);
            }
            it++;
        }
    }
    applyingSettings = false;
}





