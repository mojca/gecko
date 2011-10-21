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

#include "eventbuilderplugin.h"
#include "pluginmanager.h"
#include "runmanager.h"
#include "pluginconnectorqueued.h"


static PluginRegistrar registrar ("eventbuilder", EventBuilderPlugin::create, AbstractPlugin::GroupPack, EventBuilderPlugin::getEventBuilderAttributeMap());

EventBuilderPlugin::EventBuilderPlugin(int _id, QString _name, const Attributes &_attrs)
            : BasePlugin(_id, _name)
            , attribs_ (_attrs)
            , filePrefix("run")
            , port(40000)
            , addr(QHostAddress::LocalHost)
            , net(0)
            , total_bytes_written(0)
            , current_bytes_written(0)
            , current_file_number(0)
            , open_new_file(true)
            , runPath("/tmp")
            , total_data_length(0)
            , nofEnabledInputs(0)
{
    createSettings(settingsLayout);

    bool ok;
    int _nofInputs = _attrs.value ("nofInputs", QVariant (4)).toInt (&ok);
    if (!ok || _nofInputs <= 0 || _nofInputs > 32) {
        _nofInputs = 32;
        std::cout << _name.toStdString () << ": nofInputs invalid. Setting to 32." << std::endl;
    }

    addConnector(new PluginConnectorQVUint (this,ScopeCommon::out,"out"));
    //outData.push_back(NULL);

    for(int n = 0; n < _nofInputs; n++)
    {
        addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,QString("in %1").arg(n)));
    }

    setNumberOfMandatoryInputs(1); // Only needs one input to have data for writing the event

    connect(RunManager::ptr(),SIGNAL(runNameChanged()),this,SLOT(updateRunName()));
    connect(RunManager::ptr(),SIGNAL(runStopped()),this,SLOT(updateRunName()));

    std::cout << "Instantiated EventBuilderPlugin" << std::endl;
}

AbstractPlugin::AttributeMap EventBuilderPlugin::getEventBuilderAttributeMap() {
    AbstractPlugin::AttributeMap attrs;
    attrs.insert ("nofInputs", QVariant::Int);
    return attrs;
}
AbstractPlugin::AttributeMap EventBuilderPlugin::getAttributeMap () const { return getEventBuilderAttributeMap();}
AbstractPlugin::Attributes EventBuilderPlugin::getAttributes () const { return attribs_;}

void EventBuilderPlugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        nofInputsLabel = new QLabel(tr("%1").arg(inputs->size(),1,10));

        totalBytesWrittenLabel = new QLabel(tr("%1 MBytes").arg(total_bytes_written/1024./1024.));
        currentBytesWrittenLabel = new QLabel(tr("%1 MBytes").arg(current_bytes_written/1024./1024.));
        currentFileNameLabel = new QLabel(makeFileName());
        runPath = RunManager::ptr()->getRunName().toStdString().c_str();
        boost::uintmax_t freeBytes = boost::filesystem::space(runPath).available;
        bytesFreeOnDiskLabel = new QLabel(tr("%1 GBytes").arg((double)(freeBytes/1024./1024./1024.)));

        portSpinner = new QSpinBox();
        portSpinner->setMinimum(1024);
        portSpinner->setMaximum(65535);

        QString Octet = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
        QRegExpValidator* v = new QRegExpValidator(QRegExp("^" + Octet + "\\."
                                                          + Octet + "\\."
                                                          + Octet + "\\."
                                                          + Octet + "$"), this);

        addrEdit = new QLineEdit();
        //addrEdit->setInputMask("000.000.000.000;_");
        addrEdit->setText(addr.toString());
        addrEdit->setValidator(v);

        //cl->addWidget(new QLabel("Number of inputs:"),      0,0,1,1);
        //cl->addWidget(nofInputsLabel,                       0,1,1,1);
        QGroupBox* gf = new QGroupBox("Disk stats");
        {
            QGridLayout* cl = new QGridLayout();
            cl->addWidget(new QLabel("File:"),                  1,0,1,1);
            cl->addWidget(currentFileNameLabel,                 1,1,1,1);
            cl->addWidget(new QLabel("Data written:"),          2,0,1,1);
            cl->addWidget(currentBytesWrittenLabel,             2,1,1,1);
            cl->addWidget(new QLabel("Total Data Written:"),    3,0,1,1);
            cl->addWidget(totalBytesWrittenLabel,               3,1,1,1);
            cl->addWidget(new QLabel("Disk free:"),             4,0,1,1);
            cl->addWidget(bytesFreeOnDiskLabel,                 4,1,1,1);
            gf->setLayout(cl);
        }
        cl->addWidget(gf,0,0,1,2);

        QGroupBox* gn = new QGroupBox("Network Setup");
        {
            QGridLayout* cl = new QGridLayout();
            cl->addWidget(new QLabel("Address:"),       5,0,1,1);
            cl->addWidget(addrEdit,                     5,1,1,1);
            cl->addWidget(new QLabel("Port:"),          6,0,1,1);
            cl->addWidget(portSpinner,                  6,1,1,1);
            gn->setLayout(cl);
        }
        cl->addWidget(gn,1,0,1,2);

        container->setLayout(cl);

        connect(RunManager::ptr(),SIGNAL(runNameChanged()),this,SLOT(updateRunName()));
        connect(portSpinner,SIGNAL(valueChanged(int)),this,SLOT(uiInput()));
        connect(addrEdit,SIGNAL(editingFinished()),this,SLOT(uiInput()));
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void EventBuilderPlugin::uiInput() {
    port = portSpinner->value();
    addr = QHostAddress(addrEdit->text());
}

void EventBuilderPlugin::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
        set = "port";   if(settings->contains(set)) port = settings->value(set).toUInt();
        set = "addr";   if(settings->contains(set)) addr = settings->value(set).toString();
    settings->endGroup();

    portSpinner->setValue(port);
    addrEdit->setText(addr.toString());
}

void EventBuilderPlugin::saveSettings(QSettings* settings)
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
            settings->setValue("port",port);
            settings->setValue("addr",addr.toString());
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void EventBuilderPlugin::updateByteCounters() {
    boost::uintmax_t freeBytes = boost::filesystem::space(runPath).available;
    currentBytesWrittenLabel->setText(tr("%1 MBytes").arg(current_bytes_written/1024./1024.,2,'f',3));
    bytesFreeOnDiskLabel->setText(tr("%1 GBytes").arg((double)(freeBytes/1024./1024./1024.),2,'f',3));
    totalBytesWrittenLabel->setText(tr("%1 MBytes").arg(total_bytes_written/1024./1024.,2,'f',3));
}

void EventBuilderPlugin::updateRunName() {
    runPath = RunManager::ptr()->getRunName().toStdString().c_str();
    currentFileNameLabel->setText(makeFileName());
    open_new_file = true;
}

QString EventBuilderPlugin::makeFileName() {
    return RunManager::ptr()->getRunName() +
            tr("/%1%2%3.dat")
            .arg(filePrefix)
            .arg(QDateTime::currentDateTime().toString("_yyMMdd_hhmmss_"))
            .arg(current_file_number,4,10,QChar('0'));
}

void EventBuilderPlugin::runStartingEvent() {
    // Reset timer
    lastUpdateTime.start();

    // Get number of inputs
    nofInputs = inputs->size();

    // Resize vectors
    data.resize(nofInputs);
    data_length.resize(nofInputs);
    input_has_data.resize(nofInputs);

    // Reset counters
    current_bytes_written = 0;
    current_file_number = 0;
    total_bytes_written = 0;
    ch_mask = 0;

    // Clear vectors
    data_length.fill(0);
    input_has_data.fill(false);

    // Update UI
    updateRunName();
    updateByteCounters();
    nofInputsLabel->setText(tr("%1").arg(nofInputs));

}

void EventBuilderPlugin::userProcess()
{
    if(!net) net = new QUdpSocket ();

    //std::cout << "EventBuilderPlugin Processing" << std::endl;

    total_data_length = 0;
    nofEnabledInputs = 0;
    ch_mask = 0;

    // Get extends of each data input
    for(uint32_t i=0; i<nofInputs; ++i) {
        data[i] = inputs->at(i)->getData().value< QVector<uint32_t> >();
        if(data[i].empty()) {
            data_length[i] = 0;
            input_has_data[i] = false;
            //printf("EventBuilder: <ch%d> No data, ch_mask: 0x%02x\n",i,ch_mask[i/8]);
        } else {
            data_length[i] = data[i].size();
            input_has_data[i] = true;
            ch_mask |= (1 << i);
            total_data_length += data_length[i];
            ++nofEnabledInputs;
            //printf("EventBuilder: <ch%d> Data with length: %d, ch_mask: 0x%02x\n",i,data_length[i],ch_mask[i/8]);
        }
    }

    // File switch at 1 Gigabyte
    if(current_bytes_written >= 1024*1024*1024) {
        open_new_file = true;
    }

    if(open_new_file == true) {
        // If necessary, close old file
        if(outFile.isOpen()) {
            outFile.close();
        }

        // Open new file
        outDir = QDir(RunManager::ptr()->getRunName());
        if (outDir.exists()) {
            outFile.setFileName(makeFileName());
            outFile.open(QIODevice::WriteOnly);
            updateRunName();
            current_bytes_written = 0;
            ++current_file_number;
            open_new_file = false;
        } else {
            printf("EventBuilder: The output directory does not exist! (%s)\n",outDir.absolutePath().toStdString().c_str());
        }
    }

    QByteArray netData;
    QDataStream netOut(&netData,QIODevice::WriteOnly);

    // Write to the file
    if(outFile.isOpen()) {
        QDataStream out(&outFile);

        out.setByteOrder(QDataStream::LittleEndian);

        // Event header
        uint16_t header = 0xFEED;
        uint16_t header_length = 2 + (nofEnabledInputs/2)+1; // in words (INCORRECT)
        //uint16_t header_length = 2 + nofEnabledInputs; // in words (CORRECT)
        total_data_length += header_length
                          + (nofEnabledInputs); // To account for separators

        out << header;
        out << header_length;
        out << ch_mask;

        netOut << header;
        netOut << header_length;
        netOut << ch_mask;

        // Write channel lengths
        for(uint32_t i = 0; i < nofInputs; ++i) {
            if(data_length[i] > 0) {
                out << data_length[i];
                netOut << data_length[i];
            }
        }

        // Write channel data with separators
        for(uint32_t ch = 0; ch < nofInputs; ++ch) {
            if(data_length[ch] > 0) {
                for(uint32_t i = 0; i < data_length[ch]; ++i) {
                    out << data[ch][i];
                    netOut << data[ch][i];
                }
                uint32_t separator = 0xFFFFFFFF;
                out << separator;
                netOut << separator;
            }
        }

        current_bytes_written += total_data_length * 4;
        total_bytes_written += total_data_length * 4;
    } else {
        printf("EventBuilder: File is not open for writing.\n");
    }

    // Write to network
    net->writeDatagram(netData,addr,port);

    if(lastUpdateTime.msecsTo(QTime::currentTime()) > 500) {
        updateByteCounters();
        lastUpdateTime.start();
    }
}
