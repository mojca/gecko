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

#include "sis3302ui.h"
#include <iostream>

Sis3302UI::Sis3302UI(Sis3302Module* _module)
    : module(_module), uif(this,&tabs), applyingSettings(false)
{
    createUI();
    std::cout << "Instantiated Sis3302 UI" << std::endl;
}

Sis3302UI::~Sis3302UI()
{

}

void Sis3302UI::createUI()
{
    QGridLayout* l = new QGridLayout;
    l->setMargin(0);
    l->setVerticalSpacing(0);

    int nt = 0; // current tab number
    int ng = 0; // current group number

    // TAB ACQUISITION
    tn.append("Acquisition"); uif.addTab(tn[nt]);

    gn.append("Basic Setup"); uif.addGroupToTab(tn[nt],gn[ng]);
    uif.addPopupToGroup(tn[nt],gn[ng],"Mode","acMode",(QStringList() << "Single Event" << "Multi Event"));

    gn.append("Advanced Setup"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    uif.addCheckBoxToGroup(tn[nt],gn[ng],"Autostart Acquisition","autostart_acq");
    uif.addCheckBoxToGroup(tn[nt],gn[ng],"Use Internal Trigger","internal_trg_as_stop");
    uif.addCheckBoxToGroup(tn[nt],gn[ng],"Use External Trigger (LEMO)","enable_external_trg");
    uif.addCheckBoxToGroup(tn[nt],gn[ng],"ADC value as Big Endian","adc_value_big_endian");

    gn.append("Control"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    uif.addUnnamedGroupToGroup(tn[nt],gn[ng],"b0_");
    uif.addButtonToGroup(tn[nt],gn[ng]+"b0_","Start","start_button");
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
    uif.getWidgets()->find("singleshot_button").value()->setEnabled(false);

    // TAB TRIGGER
    tn.append("Ch 0-3"); nt++; uif.addTab(tn[nt]);

    int ch = 0;
    for(int i=0; i<2; i++)
    {
        QString un = tr("noname_%1").arg(i);
        gn.append(un); ng++; uif.addUnnamedGroupToTab(tn[nt],gn[ng]);
        for(int j=0; j<2; j++)
        {
            gn.append(tr("Channel %1").arg(ch)); ng++; uif.addGroupToGroup(tn[nt],un,gn[ng],tr("ch_enabled%1").arg(ch));
            uif.addPopupToGroup(tn[nt],un+gn[ng],"Mode",tr("trgMode_%1").arg(ch),(QStringList() << "LED, rising" << "LED, falling" << "FIR, rising" << "FIR, falling"));
            uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Threshold",tr("trigger_threshold_%1").arg(ch),0,0x1ffff); // 17 bits
            uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Sum time",tr("trigger_gap_length_%1").arg(ch),1,16);
            uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Peak time",tr("trigger_peak_length_%1").arg(ch),1,16);
            uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Pulse Length",tr("trigger_pulse_length_%1").arg(ch),0,0xff);  // 8 bits
            ch++;
        }
    }

    tn.append("Ch 4-7"); nt++; uif.addTab(tn[nt]);

    ch = 4;
    for(int i=0; i<2; i++)
    {
        QString un = tr("noname_%1").arg(i);
        gn.append(un); ng++; uif.addUnnamedGroupToTab(tn[nt],gn[ng]);
        for(int j=0; j<2; j++)
        {
            gn.append(tr("Channel %1").arg(ch)); ng++; uif.addGroupToGroup(tn[nt],un,gn[ng],tr("ch_enabled%1").arg(ch));
            uif.addPopupToGroup(tn[nt],un+gn[ng],"Mode",tr("trgMode_%1").arg(ch),(QStringList() << "LED, rising" << "LED, falling" << "FIR, rising" << "FIR, falling"));
            uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Threshold",tr("trigger_threshold_%1").arg(ch),0,0xffff); // 16 bits
            uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Sum time",tr("trigger_gap_length_%1").arg(ch),1,16);
            uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Peak time",tr("trigger_peak_length_%1").arg(ch),1,16);
            uif.addSpinnerToGroup(tn[nt],un+gn[ng],"Pulse Length",tr("trigger_pulse_length_%1").arg(ch),0,0xff);  // 8 bits
            ch++;
        }
    }

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
    uif.addPopupToGroup(tn[nt],gn[ng],"Page Wrap Size","wrapSize",(QStringList() << "64" << "128" << "256" << "512" << "1k" << "4k" << "16k" << "64" << "256k" << "1M" << "4M" << "16M"));
    uif.addPopupToGroup(tn[nt],gn[ng],"Averaging Mode","avgMode",(QStringList() << "1" << "2" << "4" << "8" << "16" << "32" << "64" << "128"));

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
             << "random clock (from internal)"
             << "external clock (from LEMO)"
             << "Real 100 MHz internal"));

    gn.append("Interrupt Setup"); ng++; uif.addGroupToTab(tn[nt],gn[ng],"","v");
    uif.addCheckBoxToGroup(tn[nt],gn[ng],"VME enable IRQ","enable_irq");
    uif.addPopupToGroup(tn[nt],gn[ng],"IRQ Mode","irqMode",(QStringList() << "RORA" << "ROAK"));
    uif.addPopupToGroup(tn[nt],gn[ng],"IRQ Source","irqSource",(QStringList() << "End of Event" << "End of Multi Event"));
    uif.addSpinnerToGroup(tn[nt],gn[ng],"IRQ level","irq_level",0,7);
    uif.addHexSpinnerToGroup(tn[nt],gn[ng],"IRQ vector","irq_vector",0,0xff);


    //###

    l->addWidget(dynamic_cast<QWidget*>(&tabs));
    this->setLayout(l);
    connect(uif.getSignalMapper(),SIGNAL(mapped(QString)),this,SLOT(uiInput(QString)));

//    QList<QWidget*> li = this->findChildren<QWidget*>();
//    foreach(QWidget* w, li)
//    {
//        printf("%s\n",w->objectName().toStdString().c_str());
//    }
}

// Slot handling

void Sis3302UI::uiInput(QString _name)
{
    if(applyingSettings == true) return;

    QGroupBox* gb = findChild<QGroupBox*>(_name);
    if(gb != 0)
    {
        if(_name.startsWith("ch_enabled")) {
            int ch = _name.right(1).toInt();
            if(gb->isChecked()) module->conf.ch_enabled[ch] = true;
            else module->conf.ch_enabled[ch] = false;
            printf("Changed ch_enabled %d\n",ch);
        }
    }

    QCheckBox* cb = findChild<QCheckBox*>(_name);
    if(cb != 0)
    {
        if(_name == "autostart_acq") {
            if(cb->isChecked()) module->conf.autostart_acq = true;
            else module->conf.autostart_acq = false;
        }
        if(_name == "internal_trg_as_stop") {
            if(cb->isChecked()) module->conf.internal_trg_as_stop = true;
            else module->conf.internal_trg_as_stop = false;
        }
        if(_name == "event_length_as_stop") {
            if(cb->isChecked()) module->conf.event_length_as_stop = true;
            else module->conf.event_length_as_stop = false;
        }
        if(_name == "adc_value_big_endian") {
            if(cb->isChecked()) module->conf.adc_value_big_endian = true;
            else module->conf.adc_value_big_endian = false;
        }
        if(_name == "enable_page_wrap") {
            if(cb->isChecked()) module->conf.enable_page_wrap = true;
            else module->conf.enable_page_wrap = false;
        }
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
        if(_name == "acMode") module->conf.acMode = static_cast<Sis3302config::AcMode>(cbb->currentIndex());
        if(_name == "wrapSize") module->conf.wrapSize = static_cast<Sis3302config::WrapSize>(cbb->currentIndex());
        if(_name == "avgMode") module->conf.avgMode = static_cast<Sis3302config::AvgMode>(cbb->currentIndex());
        if(_name == "clockSource") module->conf.clockSource = static_cast<Sis3302config::ClockSource>(cbb->currentIndex());
        if(_name == "irqSource") module->conf.irqSource = static_cast<Sis3302config::IrqSource>(cbb->currentIndex());
        if(_name == "irqMode") module->conf.irqMode = static_cast<Sis3302config::IrqMode>(cbb->currentIndex());
        if(_name.startsWith("trgMode_")) {
            int ch = _name.right(1).toInt();
            module->conf.trgMode[ch] = static_cast<Sis3302config::TrgMode>(cbb->currentIndex());
        }
        //QMessageBox::information(this,"uiInput","You changed the combobox "+_name);
    }
    QSpinBox* sb = findChild<QSpinBox*>(_name);
    if(sb != 0)
    {
        if(_name == "event_length") module->conf.event_length = sb->value();
        if(_name == "start_delay") module->conf.start_delay = sb->value();
        if(_name == "stop_delay") module->conf.stop_delay = sb->value();
        if(_name == "nof_events") module->conf.nof_events = sb->value();
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
            module->conf.trigger_gap_length[ch] = sb->value();
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
        if(_name == "stop_button") clicked_stop_button();
        if(_name == "arm_button") clicked_arm_button();
        if(_name == "disarm_button") clicked_disarm_button();
        if(_name == "reset_button") clicked_reset_button();
        if(_name == "clear_button") clicked_clear_button();
        if(_name == "configure_button") clicked_configure_button();
    }
}

void Sis3302UI::clicked_start_button()
{
    module->start_sampling();
}

void Sis3302UI::clicked_stop_button()
{
    module->stop_sampling();
}

void Sis3302UI::clicked_arm_button()
{
    module->arm();
}

void Sis3302UI::clicked_disarm_button()
{
    module->disarm();
}

void Sis3302UI::clicked_reset_button()
{
    module->reset();
}

void Sis3302UI::clicked_clear_button()
{
    module->timestamp_clear();
}

void Sis3302UI::clicked_configure_button()
{
    module->configure();
}



// Settings handling

void Sis3302UI::applySettings()
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
                if(w->objectName() == tr("ch_enabled%1").arg(ch)) w->setChecked(module->conf.ch_enabled[ch]);
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

            if(w->objectName() == "autostart_acq") w->setChecked(module->conf.autostart_acq);
            if(w->objectName() == "internal_trg_as_stop") w->setChecked(module->conf.internal_trg_as_stop);
            if(w->objectName() == "event_length_as_stop") w->setChecked(module->conf.event_length_as_stop);
            if(w->objectName() == "adc_value_big_endian") w->setChecked(module->conf.adc_value_big_endian);
            if(w->objectName() == "enable_page_wrap") w->setChecked(module->conf.enable_page_wrap);
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
            if(w->objectName() == "wrapSize") w->setCurrentIndex(module->conf.wrapSize);
            if(w->objectName() == "avgMode") w->setCurrentIndex(module->conf.avgMode);
            if(w->objectName() == "clockSource") w->setCurrentIndex(module->conf.clockSource);
            if(w->objectName() == "irqSource") w->setCurrentIndex(module->conf.irqSource);
            if(w->objectName() == "irqMode") w->setCurrentIndex(module->conf.irqMode);
            for(int ch=0; ch<8; ch++)
            {
                if(w->objectName() == tr("trgMode_%1").arg(ch)) w->setCurrentIndex(module->conf.trgMode[ch]);
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
            if(w->objectName() == "event_length") w->setValue(module->conf.event_length);
            if(w->objectName() == "start_delay") w->setValue(module->conf.start_delay);
            if(w->objectName() == "stop_delay") w->setValue(module->conf.stop_delay);
            if(w->objectName() == "nof_events") w->setValue(module->conf.nof_events);
            if(w->objectName() == "irq_level") w->setValue(module->conf.irq_level);
            if(w->objectName() == "irq_vector") w->setValue(module->conf.irq_vector);

            for(int ch=0; ch<8; ch++)
            {
                if(w->objectName() == tr("trigger_pulse_length_%1").arg(ch)) w->setValue(module->conf.trigger_pulse_length[ch]);
                if(w->objectName() == tr("trigger_gap_length_%1").arg(ch)) w->setValue(module->conf.trigger_gap_length[ch]);
                if(w->objectName() == tr("trigger_peak_length_%1").arg(ch)) w->setValue(module->conf.trigger_peak_length[ch]);
                if(w->objectName() == tr("trigger_threshold_%1").arg(ch)) w->setValue(module->conf.trigger_threshold[ch]);
                if(w->objectName() == tr("dac_offset_%1").arg(ch)) w->setValue(module->conf.dac_offset[ch]);
            }
            it++;
        }
    }
    applyingSettings = false;
}





