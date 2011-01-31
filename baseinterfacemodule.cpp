#include "baseinterfacemodule.h"

BaseInterfaceModule::BaseInterfaceModule(int _id, QString _name)
        : BaseModule(_id, _name)
{
    addrMode = "A32";
    dataMode = "D32";
}
