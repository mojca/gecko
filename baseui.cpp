#include <iostream>

#include "baseui.h"

#include "basemodule.h"
#include "baseinterfacemodule.h"

BaseUI::BaseUI(BaseModule* _module)
    : module(_module)
{
    name = module->getName();
    //std::cout << "Instantiated Base UI" << std::endl;
}

BaseUI::BaseUI(BaseInterfaceModule* _module)
    : module(_module)
{
    name = module->getName();
    //std::cout << "Instantiated Base UI for interface" << std::endl;
}
