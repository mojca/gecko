#include "fileoutputplugin.h"
#include "pluginmanager.h"
#include "pluginconnectorqueued.h"

static PluginRegistrar registrar ("fileoutput", FileOutputPlugin::create, AbstractPlugin::GroupOutput);

FileOutputPlugin::FileOutputPlugin(int _id, QString _name)
            : BasePlugin(_id, _name)
{
    createSettings(settingsLayout);
    setFilePath("/tmp");

    prefix = tr("raw");

    addConnector(new PluginConnectorQVUint (this,ScopeCommon::in, "in"));

    std::cout << "Instantiated FileOutputPlugin" << std::endl;
}

void FileOutputPlugin::createSettings(QGridLayout * l)
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

void FileOutputPlugin::setFilePath(QString _filePath)
{
    filePath = _filePath;
    fileName = filePath + tr("/%1%2.dat").arg(prefix).arg(QDateTime::currentDateTime().toString("_yyMMdd_hhmm"));
    filePathLineEdit->setText(fileName);
}

void FileOutputPlugin::filePathButtonClicked()
{
    setFilePath(QFileDialog::getExistingDirectory(this,tr("Choose filename"),
                                                  "/tmp",QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
}

void FileOutputPlugin::userProcess()
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly|QIODevice::Append);
    if(!file.isOpen())
    {
        std::cout << "File could not be opened." << std::endl;
    }
    if(file.isWritable())
    {
        QDataStream out(&file);

        QVector<uint32_t> d = inputs->first()->getData().value< QVector<uint32_t> > ();
        if(d.size() == 0) std::cout << "No data." << std::endl;
        for(int i=0; i<d.size(); i++)
        {
            out << (uint32_t) (d.at(i));
            //std::cout << std::hex << d->at(i) << std::endl;
        }
    }
    else
    {
        std::cout << "File not writeable." << std::endl;
    }
    file.close();
}

