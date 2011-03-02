#include "sis3100ui.h"
#include "sis3100module.h"

#include <iostream>

Q_DECLARE_METATYPE(Sis3100UI::AddrMode);
Q_DECLARE_METATYPE(Sis3100UI::DataMode);

Sis3100UI::Sis3100UI(Sis3100Module* _module)
    : module(_module)
    , name(_module->getName())
    , addrmode(A32)
    , datamode(D32)
{
    createUI();
    std::cout << "Instantiated Sis3100 UI" << std::endl;
}

void Sis3100UI::createUI()
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

QWidget* Sis3100UI::createButtons()
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

QWidget* Sis3100UI::createStatusView()
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

QWidget* Sis3100UI::createVmeControl()
{
    vmeControl = new QGroupBox(tr("VME Control"));

    QGridLayout* layout = new QGridLayout();

    addrLabel = new QLabel(tr("Address:"));
    dataLabel = new QLabel(tr("Data:"));

    addrEdit = new QLineEdit();
    dataEdit = new QLineEdit();

    addrModeSpinner = new QComboBox();
    dataModeSpinner = new QComboBox();

    addrModeSpinner->addItem(tr("A32"), QVariant::fromValue(A32));
    dataModeSpinner->addItem(tr("D16"), QVariant::fromValue(D16));
    dataModeSpinner->addItem(tr("D32"), QVariant::fromValue(D32));
    addrModeSpinner->setCurrentIndex(0);
    dataModeSpinner->setCurrentIndex(1);

    connect(addrEdit,SIGNAL(returnPressed()),this,SLOT(readButtonClicked()));
    connect(dataEdit,SIGNAL(returnPressed()),this,SLOT(writeButtonClicked()));
    connect(addrModeSpinner,SIGNAL(currentIndexChanged(int)),this,SLOT(addrModeChanged(int)));
    connect(dataModeSpinner,SIGNAL(currentIndexChanged(int)),this,SLOT(dataModeChanged(int)));

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

void Sis3100UI::openCloseButtonClicked()
{
    if(module->isOpen() == false)
    {
        module->open();
        moduleOpened();
    }
    else
    {
        module->close();
        moduleClosed();
    }
}

void Sis3100UI::moduleOpened () {
    opencloseButton->setText(tr("Close"));
    resetButton->setEnabled(true);
    vmeControl->setEnabled(true);
}

void Sis3100UI::moduleClosed () {
    opencloseButton->setText(tr("Open"));
    resetButton->setEnabled(false);
    vmeControl->setEnabled(false);
}

void Sis3100UI::readButtonClicked()
{
    bool ok;
    uint32_t addr = addrEdit->text().toUInt(&ok,16);

    if(!ok) { outputText(tr("Sis3100UI::readButtonClicked(): conversion failed.\n")); return; }

    switch (datamode) {
    case D32:
        {
            int ret;
            uint32_t data;
            switch (addrmode) {
            case A32:
            case A24:
                ret = module->readA32D32 (addr,&data);
                break;
            }

            outputText(tr("Read: %1 (0x%2), Status: 0x%3 \n").arg(data,1,10).arg(data,1,16).arg(ret,1,16));
        }
        break;
    case D16:
        {
            int ret;
            uint16_t data;
            switch (addrmode) {
            case A32:
            case A24:
                ret = module->readA32D16(addr,&data);
                break;
            }

            outputText(tr("Read: %1 (0x%2), Status: 0x%3 \n").arg(data,1,10).arg(data,1,16).arg(ret,1,16));
        }
        break;
    }
}

void Sis3100UI::writeButtonClicked()
{
    bool ok;
    uint32_t addr = addrEdit->text().toUInt(&ok,16);
    uint32_t data = dataEdit->text().toUInt(&ok,16);
    int ret;

    if(!ok) { outputText(tr("Sis3100UI::writeButtonClicked(): conversion failed.")); return; }

    switch (datamode) {
    case D32:
        switch (addrmode) {
        case A32:
        case A24:
            ret = module->writeA32D32 (addr,data);
            break;
        }
        break;
    case D16:
        switch (addrmode) {
        case A32:
        case A24:
            ret = module->writeA32D16(addr,data);
            break;
        }
        break;
    }

    outputText(tr("Status: 0x%1 \n").arg(ret,1,16));
}

void Sis3100UI::outputText(QString text)
{
    statusViewTextEdit->insertPlainText(text);
    statusViewTextEdit->ensureCursorVisible();
}

void Sis3100UI::addrModeChanged(int newIndex)
{
    QVariant mode = addrModeSpinner->itemData(newIndex);
    if (mode.isValid()) {
        addrmode = mode.value<AddrMode>();
        outputText(tr("Changed address mode to ") + (addrmode == A32 ? "A32" : "A24") + "\n");
    }
}

void Sis3100UI::dataModeChanged(int newIndex)
{
    QVariant mode = dataModeSpinner->itemData(newIndex);
    if (mode.isValid()) {
        datamode = mode.value<DataMode>();
        outputText(tr("Changed data mode to ") + (datamode == D32 ? "D32" : "D16") + "\n");
    }
}
