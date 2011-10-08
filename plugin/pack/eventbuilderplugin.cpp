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
#include <boost/filesystem/convenience.hpp>

static PluginRegistrar registrar ("eventbuilder", EventBuilderPlugin::create, AbstractPlugin::GroupPack, EventBuilderPlugin::getEventBuilderAttributeMap());

EventBuilderPlugin::EventBuilderPlugin(int _id, QString _name, const Attributes &_attrs)
            : BasePlugin(_id, _name)
            , attribs_ (_attrs)
            , filePrefix("run")
            , total_bytes_written(0)
            , current_bytes_written(0)
            , current_file_number(0)
{
    createSettings(settingsLayout);

    bool ok;
    int _nofInputs = _attrs.value ("nofInputs", QVariant (4)).toInt (&ok);
    if (!ok || _nofInputs <= 0) {
        _nofInputs = 1;
        std::cout << _name.toStdString () << ": nofInputs invalid. Setting to 1" << std::endl;
    }

    addConnector(new PluginConnectorQVUint (this,ScopeCommon::out,"out"));
    //outData.push_back(NULL);

    for(int n = 0; n < _nofInputs; n++)
    {
        addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,QString("in %1").arg(n)));
    }

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

        QLabel* lineEdit = new QLabel(tr("%1").arg(inputs->size(),1,10));

        totalBytesWrittenLabel = new QLabel(tr("%1 MBytes").arg(total_bytes_written/1024./1024.));
        currentBytesWrittenLabel = new QLabel(tr("%1 MBytes").arg(current_bytes_written/1024./1024.));
        currentFileNameLabel = new QLabel(makeFileName());
        boost::filesystem::path runPath(RunManager::ptr()->getRunName().toStdString().c_str());
        boost::uintmax_t freeBytes = boost::filesystem::space(runPath).available;
        bytesFreeOnDiskLabel = new QLabel(tr("%1 GBytes").arg((double)(freeBytes/1024./1024./1024.)));

        cl->addWidget(new QLabel("Number of inputs:"),      0,0,1,1);
        cl->addWidget(lineEdit,                             0,1,1,1);
        cl->addWidget(new QLabel("File:"),                  1,0,1,1);
        cl->addWidget(currentFileNameLabel,                 1,1,1,1);
        cl->addWidget(new QLabel("Data written:"),          2,0,1,1);
        cl->addWidget(currentBytesWrittenLabel,             2,1,1,1);
        cl->addWidget(new QLabel("Total Data Written:"),    3,0,1,1);
        cl->addWidget(totalBytesWrittenLabel,               3,1,1,1);
        cl->addWidget(new QLabel("Disk free:"),             4,0,1,1);
        cl->addWidget(bytesFreeOnDiskLabel,                 4,1,1,1);

        container->setLayout(cl);

        connect(RunManager::ptr(),SIGNAL(runNameChanged()),this,SLOT(updateRunName()));
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void EventBuilderPlugin::applySettings(QSettings* settings)
{
    settings->beginGroup(getName());
    settings->endGroup();
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
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void EventBuilderPlugin::updateRunName() {
    currentFileNameLabel->setText(makeFileName());
}

QString EventBuilderPlugin::makeFileName() {
    return RunManager::ptr()->getRunName() +
            tr("/%1%2%3.dat")
            .arg(filePrefix)
            .arg(QDateTime::currentDateTime().toString("_yyMMdd_hhmmss_"))
            .arg(current_file_number,4,10,QChar('0'));
}

void EventBuilderPlugin::userProcess()
{
    //std::cout << "EventBuilderPlugin Processing" << std::endl;

    uint32_t nofInputs = inputs->size();
    QVector<uint32_t> data[nofInputs];
    uint32_t data_length[nofInputs];
    uint32_t total_data_length;
    bool input_has_data[nofInputs];
    uint8_t ch_mask[nofInputs/8];

    // Clear channel mask
    for(uint32_t i=0; i<nofInputs/8; ++i) {
        ch_mask[i] = 0;
    }

    // Get extends of each data input
    for(uint32_t i=0; i<nofInputs; ++i) {
        data[i] = inputs->at(i)->getData().value< QVector<uint32_t> >();
        if(data[i].empty()) {
            data_length[i] = 0;
            input_has_data[i] = false;
            printf("EventBuilder: <%d> No data\n",i);
        } else {
            data_length[i] = data[i].size();
            input_has_data[i] = true;
            ch_mask[i/8] |= (1 << (i%8));
            total_data_length += data_length[i];
            printf("EventBuilder: <%d> Data with length: %d",i,data_length[i]);
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
        filePrefix = "run";
        outDir = QDir(RunManager::ptr()->getRunName());
        if (outDir.exists()) {
            outFile.setFileName(makeFileName());
            outFile.open(QIODevice::WriteOnly);
            currentFileNameLabel->setText(outFile.fileName());
            ++current_file_number;
        } else {
            printf("EventBuilder: The output directory does not exist! (%s)\n",outDir.absolutePath().toStdString().c_str());
        }
    }

    // Write to the file
    if(outFile.isOpen()) {
        for(uint32_t i=0; i<nofInputs; ++i) {

        }
    }

}
