#include <iostream>
#include "sis3150ui.h"

Sis3150UI::Sis3150UI(Sis3150Module* _module)
    : BaseUI(dynamic_cast<BaseModule*>(_module))
{
    createUI();
    std::cout << "Instantiated Sis3150 UI" << std::endl;
}

void Sis3150UI::createUI()
{
    QGridLayout* l = new QGridLayout;
    QGridLayout* boxL = new QGridLayout;

    QGroupBox* box = new QGroupBox;
    box->setTitle(name + " Settings");

    // Module specific code here
    QWidget *buttons;
    QWidget *status;
    QWidget *vmecontrol;

    buttons = createButtons();
    status = createStatusView();
    vmecontrol = createVmeControl();

    boxL->addWidget(buttons,0,0,1,1);
    boxL->addWidget(status,0,1,1,1);
    boxL->addWidget(vmecontrol,1,0,1,2);
    // End

    l->addWidget(box,0,0,1,1);
    box->setLayout(boxL);
    this->setLayout(l);
}

QWidget* Sis3150UI::createButtons()
{
    QWidget *buttons = new QGroupBox(tr("Device control"));

    QVBoxLayout *layout = new QVBoxLayout();

    opencloseButton = new QPushButton(tr("Open"));
    opencloseButton->setMinimumWidth(50);

    resetButton = new QPushButton(tr("Reset"));
    resetButton->setEnabled(false);
    resetButton->setMinimumWidth(50);

    connect(opencloseButton,SIGNAL(clicked()),this,SLOT(openCloseButtonClicked()));

    layout->addWidget(opencloseButton);
    layout->addWidget(resetButton);
    layout->addStretch(10);
    layout->setMargin(1);

    buttons->setLayout(layout);

    return buttons;
}

QWidget* Sis3150UI::createStatusView()
{
    QWidget *statusView = new QGroupBox(tr("Device output"));

    QVBoxLayout *layout = new QVBoxLayout();

    statusViewTextEdit = new QTextEdit();
    statusViewTextEdit->setReadOnly(true);

    layout->addWidget(statusViewTextEdit);
    layout->setMargin(1);

    statusView->setLayout(layout);

    return statusView;
}

QWidget* Sis3150UI::createVmeControl()
{
    vmeControl = new QGroupBox(tr("VME Control"));

    QGridLayout* layout = new QGridLayout();

    addrLabel = new QLabel(tr("Address:"));
    dataLabel = new QLabel(tr("Data:"));

    addrEdit = new QLineEdit();
    dataEdit = new QLineEdit();

    addrModeSpinner = new QComboBox();
    dataModeSpinner = new QComboBox();

    addrModeSpinner->addItem(tr("A32"));
    dataModeSpinner->addItem(tr("D16"));
    dataModeSpinner->addItem(tr("D32"));
    addrModeSpinner->setCurrentIndex(0);
    dataModeSpinner->setCurrentIndex(1);

    connect(addrEdit,SIGNAL(returnPressed()),this,SLOT(readButtonClicked()));
    connect(dataEdit,SIGNAL(returnPressed()),this,SLOT(writeButtonClicked()));
    connect(addrModeSpinner,SIGNAL(currentIndexChanged(QString)),this,SLOT(modeChanged(QString)));
    connect(dataModeSpinner,SIGNAL(currentIndexChanged(QString)),this,SLOT(modeChanged(QString)));

    readButton = new QPushButton(tr("VME Single Read"));
    writeButton = new QPushButton(tr("VME Single Write"));

    connect(readButton,SIGNAL(clicked()),this,SLOT(readButtonClicked()));
    connect(writeButton,SIGNAL(clicked()),this,SLOT(writeButtonClicked()));

    layout->addWidget(addrLabel,  0,0,1,1);
    layout->addWidget(dataLabel,  1,0,1,1);
    layout->addWidget(addrEdit,   0,1,1,1);
    layout->addWidget(dataEdit,   1,1,1,1);
    layout->addWidget(addrModeSpinner,   0,2,1,1);
    layout->addWidget(dataModeSpinner,   1,2,1,1);
    layout->addWidget(readButton, 0,3,1,1);
    layout->addWidget(writeButton,1,3,1,1);

    vmeControl->setLayout(layout);
    vmeControl->setEnabled(false);
    return vmeControl;
}

void Sis3150UI::openCloseButtonClicked()
{
    Sis3150Module* module = dynamic_cast<Sis3150Module*>(this->module);

    if(module->isOpen() == false)
    {
        module->open();
    }
    else
    {
        module->close();
    }
}

void Sis3150UI::moduleOpened () {
    opencloseButton->setText(tr("Close"));
    resetButton->setEnabled(true);
    vmeControl->setEnabled(true);
}

void Sis3150UI::moduleClosed () {
    opencloseButton->setText(tr("Open"));
    resetButton->setEnabled(false);
    vmeControl->setEnabled(false);
}

void Sis3150UI::readButtonClicked()
{
    Sis3150Module* module = dynamic_cast<Sis3150Module*>(this->module);

    bool ok;
    uint32_t addr = addrEdit->text().toUInt(&ok,16);
    uint32_t data = 0;

    if(!ok) std::cout << "Sis3150UI::readButtonClicked(): conversion failed." << std::endl;

    int ret = module->vmeSingleRead(addr,&data);

    outputText(tr("Read: %1, Status: %2 \n").arg(data,8,16).arg(ret,8,16));
}

void Sis3150UI::writeButtonClicked()
{
    Sis3150Module* module = dynamic_cast<Sis3150Module*>(this->module);

    bool ok;
    uint32_t addr = addrEdit->text().toUInt(&ok,16);
    uint32_t data = dataEdit->text().toUInt(&ok,16);

    if(!ok) std::cout << "Sis3150UI::writeButtonClicked(): conversion failed." << std::endl;

    int ret = module->vmeSingleWrite(addr,data);

    outputText(tr("Status: %1 \n").arg(ret,8,16));
}

void Sis3150UI::outputText(QString text)
{
    statusViewTextEdit->insertPlainText(text);
}

void Sis3150UI::modeChanged(QString newMode)
{
    Sis3150Module* module = dynamic_cast<Sis3150Module*>(this->module);

    if(newMode.startsWith('A'))
    {
        module->setAddrMode(newMode);
        outputText(tr("Changed address mode to ")+newMode+"\n");
    }
    else if(newMode.startsWith('D'))
    {
        module->setDataMode(newMode);
        outputText(tr("Changed data mode to ")+newMode+"\n");
    }
}
