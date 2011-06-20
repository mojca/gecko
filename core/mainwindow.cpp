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

#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //! Ideas
    //  - Add status bar with fill status of buffers and state of threads

//    this->deviceOpen = false;

    c = new ConfigManager();
    c->readConfig("3350scope.conf");

    sis3150 = new Sis3150control();
    sis = new Sis3350control();

    createUI();

    setupPostProcessWindow();

    freeRunnerTimer = new QTimer(this);
    connect(freeRunnerTimer,SIGNAL(timeout()),this,SLOT(singleShotClicked()));
    freeRunnerTimer->setInterval(0);
}

void MainWindow::createUI()
{
    tabs = new QTabWidget(this);

    sis3150usbUI = new Sis3150usbUI(this->sis3150);
    connect(sis3150usbUI,SIGNAL(deviceOpened()),this,SLOT(connectDevicesTo3150()));

    sis3350UI = new Sis3350UI(this->sis);

    tabs->addTab(sis3150usbUI,tr("sis3150"));
    tabs->addTab(sis3350UI,   tr("sis3350"));

    this->setCentralWidget(tabs);

}

void MainWindow::setupPostProcessWindow()
{
    ppw = new PostProcessWindow(sis);
    connect(ppw,SIGNAL(ppChanged()),this,SLOT(singleShotClicked()));
}

//void MainWindow::singleShotClicked()
//{
//    sis->reset();
//
//    // Only do one acquisition and do not save files
//    unsigned int temp = sis->conf.nof_reads;
//    sis->conf.nof_reads = 1;
//    sis->conf.writeEnabled[0] = false;
//    sis->conf.writeEnabled[1] = false;
//    sis->conf.writeEnabled[2] = false;
//    sis->conf.writeEnabled[3] = false;
//    unsigned int tempPollcount = sis->conf.pollcount;
//    unsigned int tempMultiEventCount = sis->conf.multievent_max_nof_events;
//    sis->conf.pollcount = 1000;
//   // sis->conf.multievent_max_nof_events = 1;
//
//    // Do the readout
//    sis->configure();
//    sis->acquisition();
//    if(ppCheck->isChecked()) ppw->postProcess();
//
//    // Reset values
//    sis->conf.nof_reads = temp;
//    sis->conf.pollcount = tempPollcount;
//    sis->conf.multievent_max_nof_events = tempMultiEventCount;
//    if(writeEnable->at(0)->isChecked()) sis->conf.writeEnabled[0] = true;
//    if(writeEnable->at(1)->isChecked()) sis->conf.writeEnabled[1] = true;
//    if(writeEnable->at(2)->isChecked()) sis->conf.writeEnabled[2] = true;
//    if(writeEnable->at(3)->isChecked()) sis->conf.writeEnabled[3] = true;
//    viewport->update();
//}
//
//void MainWindow::freeRunningButtonClicked()
//{
//    static bool isRunning = false;
//
//    if(isRunning == false)
//    {
//        isRunning = true;
//        freeRunnerTimer->start();
//    }
//    else
//    {
//        isRunning = false;
//        freeRunnerTimer->stop();
//    }
//}
//
//void MainWindow::acquisitionButtonClicked()
//{
//    sis->reset();
//    sis->configure();
//    sis->acquisition();
//    if(ppCheck->isChecked()) ppw->postProcess();
//    viewport->update();
//}
//
//void MainWindow::gainChanged(int ch)
//{
//    int newValue = gains->at(ch)->value();
//    sis->conf.variable_gain[ch] = newValue;
//    this->singleShotClicked();
//}
//
//void MainWindow::offChanged(int ch)
//{
//    int newValue = offsets->at(ch)->value();
//    sis->conf.adc_offset[ch] = newValue;
//    this->singleShotClicked();
//}
//
//void MainWindow::thrChanged(int ch)
//{
//    int newValue = thresholds->at(ch)->value();
//    sis->conf.trigger_threshold[ch] = newValue;
//    this->singleShotClicked();
//}
//
//void MainWindow::gateChanged(int ch)
//{
//    int newValue = gates->at(ch)->value();
//    sis->conf.trigger_gap_length[ch] = newValue;
//    this->singleShotClicked();
//}
//
//void MainWindow::peakChanged(int ch)
//{
//    int newValue = peaks->at(ch)->value();
//    sis->conf.trigger_peak_length[ch] = newValue;
//    this->singleShotClicked();
//}
//
//void MainWindow::enableChanged(int ch)
//{
//    bool newValue = triggerEnable->at(ch)->isChecked();
//    sis->conf.trigger_enable[ch] = newValue;
//    this->singleShotClicked();
//}
//
//void MainWindow::firChanged(int ch)
//{
//    bool newValue = triggerFir->at(ch)->isChecked();
//    sis->conf.trigger_fir[ch] = newValue;
//    this->singleShotClicked();
//}
//
//void MainWindow::gtChanged(int ch)
//{
//    bool newValue = triggerGt->at(ch)->isChecked();
//    sis->conf.trigger_gt[ch] = newValue;
//    this->singleShotClicked();
//}
//
//void MainWindow::writeEnableChanged(int ch)
//{
//    bool newValue = writeEnable->at(ch)->isChecked();
//    sis->conf.writeEnabled[ch] = newValue;
//}
//
//void MainWindow::nofEventsChanged()
//{
//    int newValue = nofEventSpinner->value();
//    sis->conf.nof_reads = newValue;
//    //printf("Changed nof_reads to %d\n",sis->conf.nof_reads);
//}
//
//void MainWindow::nofMultiEventsChanged()
//{
//    int newValue = nofMultiEventSpinner->value();
//    sis->conf.multievent_max_nof_events = newValue;
//    //printf("Changed nof_multievents to %d\n",sis->conf.multievent_max_nof_events);
//}
//
//void MainWindow::pretriggerChanged()
//{
//    int newValue = pretriggerDelaySpinner->value();
//    sis->conf.pre_delay = newValue;
//    this->singleShotClicked();
//}
//
//void MainWindow::postProcessChanged()
//{
//    if(ppCheck->isChecked())
//    {
//        ppw->show();
//    }
//    else if(ppw != NULL)
//    {
//        ppw->hide();
//    }
//}
//
//void MainWindow::clockChanged()
//{
//    char buf[100];
//    int newValue = clock1Spinner->value();
//    sis->conf.clock1 = newValue;
//    int tmp = 25*newValue;
//    newValue = clock2Spinner->value();
//    sis->conf.clock2 = newValue;
//    switch(newValue)
//    {
//        case 1:
//            tmp /= 2; break;
//        case 2:
//            tmp /= 4; break;
//        case 3:
//            tmp /= 8; break;
//    }
//    sprintf(buf,"%d MHz",tmp);
//    this->clockFrequencyLabel->setText(tr(buf));
//    this->singleShotClicked();
//}
//
//void MainWindow::sampleLengthChanged()
//{
//    int newValue = sampleLengthSpinner->value();
//    newValue &= ~(0x7);
//    if(newValue < 16) newValue = 16;
//    sampleLengthSpinner->setValue(newValue);
//    sis->conf.sample_length = newValue;
//    viewport->changeSampleLength(newValue);
//    this->singleShotClicked();
//}
//
//void MainWindow::filePathChanged()
//{
//    char newValue[200];
//    strncpy(newValue,filePathLineEdit->text().toLatin1().constData(),199);
//    strncpy(sis->conf.filePath,newValue,199);
//}
//
//void MainWindow::filePointSkipChanged()
//{
//    int newValue = filePointSkipSpinner->value();
//    sis->conf.filePointSkip = newValue;
//}

void MainWindow::connectDevicesTo3150()
{
    sis->setDevice(sis3150->getDevice());
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    if(ppw != NULL)
    {
        ppw->close();
        delete ppw;
        ppw = NULL;
    }
    ev->accept();
}

MainWindow::~MainWindow()
{
    int ret = 0;
    if(sis3150->isOpen() == true)
    {
        ret = sis->close();
//        if(ret == 0)
//        {
//            this->deviceOpen = false;
//            this->openclose->setText(tr("Open"));
//        }
    }
    if(ppw != NULL)
    {
        ppw->close();
        delete ppw;
        ppw = NULL;
    }
    freeRunnerTimer->stop();
    delete freeRunnerTimer;
}
