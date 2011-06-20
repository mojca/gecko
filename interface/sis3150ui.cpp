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

#include <iostream>
#include "sis3150ui.h"

Q_DECLARE_METATYPE(Sis3150UI::AddrMode);
Q_DECLARE_METATYPE(Sis3150UI::DataMode);

Sis3150UI::Sis3150UI(Sis3150Module* _module)
    : addrmode (A32)
    , datamode (D32)
    , module (_module)
    , name (_module->getName ())
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

    addrModeSpinner->addItem(tr("A32"), QVariant::fromValue (A32));
    dataModeSpinner->addItem(tr("D16"), QVariant::fromValue (D16));
    dataModeSpinner->addItem(tr("D32"), QVariant::fromValue (D32));
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
    bool ok;
    uint32_t addr = addrEdit->text().toUInt(&ok,16);

    if(!ok) { outputText(tr("Sis3150UI::readButtonClicked(): conversion failed.\n")); return; }

    switch (datamode) {
    case D32:
        {
            int ret = 0;
            uint32_t data = 0xC0FFEEEE;
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
            int ret = 0;
            uint16_t data = 0xDEAD;
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

void Sis3150UI::writeButtonClicked()
{
    bool ok;
    uint32_t addr = addrEdit->text().toUInt(&ok,16);
    uint32_t data = dataEdit->text().toUInt(&ok,16);
    int ret = 0;

    if(!ok) { outputText(tr("Sis3150UI::writeButtonClicked(): conversion failed.")); return; }

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

void Sis3150UI::outputText(QString text)
{
    statusViewTextEdit->textCursor().movePosition(QTextCursor::End);
    statusViewTextEdit->insertPlainText(text);
    statusViewTextEdit->ensureCursorVisible();
}

void Sis3150UI::addrModeChanged(int newIndex)
{
    QVariant mode = addrModeSpinner->itemData(newIndex);
    if (mode.isValid()) {
        addrmode = mode.value<AddrMode>();
        outputText(tr("Changed address mode to ") + (addrmode == A32 ? "A32" : "A24") + "\n");
    }
}

void Sis3150UI::dataModeChanged(int newIndex)
{
    QVariant mode = dataModeSpinner->itemData(newIndex);
    if (mode.isValid()) {
        datamode = mode.value<DataMode>();
        outputText(tr("Changed data mode to ") + (datamode == D32 ? "D32" : "D16") + "\n");
    }
}
