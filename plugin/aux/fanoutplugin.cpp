#include "fanoutplugin.h"

static PluginRegistrar registrarInt ("fanoutInt", FanOutPlugin<uint32_t>::create, AbstractPlugin::GroupAux, FanOutPlugin<uint32_t>::getFanoutAttributeMap ());
static PluginRegistrar registrarDouble ("fanoutDouble", FanOutPlugin<double>::create, AbstractPlugin::GroupAux, FanOutPlugin<double>::getFanoutAttributeMap ());
