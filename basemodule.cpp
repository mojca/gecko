#include <iostream>

#include "basemodule.h"

BaseModule::BaseModule(int _id, QString _name)
{
    name = _name;
    id = _id;
    ui = NULL;

    //std::cout << "Instantiated Base Module" << std::endl;
}
