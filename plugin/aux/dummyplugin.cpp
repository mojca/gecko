#include "dummyplugin.h"
#include "pluginmanager.h"
#include "pluginconnectorqueued.h"

static PluginRegistrar registrar ("dummy", DummyPlugin::create, AbstractPlugin::GroupAux);

DummyPlugin::DummyPlugin(int _id, QString _name)
        : BasePlugin(_id, _name)
{
    addConnector (new PluginConnectorQueued<uint32_t> (this, ScopeCommon::out, "out"));
}

void DummyPlugin::userProcess()
{
    // Here goes nothing!
}

void DummyPlugin::createSettings(QGridLayout *)
{
    // Here goes nothing!
}
