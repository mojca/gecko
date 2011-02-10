#include "modulemanager.h"
#include "basedaqmodule.h"

#include <stdexcept>

ModuleManager *ModuleManager::inst = NULL;

struct ModuleTypeDesc {
    ModuleManager::ModuleFactory fac;
    AbstractModule::Type type;
};

ModuleManager *ModuleManager::ptr () {
	if (inst == NULL)
		inst = new ModuleManager ();
	return inst;
}

ModuleManager &ModuleManager::ref () {
	return *ptr ();
}

ModuleManager::ModuleManager()
{
    items = new QList<BaseModule*>;
    daqItems = new QList<BaseDAqModule*>;
    interfaceItems = new QList<BaseInterface*>;

    std::cout << "Instantiated ModuleManager" << std::endl;
    mainInterface = NULL;
}

ModuleManager::~ModuleManager()
{
    clear();
    delete items;
    delete daqItems;
    delete interfaceItems;
}

void ModuleManager::clear()
{
    while (!daqItems->empty ())
        remove (daqItems->first ());
    while (!interfaceItems->empty ())
        remove (interfaceItems->first ());
}

void ModuleManager::identify()
{
    std::cout << "ModuleManager" << std::endl;
}

bool ModuleManager::remove(BaseModule* rmModule)
{
    QList<BaseModule*>::iterator it(items->begin());
    while(it != items->end())
    {
        if((*it) == rmModule)
        {
            if((*it)->getModuleType() == AbstractModule::TypeDAq)
            {
                daqItems->removeAll(dynamic_cast<BaseDAqModule*>(*it));
            }
            else
            {
                interfaceItems->removeAll(dynamic_cast<BaseInterface*>(*it));
            }
            items->erase(it);
            emit moduleRemoved (rmModule);
            rmModule->deleteLater ();
            return true;
        }
        it++;
    }
    return false;
}

bool ModuleManager::remove (int id)
{
    BaseModule* am;
    if((am = get (id)) != NULL)
    {
        remove(am);
        return true;
    }
    return false;
}

bool ModuleManager::remove (const QString &name)
{
    BaseModule* am;
    if((am = get (name)) != NULL)
    {
        remove(am);
        return true;
    }
    return false;
}


BaseModule* ModuleManager::get (int id)
{
    QList<BaseModule*>::iterator it(items->begin());
    while(it != items->end())
    {
        BaseModule* bm = (*it);
        if(bm->getId() == id)
        {
            return *it;
        }
        it++;
    }
    return NULL;
}

BaseModule* ModuleManager::get (const QString &name)
{
    QList<BaseModule*>::iterator it(items->begin());
    while(it != items->end())
    {
        if((*it)->getName() == name)
        {
            return *it;
        }
        it++;
    }
    return NULL;
}

BaseDAqModule *ModuleManager::getDAq (int id) {
    BaseModule *m = get (id);
    if (m->getModuleType () != AbstractModule::TypeDAq)
        return NULL;
    return static_cast<BaseDAqModule *> (m);
}

BaseDAqModule *ModuleManager::getDAq (const QString &name) {
    BaseModule *m = get (name);
    if (m->getModuleType () != AbstractModule::TypeDAq)
        return NULL;
    return static_cast<BaseDAqModule *> (m);
}

BaseInterface *ModuleManager::getIface (int id) {
    BaseModule *m = get (id);
    if (m->getModuleType () != AbstractModule::TypeInterface)
        return NULL;
    return static_cast<BaseInterface *> (m);
}

BaseInterface *ModuleManager::getIface (const QString &name) {
    BaseModule *m = get (name);
    if (m->getModuleType () != AbstractModule::TypeInterface)
        return NULL;
    return static_cast<BaseInterface *> (m);
}

void ModuleManager::applySettings(QSettings* newSettings)
{
    QList<BaseModule*>::iterator it(items->begin());
    while(it != items->end())
    {
        BaseModule* bm = (*it);
        bm->applySettings(newSettings);
        it++;
    }
}

void ModuleManager::saveSettings(QSettings* settings)
{
    QList<BaseModule*>::iterator it(items->begin());
    while(it != items->end())
    {
        BaseModule* bm = (*it);
        bm->saveSettings(settings);
        it++;
    }
}

void ModuleManager::setModuleName (BaseModule *m, const QString &name) {
    if (m->name == name || get (name)) // no change or name already exists
        return;

    QString oldname = m->name;
    m->name = name;
    emit moduleNameChanged (m, oldname);
}

ThreadBuffer<uint32_t>* ModuleManager::createBuffer(uint32_t _size, uint32_t _chunkSize, int _id)
{
    return new ThreadBuffer<uint32_t>(_size,_chunkSize,_id);
}

void ModuleManager::registerModuleType (const QString &type, ModuleFactory fac, AbstractModule::Type mtype) {
    if (registry.contains (type)) {
        std::cout << "Double registration of module type " << type.toStdString () << ". Ignoring!" << std::endl;
        return;
    }
    ModuleTypeDesc desc = {fac, mtype};
    registry.insert (type, desc);
}


BaseModule *ModuleManager::create (const QString &type, const QString &name) {
    if (get (name)) { // name already exists
        return NULL;
    }

    if (registry.contains (type)) {
        BaseModule *m = (*registry.value (type).fac) (getNextId (), name);
        m->typename_ = type;
        items->push_back (m);
        if(m->getModuleType() == AbstractModule::TypeDAq)
        {
            daqItems->push_back(dynamic_cast<BaseDAqModule*>(m));
        }
        else
        {
            if (interfaceItems->empty ()) // this is the first interface, make it the main interface
                setMainInterface (static_cast<BaseInterface*> (m));
            interfaceItems->push_back(static_cast<BaseInterface*>(m));
        }
        emit moduleAdded (m);
        return m;
    } else
        return NULL;
}

AbstractModule::Type ModuleManager::getModuleTypeClass (const QString &type) const {
    if (registry.contains (type)) {
        return registry.value (type).type;
    } else {
        throw std::invalid_argument ("ModuleManager::getModuleTypeClass");
    }
}

QStringList ModuleManager::getAvailableTypes () const {
    return registry.keys ();
}

int ModuleManager::getNextId () {
    int newId = 0;
    bool found = false;
    while(!found)
    {
        if(get (newId) == NULL)
        {
            found = true;
        }
        else
        {
            newId++;
        }
    }

    return newId;
}
