
#include <QSignalMapper>
#include "rawwritesis3350pluginV2.h"
#include "pluginmanager.h"
#include "runmanager.h"
#include "pluginconnectorqueued.h"

static PluginRegistrar registrar ("rawwritesis3350V2", RawWriteSis3350PluginV2::create, AbstractPlugin::GroupOutput);

RawWriteSis3350PluginV2::RawWriteSis3350PluginV2(int _id, QString _name)
    : BasePlugin(_id, _name)
{
    prefix = tr("vector");
    chMask = 0x0;
    nofEnabledChannels = 0;
    fileNo = 0;
    reset();

    createSettings(settingsLayout);
    setFilePath("/tmp");

    addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,"in0"));
    addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,"in1"));
    addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,"in2"));
    addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,"in3"));

    addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,"meta in"));

    std::cout << "Instantiated RawWriteSis3350PluginV2" << std::endl;
}

void RawWriteSis3350PluginV2::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;
        QSignalMapper *mapper = new QSignalMapper();

        QLabel* fileoutputLabel = new QLabel(tr("Output to:"));
        filePathLineEdit = new QLineEdit();
        filePathButton = new QPushButton(tr("..."));
        connect(filePathButton,SIGNAL(clicked()),this,SLOT(filePathButtonClicked()));
        connect(filePathLineEdit,SIGNAL(editingFinished()),this,SLOT(filePathEditChanged()));

        cl->addWidget(fileoutputLabel,0,0,1,1);
        cl->addWidget(filePathLineEdit,1,1,1,1);
        cl->addWidget(filePathButton,1,2,1,1);

	for(int i=0; i<4; i++)
        {
            saveEnabled[i] = false;
            fileSaveCheck[i] = new QCheckBox(tr("Write Channel %1").arg(i));
            fileSaveCheck[i]->setCheckState(Qt::Unchecked);
            cl->addWidget(fileSaveCheck[i],2+i,0,1,2);
            connect(fileSaveCheck[i],SIGNAL(stateChanged(int)),mapper,SLOT(map()));
            mapper->setMapping(fileSaveCheck[i],i);
        }
        connect(mapper,SIGNAL(mapped(int)),this,SLOT(fileSaveCheckToggled(int)));

        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void RawWriteSis3350PluginV2::fileSaveCheckToggled(int ch)
{
    bool newValue = fileSaveCheck[ch]->isChecked();
    saveEnabled[ch] = newValue;
    updateChMask();
}

void RawWriteSis3350PluginV2::updateChMask()
{
    chMask = 0x0;
    nofEnabledChannels = 0;
    for(int i=0; i < 4; i++)
    {
        if(saveEnabled[i] == true)
        {
            chMask |= (0x1 << i);
            nofEnabledChannels++;
        }
    }
}

void RawWriteSis3350PluginV2::fileNameEditChanged()
{
    fileName = filePathLineEdit->text();
    reset();
}

void RawWriteSis3350PluginV2::reset()
{
    bytesWritten = 0;
    cycles = 0;
    cycleSkip = 0;
    fileNo = 0;
}

void RawWriteSis3350PluginV2::setFilePath(QString _filePath)
{
    filePath = _filePath;
    fileName = filePath + tr("/%1%2.dat").arg(prefix).arg(QDateTime::currentDateTime().toString("_yyMMdd_hhmm"));
    filePathLineEdit->setText(fileName);
    reset();
}

void RawWriteSis3350PluginV2::filePathButtonClicked()
{
    setFilePath(QFileDialog::getExistingDirectory(this,tr("Choose filename"),
                                                  RunManager::ref().getRunName(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
}

void RawWriteSis3350PluginV2::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
        set = "filePath";   if(settings->contains(set)) filePath = settings->value(set).toString();
        set = "fileName";   if(settings->contains(set)) fileName = settings->value(set).toString();
        set = "saveEnabled_0";   if(settings->contains(set)) saveEnabled[0] = settings->value(set).toBool();
        set = "saveEnabled_1";   if(settings->contains(set)) saveEnabled[1] = settings->value(set).toBool();
        set = "saveEnabled_2";   if(settings->contains(set)) saveEnabled[2] = settings->value(set).toBool();
        set = "saveEnabled_3";   if(settings->contains(set)) saveEnabled[3] = settings->value(set).toBool();
   settings->endGroup();

    // UI update
    filePathLineEdit->setText(fileName);
    if(saveEnabled[0] == true) fileSaveCheck[0]->setCheckState(Qt::Checked);
    if(saveEnabled[1] == true) fileSaveCheck[1]->setCheckState(Qt::Checked);
    if(saveEnabled[2] == true) fileSaveCheck[2]->setCheckState(Qt::Checked);
    if(saveEnabled[3] == true) fileSaveCheck[3]->setCheckState(Qt::Checked);
    updateChMask();
}

void RawWriteSis3350PluginV2::saveSettings(QSettings* settings)
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
            settings->setValue("saveEnabled_0",saveEnabled[0]);
            settings->setValue("saveEnabled_1",saveEnabled[1]);
            settings->setValue("saveEnabled_2",saveEnabled[2]);
            settings->setValue("saveEnabled_3",saveEnabled[3]);
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void RawWriteSis3350PluginV2::userProcess()
{
    //std::cout << "RawWriteSis3350PluginV2 Processing" << std::endl;

    const std::vector<uint32_t>* data[4];
    for(int i = 0; i < 4; i++)
    {
        data[i] = reinterpret_cast<const std::vector<uint32_t>*>(inputs->at(i)->getData());
    }

    const std::vector<uint32_t>* meta = reinterpret_cast<const std::vector<uint32_t>*>(inputs->at(4)->getData());

    QString fileNameWithNumber(tr("%1%2").arg(fileName).arg(fileNo,6,10,QChar('0')));

    QFile file(fileNameWithNumber);
    file.open(QIODevice::WriteOnly|QIODevice::Append);
    if(!file.isOpen())
    {
        std::cout << "File could not be opened." << std::endl;
    }
    if(file.isWritable())
    {
        QDataStream out(&file);
        out.setByteOrder(QDataStream::LittleEndian); //!

        for(unsigned int i = 0; i < meta->size(); i++)
        {
            // Update total length information
            if(i==1) out << (uint32_t) (8 + (meta->at(4) * nofEnabledChannels));
            // Update channel mask information
            else if(i==5) out << (uint32_t)(chMask);
            else out << meta->at(i);
        }

        for(unsigned int ch = 0; ch < 4; ch++)
        {
            if(data[ch]->size() == 0)
            {
                std::cout << "No data in ch " << std::dec << ch << std::endl;
                continue;
            }

            if(saveEnabled[ch] == true)
            {
                for(unsigned int i = 0; i < data[ch]->size(); i++)
                {
                    out << (uint16_t)(data[ch]->at(i) & 0xFFFF);               
                }
                bytesWritten += data[ch]->size()*2;
            }
        }
    }
    else
    {
        std::cout << "File not writeable." << std::endl;
    }
    file.close();

    cycles++;
    cycleSkip++;

    // Reset cycle skip and start new file
    if(cycleSkip == 1000)
    {
        cycleSkip = 0;
        fileNo++;
        std::cout << std::dec << (quint64)bytesWritten/1024/1024 << " Mbytes written in " 
              << std::dec << cycles << " cycles." << std::endl;
    }

}
