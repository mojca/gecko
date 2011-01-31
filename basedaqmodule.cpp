#include "basedaqmodule.h"
#include "pluginmanager.h"

BaseDAqModule::BaseDAqModule (int _id, QString _name)
    : BaseModule (_id, _name)
    , iface (NULL)
    , output (NULL)
{
}

void BaseDAqModule::emitTrigger()
{
    //std::cout << thread()->currentThreadId() << ": emit triggered" << std::endl;
    emit triggered(channels.back());
}
