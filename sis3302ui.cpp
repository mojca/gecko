#include "sis3302ui.h"

Sis3302UI::Sis3302UI(Sis3302Module* _module)
    : BaseUI(_module)
{
    module = _module;

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
    addPopupToGroup(tn[nt],gn[ng],"Mode",(QStringList() << "Single Event" << "Multi Event"));

    gn.append("Advanced Setup"); ng++; addGroupToTab(tn[nt],gn[ng]);
    addCheckBoxToGroup(tn[nt],gn[ng],"Autostart Acquisition");
    addCheckBoxToGroup(tn[nt],gn[ng],"Internal Trigger as Stop");
    addCheckBoxToGroup(tn[nt],gn[ng],"ADC value as Big Endian");

    gn.append("Control"); ng++; addGroupToTab(tn[nt],gn[ng]);
    addUnnamedGroupToGroup(tn[nt],gn[ng],"b0_");
    addButtonToGroup(tn[nt],gn[ng]+"b0_","Start");
    addButtonToGroup(tn[nt],gn[ng]+"b0_","Stop");
    addUnnamedGroupToGroup(tn[nt],gn[ng],"b1_");
    addButtonToGroup(tn[nt],gn[ng]+"b1_","Arm");
    addButtonToGroup(tn[nt],gn[ng]+"b1_","Disarm");
    addUnnamedGroupToGroup(tn[nt],gn[ng],"b2_");
    addButtonToGroup(tn[nt],gn[ng]+"b2_","Reset");
    addButtonToGroup(tn[nt],gn[ng]+"b2_","Clear Timestamp");

    // TAB TRIGGER
    tn.append("Trigger 0-3"); nt++; addTab(tn[nt]);

    int ch = 0;
    for(int i=0; i<2; i++)
    {
        QString un = tr("noname_%1").arg(i);
        gn.append(un); ng++; addUnnamedGroupToTab(tn[nt],gn[ng]);
        for(int j=0; j<2; j++)
        {
            gn.append(tr("Channel %1").arg(ch)); ng++; addGroupToGroup(tn[nt],un,gn[ng]); ch++;
            addPopupToGroup(tn[nt],un+gn[ng],"Mode",(QStringList() << "LED, rising" << "LED, falling" << "FIR, rising" << "FIR, falling"));
            addSpinnerToGroup(tn[nt],un+gn[ng],"Threshold",0,0xffff); // 16 bits
            addSpinnerToGroup(tn[nt],un+gn[ng],"Sum time",1,16);
            addSpinnerToGroup(tn[nt],un+gn[ng],"Peak time",1,16);
            addSpinnerToGroup(tn[nt],un+gn[ng],"Pulse Length",0,0xff);  // 8 bits
        }
    }

    tn.append("Trigger 4-7"); nt++; addTab(tn[nt]);

    ch = 4;
    for(int i=0; i<2; i++)
    {
        QString un = tr("noname_%1").arg(i);
        gn.append(un); ng++; addUnnamedGroupToTab(tn[nt],gn[ng]);
        for(int j=0; j<2; j++)
        {
            gn.append(tr("Channel %1").arg(ch)); ng++; addGroupToGroup(tn[nt],un,gn[ng]); ch++;
            addPopupToGroup(tn[nt],un+gn[ng],"Mode",(QStringList() << "LED, rising" << "LED, falling" << "FIR, rising" << "FIR, falling"));
            addSpinnerToGroup(tn[nt],un+gn[ng],"Threshold",0,0xffff); // 16 bits
            addSpinnerToGroup(tn[nt],un+gn[ng],"Sum time",1,16);
            addSpinnerToGroup(tn[nt],un+gn[ng],"Peak time",1,16);
            addSpinnerToGroup(tn[nt],un+gn[ng],"Pulse Length",0,0xff);  // 8 bits
        }
    }

    // TAB EVENT
    tn.append("Event"); nt++; addTab(tn[nt]);

    gn.append("Delay and Length"); ng++; addGroupToTab(tn[nt],gn[ng]);
    addSpinnerToGroup(tn[nt],gn[ng],"Start Delay",0,0xffffff); // 24 bits
    addSpinnerToGroup(tn[nt],gn[ng],"Stop Delay",0,0xffffff); // 24 bits
    addSpinnerToGroup(tn[nt],gn[ng],"Number of Events",0,0x0fffff); // 24 bits
    addCheckBoxToGroup(tn[nt],gn[ng],"Event Length Stop Mode");
    addSpinnerToGroup(tn[nt],gn[ng],"Event Length",0,0x0fffff); // 24 bits

    gn.append("Advanced"); ng++; addGroupToTab(tn[nt],gn[ng]);
    addCheckBoxToGroup(tn[nt],gn[ng],"Page Wrap");
    addPopupToGroup(tn[nt],gn[ng],"Page Wrap Size",(QStringList() << "64" << "128" << "256" << "512" << "1k" << "4k" << "16k" << "64" << "256k" << "1M" << "4M" << "16M"));
    addPopupToGroup(tn[nt],gn[ng],"Averaging Mode",(QStringList() << "1" << "2" << "4" << "8" << "16" << "32" << "64" << "128"));

    // TAB DAC SETUP
    tn.append("DAC setup"); nt++; addTab(tn[nt]);

    gn.append("Offsets"); ng++; addGroupToTab(tn[nt],gn[ng]);
    for(int ch=0; ch<8; ch++)
    {
        addSpinnerToGroup(tn[nt],gn[ng],tr("DAC offset %1").arg(ch),0,0xffff);
    }

    //###

    l->addWidget(dynamic_cast<QWidget*>(&tabs));
    this->setLayout(l);

    connect(&sm,SIGNAL(mapped(QString)),this,SLOT(uiInput(QString)));

//    int i = 0;
//    QMap<QString,QWidget*>::iterator it = widgets.begin();
//    while(it != widgets.end())
//    {
//        QString s = it.key();
//        QWidget* w = it.value();
//        cout << i << "::: " << s.toStdString() << "::: "
//                << w->accessibleName().toStdString() << " is a "
//                << w->metaObject()->className() << endl;
//        i++;
//        it++;
//    }
}

void Sis3302UI::uiInput(QString _name)
{
    QMessageBox::information(this,"uiInput","You used "+_name);
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

void Sis3302UI::addGroupToTab(QString _tname, QString _name)
{
    if (tabsMap.contains(_tname)) {
        QWidget* c = tabsMap.value(_tname);
        QGroupBox* g = new QGroupBox(_name,c);
        QString identifier = _tname+_name;
        QGridLayout* l = new QGridLayout;
        l->setMargin(0);
        l->setVerticalSpacing(0);
        g->setLayout(l);
        g->setAccessibleName(identifier);
        c->layout()->addWidget(g);
        groups.insert(identifier,g);
        //cout << "Adding " << identifier.toStdString() << " to groups." << endl;
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
        g->setAccessibleName(identifier);
        c->layout()->addWidget(g);
        groups.insert(identifier,g);
        //cout << "Adding " << identifier.toStdString() << " to groups." << endl;
    }
}

void Sis3302UI::addGroupToGroup(QString _tname, QString _gname, QString _name)
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
        b->setAccessibleName(identifier);
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
        b->setAccessibleName(identifier);
    }
}
void Sis3302UI::addButtonToGroup(QString _tname, QString _gname, QString _name)
{
    QString identifier = _tname+_gname;
    if (groups.contains(identifier)) {
        QWidget* g = groups.value(identifier);
        QPushButton* b = new QPushButton(_name,g);
        g->layout()->addWidget(b);
        identifier += _name;
        sm.setMapping(b,identifier);
        widgets.insert(identifier,b);
        b->setAccessibleName(identifier);
        connect(b,SIGNAL(clicked()),&sm,SLOT(map()));
    }
}

void Sis3302UI::addSpinnerToGroup(QString _tname, QString _gname, QString _name, int min, int max)
{
    QString identifier = _tname+_gname;
    if (groups.contains(identifier)) {
        QWidget* g = groups.value(identifier);
        QSpinBox* b = new QSpinBox(g);
        b->setMinimum(min);
        b->setMaximum(max);
        QWidget* w = attachLabel(b,_name);
        g->layout()->addWidget(w);
        identifier += _name;
        sm.setMapping(b,identifier);
        widgets.insert(identifier,b);
        b->setAccessibleName(identifier);
        connect(b,SIGNAL(valueChanged(int)),&sm,SLOT(map()));
    }
}

void Sis3302UI::addDoubleSpinnerToGroup(QString _tname, QString _gname, QString _name, double min, double max)
{
    QString identifier = _tname+_gname;
    if (groups.contains(identifier)) {
        QWidget* g = groups.value(identifier);
        QDoubleSpinBox* b = new QDoubleSpinBox(g);
        b->setMinimum(min);
        b->setMaximum(max);
        QWidget* w = attachLabel(b,_name);
        g->layout()->addWidget(w);
        identifier += _name;
        sm.setMapping(b,identifier);
        widgets.insert(identifier,b);
        b->setAccessibleName(identifier);
        connect(b,SIGNAL(valueChanged(int)),&sm,SLOT(map()));
    }
}

void Sis3302UI::addCheckBoxToGroup(QString _tname, QString _gname, QString _name)
{
    QString identifier = _tname+_gname;
    if (groups.contains(identifier)) {
        QWidget* g = groups.value(identifier);
        QCheckBox* b = new QCheckBox(g);
        QWidget* w = attachLabel(b,_name);
        g->layout()->addWidget(w);
        identifier += _name;
        sm.setMapping(b,identifier);
        widgets.insert(identifier,b);
        b->setAccessibleName(identifier);
        connect(b,SIGNAL(stateChanged(int)),&sm,SLOT(map()));
    }
}

void Sis3302UI::addPopupToGroup(QString _tname, QString _gname, QString _name, QStringList _itNames)
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
        identifier += _name;
        sm.setMapping(b,identifier);
        widgets.insert(identifier,b);
        b->setAccessibleName(identifier);
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




