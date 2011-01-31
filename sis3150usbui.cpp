#include "sis3150usbui.h"

Sis3150usbUI::Sis3150usbUI(Sis3150control *sis)
{
    this->sis = sis;
    deviceOpen = false;

    createUI();
}

void Sis3150usbUI::createUI()
{
    QHBoxLayout *layout = new QHBoxLayout();
    QWidget *buttons;
    QWidget *status;

    buttons = createButtons();
    status = createStatusView();

    layout->setMargin(1);
    layout->addWidget(buttons);
    layout->addWidget(status);
    this->setLayout(layout);
}

QWidget* Sis3150usbUI::createButtons()
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
    layout->setMargin(1);

    buttons->setLayout(layout);

    return buttons;
}

QWidget* Sis3150usbUI::createStatusView()
{
    QWidget *statusView = new QGroupBox(tr("Device output"));

    QVBoxLayout *layout = new QVBoxLayout();

    statusViewTextEdit = new QTextEdit();
    statusViewTextEdit->setReadOnly(true);

    connect(sis,SIGNAL(outputText(QString&)),this,SLOT(outputText(QString&)));

    layout->addWidget(statusViewTextEdit);
    layout->setMargin(1);

    statusView->setLayout(layout);

    return statusView;
}

void Sis3150usbUI::openCloseButtonClicked()
{
    if(deviceOpen == false)
    {
        int ret = sis->open();
        if(ret == 0)
        {
            opencloseButton->setText(tr("Close"));
            resetButton->setEnabled(true);
            emit deviceOpened();
        }
    }
    else
    {
        int ret = sis->close();
        if(ret == 0)
        {
            opencloseButton->setText(tr("Open"));
            resetButton->setEnabled(false);
            emit deviceClosed();
        }
    }
}

void Sis3150usbUI::outputText(QString & text)
{
    statusViewTextEdit->insertPlainText(text);
}
