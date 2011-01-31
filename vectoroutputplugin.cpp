#include "vectoroutputplugin.h"
#include "pluginmanager.h"
#include "runmanager.h"
#include "pluginconnectorqueued.h"

static PluginRegistrar registrar ("vectoroutput", VectorOutputPlugin::create, AbstractPlugin::GroupOutput);

VectorOutputPlugin::VectorOutputPlugin(int _id, QString _name)
    : BasePlugin(_id, _name)
{
    createSettings(settingsLayout);
    setFilePath("/tmp");

    prefix = tr("vector");

    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"in"));

    std::cout << "Instantiated VectorOutputPlugin" << std::endl;
}

void VectorOutputPlugin::createSettings(QGridLayout * l)
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

void VectorOutputPlugin::setFilePath(QString _filePath)
{
    filePath = _filePath;
    fileName = filePath + tr("/%1%2.dat").arg(prefix).arg(QDateTime::currentDateTime().toString("_yyMMdd_hhmm"));
    filePathLineEdit->setText(fileName);
}

void VectorOutputPlugin::filePathButtonClicked()
{
    setFilePath(QFileDialog::getExistingDirectory(this,tr("Choose filename"),
                                                  "/tmp",QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
}

void VectorOutputPlugin::userProcess()
{
    std::cout << "VectorOutputPlugin Processing" << std::endl;

    QFile file(fileName);
    file.open(QIODevice::WriteOnly|QIODevice::Append);
    if(!file.isOpen())
    {
        std::cout << "File could not be opened." << std::endl;
    }
    if(file.isWritable())
    {
        QTextStream out(&file);

        const std::vector<double>* d = reinterpret_cast<const std::vector<double>*>(inputs->first()->getData());
        if(d->size() == 0) std::cout << "No data." << std::endl;

        for(unsigned int i = 0; i < d->size(); i++)
        {
            out << d->at(i) << "\n";
        }
    }
    else
    {
        std::cout << "File not writeable." << std::endl;
    }
    file.close();
}
