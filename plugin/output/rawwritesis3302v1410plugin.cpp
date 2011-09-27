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

#include "rawwritesis3302v1410plugin.h"
#include "pluginmanager.h"
#include "runmanager.h"
#include "pluginconnectorqueued.h"

static PluginRegistrar registrar ("rawwritesis3302v1410", RawWriteSis3302v1410Plugin::create, AbstractPlugin::GroupOutput);

RawWriteSis3302v1410Plugin::RawWriteSis3302v1410Plugin(int _id, QString _name)
    : BasePlugin(_id, _name)
{
    prefix = tr("vector");
    intervalMode = false;
    settings_changed = false;
    interval_minutes = 2;
    interval_number = 0;
    nof_events = 100;

    now.start();
    next_interval_time.start();

    createSettings(settingsLayout);
    setFilePath("/tmp");

    for (int i=0; i<8; ++i) {
        addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,tr("in_%1").arg(i)));
    }

    std::cout << "Instantiated RawWriteSis3302v1410Plugin" << std::endl;
}

void RawWriteSis3302v1410Plugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;


        QLabel* fileoutputLabel = new QLabel(tr("Output to:"));
        QLabel* intervalLabel = new QLabel(tr("minutes"));
        QLabel* nofEventsLabel = new QLabel(tr("Number of events"));
        writingDataLabel = new QLabel("Status: idle");
        filePathLineEdit = new QLineEdit();
        filePathButton = new QPushButton(tr("..."));
        intervalModeCheckBox = new QCheckBox("Interval");
        intervalSpinBox = new QSpinBox();
        intervalSpinBox->setMaximum(3600);
        nofEventsSpinBox = new QSpinBox();
        nofEventsSpinBox->setMaximum(1000000);

        cl->addWidget(fileoutputLabel,0,0,1,1);
        cl->addWidget(filePathLineEdit,0,1,1,1);
        cl->addWidget(filePathButton,0,2,1,1);

        cl->addWidget(intervalModeCheckBox,1,0,1,1);
        cl->addWidget(intervalSpinBox,1,1,1,1);
        cl->addWidget(intervalLabel,1,2,1,1);

        cl->addWidget(nofEventsLabel,2,0,1,2);
        cl->addWidget(nofEventsSpinBox,2,2,1,1);

        cl->addWidget(writingDataLabel,3,0,1,3);

        connect(filePathButton,SIGNAL(clicked()),this,SLOT(filePathButtonClicked()));
        connect(intervalSpinBox,SIGNAL(valueChanged(int)),this,SLOT(settingsChanged()));
        connect(nofEventsSpinBox,SIGNAL(valueChanged(int)),this,SLOT(settingsChanged()));
        connect(intervalModeCheckBox,SIGNAL(toggled(bool)),this,SLOT(settingsChanged()));

        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void RawWriteSis3302v1410Plugin::settingsChanged() {
    settings_changed = true;
}

void RawWriteSis3302v1410Plugin::setFilePath(QString _filePath)
{
    filePath = _filePath;
    fileName = filePath + tr("/%1%2.dat").arg(prefix).arg(QDateTime::currentDateTime().toString("_yyMMdd_hhmmss"));
    filePathLineEdit->setText(fileName);
}

void RawWriteSis3302v1410Plugin::filePathButtonClicked()
{
    setFilePath(QFileDialog::getExistingDirectory(this,tr("Choose filename"),
                                                  RunManager::ref().getRunName(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
    settings_changed = true;
}

void RawWriteSis3302v1410Plugin::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
        set = "filePath";   if(settings->contains(set)) filePath = settings->value(set).toString();
        set = "fileName";   if(settings->contains(set)) fileName = settings->value(set).toString();
        set = "intervalMode";   if(settings->contains(set)) intervalMode = settings->value(set).toBool();
        set = "interval_minutes";   if(settings->contains(set)) interval_minutes = settings->value(set).toInt();
        set = "nof_events";   if(settings->contains(set)) nof_events = settings->value(set).toInt();
    settings->endGroup();

    // UI update
    filePathLineEdit->setText(fileName);
    intervalModeCheckBox->setChecked(intervalMode);
    intervalSpinBox->setValue(interval_minutes);
    nofEventsSpinBox->setValue(nof_events);

    settings_changed = true;
}

void RawWriteSis3302v1410Plugin::saveSettings(QSettings* settings)
{
    if(settings == NULL)
    {
        std::cout << getName().toStdString() << ": no settings file" << std::endl;
        return;
    }
    else
    {
        std::cout << getName().toStdString() << " saving settings...";
        settings->beginGroup(getName());
            settings->setValue("filePath",filePath);
            settings->setValue("fileName",fileName);
            settings->setValue("intervalMode",intervalModeCheckBox->isChecked());
            settings->setValue("interval_minutes",intervalSpinBox->value());
            settings->setValue("nof_events",nofEventsSpinBox->value());
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void RawWriteSis3302v1410Plugin::userProcess()
{
    //std::cout << "RawWriteSis3302v1410Plugin Processing" << std::endl;

    QVector<uint32_t> data[8];
    uint32_t ch_mask = 0;
    uint32_t nof_enabled_channels = 0;
    uint32_t total_data_length = 0;
    uint32_t data_length[8];

    for(int i = 0; i < 8; ++i) {
        data[i] = inputs->at(i)->getData().value< QVector<uint32_t> > ();
        if(!data[i].empty()) {
            ch_mask |= (1 << i);
            nof_enabled_channels++;
            data_length[i] = data[i].size();
            total_data_length += data_length[i];
        }
        data_length[i] = 0;
    }

    if(settings_changed) {
        intervalMode = intervalModeCheckBox->isChecked();
        interval_minutes = intervalSpinBox->value();
        nof_events = nofEventsSpinBox->value();
        settings_changed = false;
        next_interval_time.start();
        nof_events_written = 0;
        if(file && file->isOpen()) {
            file->close();
            delete file;
        }
        writingDataLabel->setText(tr("Status: Idle"));
    }

    // Get current time
    now.start();

    // Check how many events have to be written
    int events_to_go = nof_events - nof_events_written;

    // Close the file, if we are done
    if(events_to_go == 0) {
        if(file && file->isOpen()) {
            file->close();
            delete file;
            file = 0;
            writingDataLabel->setText(tr("Status: Idle, next start: %1").arg(next_interval_time.toString()));
        }
    }

    // Check how many seconds until next start
    int seconds_until_next_start = now.secsTo(next_interval_time);

    //printf("sec_to %d, ev_to_go %d, \n",seconds_until_next_start,events_to_go);

    // Check interval mode
    if(intervalMode == true
            && seconds_until_next_start > 0
            && events_to_go == 0) return;

    // Only if the next deadline has passed
    // or we still have to write events
    if(nof_events_written == 0) {
        interval_number += 1;
        if(file && file->isOpen()) {
            file->close();
            delete file;
            file = 0;
        }
        writingDataLabel->setText(tr("Status: Idle, next start: %1").arg(next_interval_time.toString()));
    }
    if(!file){
        setFilePath(filePath);
        file = new QFile(fileName);
        file->open(QIODevice::WriteOnly|QIODevice::Append);
        last_interval_time = now;
        next_interval_time = now.addSecs(60*interval_minutes);
        nof_events_written = 0;
        writingDataLabel->setText(tr("Status: Writing data since ").arg(now.toString()));
    }
    if(!file->isOpen()) {
        std::cout << "File could not be opened." << std::endl;
    }

    // Write one event
    if(file->isWritable())
    {

        QDataStream out(file);
        out.setByteOrder(QDataStream::LittleEndian); //!

        uint32_t header = 0x33021410;
        uint32_t header_length = 4 + nof_enabled_channels;
        uint32_t total_length = header_length + total_data_length;

        // Write header data
        out << header;
        out << header_length;
        out << ch_mask;
        out << total_length;
        for(int ch = 0; ch < 8; ++ch) {
            if(data_length[ch] > 0) out << data_length[ch];
        }

        // Write channel contents
        for(int ch = 0; ch < 8; ++ch) {
            if(data[ch].empty ()) {
                //std::cout << "No data." << std::endl;
                continue;
            }
            for(int i = 0; i < data[ch].size(); i++) {
                out << (uint16_t)(data[ch].at(i) & 0xFFFF);
            }
        }

        ++nof_events_written;
    } else {
        std::cout << "File not writeable." << std::endl;
        file->close();
    }
}
