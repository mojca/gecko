#include "sis3302ui.h"

Sis3302UI::Sis3302UI(Sis3302Module* _module)
    : BaseUI(_module)
{
    module = _module;
    applyingSettings = false;

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
    tn.append("Acquisition"); addTab(tn[nt]);

    gn.append("Basic Setup"); addGroupToTab(tn[nt],gn[ng]);
    addPopupToGroup(tn[nt],gn[ng],"Mode","acMode",(QStringList() << "Single Event" << "Multi Event"));

    gn.append("Advanced Setup"); ng++; addGroupToTab(tn[nt],gn[ng]);
    addCheckBoxToGroup(tn[nt],gn[ng],"Autostart Acquisition","autostart_acq");
    addCheckBoxToGroup(tn[nt],gn[ng],"Use Internal Trigger","internal_trg_as_stop");
    addCheckBoxToGroup(tn[nt],gn[ng],"Use External Trigger (LEMO)","enable_external_trg");
    addCheckBoxToGroup(tn[nt],gn[ng],"ADC value as Big Endian","adc_value_big_endian");

    gn.append("Control"); ng++; addGroupToTab(tn[nt],gn[ng]);
    addUnnamedGroupToGroup(tn[nt],gn[ng],"b0_");
    addButtonToGroup(tn[nt],gn[ng]+"b0_","Start","start_button");
    addButtonToGroup(tn[nt],gn[ng]+"b0_","Stop","stop_button");
    addUnnamedGroupToGroup(tn[nt],gn[ng],"b1_");
    addButtonToGroup(tn[nt],gn[ng]+"b1_","Arm","arm_button");
    addButtonToGroup(tn[nt],gn[ng]+"b1_","Disarm","disarm_button");
    addUnnamedGroupToGroup(tn[nt],gn[ng],"b2_");
    addButtonToGroup(tn[nt],gn[ng]+"b2_","Reset","reset_button");
    addButtonToGroup(tn[nt],gn[ng]+"b2_","Clear Timestamp","clear_button");

    // TAB TRIGGER
    tn.append("Ch 0-3"); nt++; addTab(tn[nt]);

    int ch = 0;
    for(int i=0; i<2; i++)
    {
        QString un = tr("noname_%1").arg(i);
        gn.append(un); ng++; addUnnamedGroupToTab(tn[nt],gn[ng]);
        for(int j=0; j<2; j++)
        {
            gn.append(tr("Channel %1").arg(ch)); ng++; addGroupToGroup(tn[nt],un,gn[ng],tr("ch_enabled%1").arg(ch));
            addPopupToGroup(tn[nt],un+gn[ng],"Mode",tr("trgMode_%1").arg(ch),(QStringList() << "LED, rising" << "LED, falling" << "FIR, rising" << "FIR, falling"));
            addSpinnerToGroup(tn[nt],un+gn[ng],"Threshold",tr("trigger_threshold_%1").arg(ch),0,0xffff); // 16 bits
            addSpinnerToGroup(tn[nt],un+gn[ng],"Sum time",tr("trigger_gap_length_%1").arg(ch),1,16);
            addSpinnerToGroup(tn[nt],un+gn[ng],"Peak time",tr("trigger_peak_length_%1").arg(ch),1,16);
            addSpinnerToGroup(tn[nt],un+gn[ng],"Pulse Length",tr("trigger_pulse_length_%1").arg(ch),0,0xff);  // 8 bits
            ch++;
        }
    }

    tn.append("Ch 4-7"); nt++; addTab(tn[nt]);

    ch = 4;
    for(int i=0; i<2; i++)
    {
        QString un = tr("noname_%1").arg(i);
        gn.append(un); ng++; addUnnamedGroupToTab(tn[nt],gn[ng]);
        for(int j=0; j<2; j++)
        {
            gn.append(tr("Channel %1").arg(ch)); ng++; addGroupToGroup(tn[nt],un,gn[ng],tr("ch_enabled%1").arg(ch));
            addPopupToGroup(tn[nt],un+gn[ng],"Mode",tr("trgMode_%1").arg(ch),(QStringList() << "LED, rising" << "LED, falling" << "FIR, rising" << "FIR, falling"));
            addSpinnerToGroup(tn[nt],un+gn[ng],"Threshold",tr("trigger_threshold_%1").arg(ch),0,0xffff); // 16 bits
            addSpinnerToGroup(tn[nt],un+gn[ng],"Sum time",tr("trigger_gap_length_%1").arg(ch),1,16);
            addSpinnerToGroup(tn[nt],un+gn[ng],"Peak time",tr("trigger_peak_length_%1").arg(ch),1,16);
            addSpinnerToGroup(tn[nt],un+gn[ng],"Pulse Length",tr("trigger_pulse_length_%1").arg(ch),0,0xff);  // 8 bits
            ch++;
        }
    }

    // TAB EVENT
    tn.append("Event"); nt++; addTab(tn[nt]);

    gn.append("Delay and Length"); ng++; addGroupToTab(tn[nt],gn[ng]);
    addSpinnerToGroup(tn[nt],gn[ng],"Start Delay","start_delay",0,0xffffff); // 24 bits
    addSpinnerToGroup(tn[nt],gn[ng],"Stop Delay","stop_delay",0,0xffffff); // 24 bits
    addSpinnerToGroup(tn[nt],gn[ng],"Number of Events","nof_events",0,0x0fffff); // 24 bits
    addCheckBoxToGroup(tn[nt],gn[ng],"Event Length Stop Mode","event_length_as_stop");
    addSpinnerToGroup(tn[nt],gn[ng],"Event Length","event_length",0,0x0fffff); // 24 bits

    gn.append("Advanced"); ng++; addGroupToTab(tn[nt],gn[ng]);
    addCheckBoxToGroup(tn[nt],gn[ng],"Page Wrap","enable_page_wrap");
    addPopupToGroup(tn[nt],gn[ng],"Page Wrap Size","wrapSize",(QStringList() << "64" << "128" << "256" << "512" << "1k" << "4k" << "16k" << "64" << "256k" << "1M" << "4M" << "16M"));
    addPopupToGroup(tn[nt],gn[ng],"Averaging Mode","avgMode",(QStringList() << "1" << "2" << "4" << "8" << "16" << "32" << "64" << "128"));

    // TAB DAC SETUP
    tn.append("DAC setup"); nt++; addTab(tn[nt]);

    gn.append("Offsets"); ng++; addGroupToTab(tn[nt],gn[ng]);
    for(int ch=0; ch<8; ch++)
    {
        addSpinnerToGroup(tn[nt],gn[ng],tr("DAC offset %1").arg(ch),tr("dac_offset_%1").arg(ch),0,0xffff);
    }

    // TAB Clock and IRQ
    tn.append("Clock && IRQ"); nt++; addTab(tn[nt]);

    gn.append("Clock"); ng++; addGroupToTab(tn[nt],gn[ng]);
    addPopupToGroup(tn[nt],gn[ng],"Clock source","clockSource",(QStringList()
             << "Internal 100 MHz"
             << "Internal 50 MHz"
             << "Internal 25 MHz"
             << "Internal 10 MHz"
             << "Internal 1 MHz"
             << "random clock (from internal)"
             << "external clock (from LEMO)"
             << "Real 100 MHz internal"));

    gn.append("Interrupt Setup"); ng++; addGroupToTab(tn[nt],gn[ng]);
    addCheckBoxToGroup(tn[nt],gn[ng],"VME enable IRQ","enable_irq");
    addPopupToGroup(tn[nt],gn[ng],"IRQ Mode","irqMode",(QStringList() << "RORA" << "ROAK"));
    addPopupToGroup(tn[nt],gn[ng],"IRQ Source","irqSource",(QStringList() << "End of Event" << "End of Multi Event"));
    addSpinnerToGroup(tn[nt],gn[ng],"IRQ level","irq_level",0,7);
    addHexSpinnerToGroup(tn[nt],gn[ng],"IRQ vector","irq_vector",0,0xff);


    //###

    l->addWidget(dynamic_cast<QWidget*>(&tabs));
    this->setLayout(l);

    connect(&sm,SIGNAL(mapped(QString)),this,SLOT(uiInput(QString)));

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
            if(cb->isChecked()) module->conf.autostart_acq = true;
            else module->conf.autostart_acq = false;
        }
        if(_name == "event_length_as_stop") {
            if(cb->isChecked()) module->conf.autostart_acq = true;
            else module->conf.autostart_acq = false;
        }
        if(_name == "adc_value_big_endian") {
            if(cb->isChecked()) module->conf.autostart_acq = true;
            else module->conf.autostart_acq = false;
        }
        if(_name == "enable_page_wrap") {
            if(cb->isChecked()) module->conf.autostart_acq = true;
            else module->conf.autostart_acq = false;
        }
        if(_name == "enable_irq") {
            if(cb->isChecked()) module->conf.autostart_acq = true;
            else module->conf.autostart_acq = false;
        }
        if(_name == "enable_external_trg") {
            if(cb->isChecked()) module->conf.autostart_acq = true;
            else module->conf.autostart_acq = false;
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


// Generic UI constructor methods

void Sis3302UI::addTab(QString _name)
{
    QWidget* c = new QWidget();
    QGridLayout* l = new QGridLayout;
    l->setMargin(0);
    l->setVerticalSpacing(0);
    c->setLayout(l);
    int idx = tabs.addTab(c,_name);
    tabsMap.insert(_name,tabs.widget(idx));
}

void Sis3302UI::addGroupToTab(QString _tname, QString _name, QString _cname)
{
    if (tabsMap.contains(_tname)) {
        QWidget* c = tabsMap.value(_tname);
        QGroupBox* b = new QGroupBox(_name,b);
        QString identifier = _tname+_name;
        QGridLayout* l = new QGridLayout;
        l->setMargin(0);
        l->setVerticalSpacing(0);
        b->setLayout(l);
        c->layout()->addWidget(b);
        groups.insert(identifier,b);
        //cout << "Adding " << identifier.toStdString() << " to groups." << endl;
        if(!_cname.isEmpty())
        {
            b->setCheckable(true);
            b->setObjectName(_cname);
            widgets.insert(_cname,b);
            sm.setMapping(b,_cname);
            connect(b,SIGNAL(toggled(bool)),&sm,SLOT(map()));
        }
    }
}

void Sis3302UI::addUnnamedGroupToTab(QString _tname, QString _name)
{
    if (tabsMap.contains(_tname)) {
        QWidget* c = tabsMap.value(_tname);
        QWidget* g = new QWidget(c);
        QString identifier = _tname+_name;
        QHBoxLayout* l = new QHBoxLayout;
        l->setMargin(0);
        l->setSpacing(0);
        g->setLayout(l);
        g->setObjectName(identifier);
        c->layout()->addWidget(g);
        groups.insert(identifier,g);
        //cout << "Adding " << identifier.toStdString() << " to groups." << endl;
    }
}

void Sis3302UI::addGroupToGroup(QString _tname, QString _gname, QString _name, QString _cname)
{
    QString identifier = _tname+_gname;
    if (groups.contains(identifier)) {
        QWidget* g = groups.value(identifier);
        QGroupBox* b = new QGroupBox(_name,g);
        QGridLayout* l = new QGridLayout;
        l->setMargin(0);
        l->setVerticalSpacing(0);
        b->setLayout(l);
        g->layout()->addWidget(b);
        identifier += _name;
        groups.insert(identifier,b);
        //cout << "Adding " << identifier.toStdString() << " to groups." << endl;
        b->setObjectName(identifier);
        if(!_cname.isEmpty())
        {
            b->setCheckable(true);
            b->setObjectName(_cname);
            widgets.insert(_cname,b);
            sm.setMapping(b,_cname);
            connect(b,SIGNAL(toggled(bool)),&sm,SLOT(map()));
        }
    }
}

void Sis3302UI::addUnnamedGroupToGroup(QString _tname, QString _gname, QString _name)
{
    QString identifier = _tname+_gname;
    if (groups.contains(identifier)) {
        QWidget* g = groups.value(identifier);
        QWidget* b = new QWidget(g);
        QHBoxLayout* l = new QHBoxLayout;
        l->setMargin(0);
        l->setSpacing(0);
        b->setLayout(l);
        g->layout()->addWidget(b);
        identifier += _name;
        groups.insert(identifier,b);
        //cout << "Adding " << identifier.toStdString() << " to groups." << endl;
        b->setObjectName(identifier);
    }
}
void Sis3302UI::addButtonToGroup(QString _tname, QString _gname, QString _name, QString _cname)
{
    QString identifier = _tname+_gname;
    if (groups.contains(identifier)) {
        QWidget* g = groups.value(identifier);
        QPushButton* b = new QPushButton(_name,g);
        g->layout()->addWidget(b);
        widgets.insert(_cname,b);
        b->setObjectName(_cname);
        sm.setMapping(b,_cname);
        connect(b,SIGNAL(clicked()),&sm,SLOT(map()));
    }
}

void Sis3302UI::addSpinnerToGroup(QString _tname, QString _gname, QString _name, QString _cname, int min, int max)
{
    QString identifier = _tname+_gname;
    if (groups.contains(identifier)) {
        QWidget* g = groups.value(identifier);
        QSpinBox* b = new QSpinBox(g);
        b->setMinimum(min);
        b->setMaximum(max);
        QWidget* w = attachLabel(b,_name);
        g->layout()->addWidget(w);
        sm.setMapping(b,_cname);
        widgets.insert(_cname,b);
        b->setObjectName(_cname);
        connect(b,SIGNAL(valueChanged(int)),&sm,SLOT(map()));
    }
}

void Sis3302UI::addDoubleSpinnerToGroup(QString _tname, QString _gname, QString _name, QString _cname, double min, double max)
{
    QString identifier = _tname+_gname;
    if (groups.contains(identifier)) {
        QWidget* g = groups.value(identifier);
        QDoubleSpinBox* b = new QDoubleSpinBox(g);
        b->setMinimum(min);
        b->setMaximum(max);
        QWidget* w = attachLabel(b,_name);
        g->layout()->addWidget(w);
        sm.setMapping(b,_cname);
        widgets.insert(_cname,b);
        b->setObjectName(_cname);
        connect(b,SIGNAL(valueChanged(int)),&sm,SLOT(map()));
    }
}

void Sis3302UI::addHexSpinnerToGroup(QString _tname, QString _gname, QString _name, QString _cname, int min, int max)
{
    QString identifier = _tname+_gname;
    if (groups.contains(identifier)) {
        QWidget* g = groups.value(identifier);
        HexSpinBox* b = new HexSpinBox(g);
        b->setPrefix ("0x");
        b->setMinimum(min);
        b->setMaximum(max);
        QWidget* w = attachLabel(b,_name);
        g->layout()->addWidget(w);
        sm.setMapping(b,_cname);
        widgets.insert(_cname,b);
        b->setObjectName(_cname);
        connect(b,SIGNAL(valueChanged(int)),&sm,SLOT(map()));
    }
}

void Sis3302UI::addCheckBoxToGroup(QString _tname, QString _gname, QString _name, QString _cname)
{
    QString identifier = _tname+_gname;
    if (groups.contains(identifier)) {
        QWidget* g = groups.value(identifier);
        QCheckBox* b = new QCheckBox(g);
        QWidget* w = attachLabel(b,_name);
        g->layout()->addWidget(w);
        sm.setMapping(b,_cname);
        widgets.insert(_cname,b);
        b->setObjectName(_cname);
        connect(b,SIGNAL(stateChanged(int)),&sm,SLOT(map()));
    }
}

void Sis3302UI::addPopupToGroup(QString _tname, QString _gname, QString _name, QString _cname, QStringList _itNames)
{
    QString identifier = _tname+_gname;
    if (groups.contains(identifier)) {
        QWidget* g = groups.value(identifier);
        QComboBox* b = new QComboBox(g);
        QWidget* w = attachLabel(b,_name);
        g->layout()->addWidget(w);
        for(int i = 0; i<_itNames.size();i++)
        {
            QString _it = _itNames.at(i);
            b->addItem(_it,QVariant(i));
        }
        sm.setMapping(b,_cname);
        widgets.insert(_cname,b);
        b->setObjectName(_cname);
        connect(b,SIGNAL(currentIndexChanged(int)),&sm,SLOT(map()));
    }
}

QWidget* Sis3302UI::attachLabel(QWidget* w,QString _label)
{
    QLabel* lbl = new QLabel(_label);
    QHBoxLayout* l = new QHBoxLayout();
    QWidget* ret = new QWidget();
    l->setMargin(0);
    l->addWidget(lbl);
    l->addWidget(w);
    l->setSpacing(0);
    ret->setLayout(l);
    return ret;
}




