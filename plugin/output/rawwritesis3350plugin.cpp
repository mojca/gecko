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

#include "rawwritesis3350plugin.h"
#include "pluginmanager.h"
#include "runmanager.h"
#include "pluginconnectorqueued.h"

static PluginRegistrar registrar ("rawwritesis3350", RawWriteSis3350Plugin::create, AbstractPlugin::GroupOutput);

RawWriteSis3350Plugin::RawWriteSis3350Plugin(int _id, QString _name)
    : BasePlugin(_id, _name)
{
    createSettings(settingsLayout);
    setFilePath("/tmp");

    prefix = tr("vector");

    addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,"in"));
    addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,"meta in"));

    std::cout << "Instantiated RawWriteSis3350Plugin" << std::endl;
}

void RawWriteSis3350Plugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        QLabel* fileoutputLabel = new QLabel(tr("Output to:"));
        filePathLineEdit = new QLineEdit();
        filePathButton = new QPushButton(tr("..."));
        connect(filePathButton,SIGNAL(clicked()),this,SLOT(filePathButtonClicked()));

        cl->addWidget(fileoutputLabel,0,0,1,1);
        cl->addWidget(filePathLineEdit,0,1,1,1);
        cl->addWidget(filePathButton,0,2,1,1);

        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void RawWriteSis3350Plugin::setFilePath(QString _filePath)
{
    filePath = _filePath;
    fileName = filePath + tr("/%1%2.dat").arg(prefix).arg(QDateTime::currentDateTime().toString("_yyMMdd_hhmm"));
    filePathLineEdit->setText(fileName);
}

void RawWriteSis3350Plugin::filePathButtonClicked()
{
    setFilePath(QFileDialog::getExistingDirectory(this,tr("Choose filename"),
                                                  RunManager::ref().getRunName(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
}

void RawWriteSis3350Plugin::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
        set = "filePath";   if(settings->contains(set)) filePath = settings->value(set).toString();
        set = "fileName";   if(settings->contains(set)) fileName = settings->value(set).toString();
    settings->endGroup();

    // UI update
    filePathLineEdit->setText(fileName);
}

void RawWriteSis3350Plugin::saveSettings(QSettings* settings)
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
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void RawWriteSis3350Plugin::userProcess()
{
    //std::cout << "RawWriteSis3350Plugin Processing" << std::endl;

    QVector<uint32_t> data = inputs->at(0)->getData().value< QVector<uint32_t> > ();
    QVector<uint32_t> meta = inputs->at(1)->getData().value< QVector<uint32_t> > ();

    QFile file(fileName);
    file.open(QIODevice::WriteOnly|QIODevice::Append);
    if(!file.isOpen())
    {
        std::cout << "File could not be opened." << std::endl;
    }
    if(file.isWritable())
    {
        QDataStream out(&file);
        out.setByteOrder(QDataStream::LittleEndian); //!

        if(data.empty ())
        {
            std::cout << "No data." << std::endl;
            return;
        }

        for(int i = 0; i < meta.size(); i++)
        {
            out << meta.at(i);
        }
        for(int i = 0; i < data.size(); i++)
        {
            out << (uint16_t)(data.at(i) & 0xFFFF);
        }
    }
    else
    {
        std::cout << "File not writeable." << std::endl;
    }
    file.close();
}
