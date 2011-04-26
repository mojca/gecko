#include "packsis3350plugin.h"
#include "pluginmanager.h"
#include "runmanager.h"
#include "pluginconnectorqueued.h"

static PluginRegistrar registrar ("packsis3350", PackSis3350Plugin::create, AbstractPlugin::GroupPack);

PackSis3350Plugin::PackSis3350Plugin(int _id, QString _name)
    : BasePlugin(_id, _name)
{
    createSettings(settingsLayout);

    addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,"in"));
    addConnector(new PluginConnectorQVUint(this,ScopeCommon::in,"meta in"));
    addConnector(new PluginConnectorQVUint(this,ScopeCommon::out,"packed out"));

    std::cout << "Instantiated PackSis3350Plugin" << std::endl;
}

void PackSis3350Plugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;
        QLabel* label = new QLabel(tr("Packing plugin for SIS3350 ADC data"));
        cl->addWidget(label,0,0,1,1);
        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void PackSis3350Plugin::applySettings(QSettings* settings)
{
    settings->beginGroup(getName());
    settings->endGroup();
}

void PackSis3350Plugin::saveSettings(QSettings* settings)
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

void PackSis3350Plugin::userProcess()
{
    //std::cout << "PackSis3350Plugin Processing" << std::endl;

    QVector<uint32_t> data = inputs->at(0)->getData().value< QVector<uint32_t> > ();
    QVector<uint32_t> meta = inputs->at(1)->getData().value< QVector<uint32_t> > ();

    if(data.empty())
    {
        std::cout << "No data." << std::endl;
        return;
    }

    QVector<uint32_t> out (meta);
    out << data;

    outputs->first()->setData(QVariant::fromValue (out));
}
