#include "sis3302module.h"
#include "modulemanager.h"

static ModuleRegistrar registrar ("sis3302", Sis3302Module::create, AbstractModule::TypeDAq);

Sis3302Module::Sis3302Module(int _id, QString _name)
    : BaseDAqModule(_id, _name)
{
    //getConfigFromCode();

    //setDefaultConfig();
    setUI (new Sis3302UI(this));

    setChannels();
    createOutputPlugin();

    std::cout << "Instantiated Sis3302 Module" << std::endl;
}

Sis3302Module::~Sis3302Module()
{
    delete buffer;
}
