#include "dummyui.h"

DummyUI::DummyUI(DummyModule* _module)
    : BaseUI(dynamic_cast<BaseModule*>(_module))
{
    createUI();
    std::cout << "Instantiated Dummy UI" << std::endl;
}

void DummyUI::createUI()
{
    DummyModule* module = dynamic_cast<DummyModule*>(this->module);

    QGridLayout* l = new QGridLayout;

    QGroupBox* box = new QGroupBox;
    box->setTitle(name + " Settings");

        QGridLayout* boxL = new QGridLayout;

        QLabel* label = new QLabel("Here be dragons.");
        trigger = new QPushButton(tr("Trigger"));
        pollTrigger = new QPushButton(tr("Set Poll Trigger Flag"));
        connect(trigger,SIGNAL(clicked()),module,SLOT(emitTrigger()));
        connect(pollTrigger,SIGNAL(clicked()),module,SLOT(setPollTriggerFlag()));

        boxL->addWidget(label,0,0,1,1);
        boxL->addWidget(trigger,1,0,1,1);
        boxL->addWidget(pollTrigger,2,0,1,1);

        box->setLayout(boxL);

    l->addWidget(box);

    this->setLayout(l);
}


