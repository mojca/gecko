#include "vectoroutputplugin.h"
#include "pluginmanager.h"
#include "runmanager.h"
#include "pluginconnectorqueued.h"
#include "confmap.h"

static PluginRegistrar registrar ("vectoroutput", VectorOutputPlugin::create, AbstractPlugin::GroupOutput);

struct VectorOutputConfig {
    QString prefix;
};

VectorOutputPlugin::VectorOutputPlugin(int _id, QString _name)
    : BasePlugin(_id, _name)
    , cfg (new VectorOutputConfig)
{
    createSettings(settingsLayout);
    cfg->prefix = tr("vector");

    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"in"));

    std::cout << "Instantiated VectorOutputPlugin" << std::endl;
}

void VectorOutputPlugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;


        prefixLineEdit = new QLineEdit ();
        cl->addWidget(new QLabel(tr("File Prefix:")), 0, 0, 1, 1);
        cl->addWidget(prefixLineEdit, 0, 1, 1, 1);

        pathLabel = new QLabel (tr ("<none>"));
        cl->addWidget (new QLabel (tr ("Last file:")), 1, 0, 1, 1);
        cl->addWidget (pathLabel, 1, 1, 1, 1);

        container->setLayout(cl);

        connect (prefixLineEdit, SIGNAL(textChanged(QString)), SLOT(prefixChanged()));
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void VectorOutputPlugin::prefixChanged ()
{
    cfg->prefix = prefixLineEdit->text ();
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

        QVector<double> d = inputs->first()->getData().value< QVector<double> > ();
        if(d.empty()) std::cout << "No data." << std::endl;

        for(int i = 0; i < d.size(); i++)
        {
            out << d.at(i) << "\n";
        }
    }
    else
    {
        std::cout << "File not writeable." << std::endl;
    }
    file.close();
}

void VectorOutputPlugin::runStartingEvent () {
    fileName = QDir::cleanPath (RunManager::ref().getRunName() + "/" +
            cfg->prefix +
            QDateTime::currentDateTime().toString("_yyMMdd_hhmm") + ".dat");

    pathLabel->setText (fileName);
}

typedef ConfMap::confmap_t<VectorOutputConfig> confmap_t;
static const confmap_t confmap [] = {
    confmap_t ("prefix", &VectorOutputConfig::prefix)
};

void VectorOutputPlugin::applySettings(QSettings *s) {
    s->beginGroup (getName());
    ConfMap::apply (s, cfg, confmap);
    s->endGroup ();

    prefixLineEdit->setText (cfg->prefix);
}

void VectorOutputPlugin::saveSettings(QSettings *s) {
    s->beginGroup (getName());
    ConfMap::save (s, cfg, confmap);
    s->endGroup ();
}
