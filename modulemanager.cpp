#include "modulemanager.h"
#include "abstractmodule.h"
#include "threadbuffer.h"

#include <stdexcept>
#include <QStringList>

ModuleManager *ModuleManager::inst = NULL;

struct ModuleTypeDesc {
    ModuleManager::ModuleFactory fac;
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
    items = new list_type;

    std::cout << "Instantiated ModuleManager" << std::endl;
}

ModuleManager::~ModuleManager()
{
    clear();
    delete items;
}

void ModuleManager::clear()
{
    while (!items->empty ())
        remove (items->first ());
}

void ModuleManager::identify()
{
    std::cout << "ModuleManager" << std::endl;
}

bool ModuleManager::remove(AbstractModule* rmModule)
{
    list_type::iterator it(items->begin());
    while(it != items->end())
    {
        if((*it) == rmModule)
        {
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
    AbstractModule* am;
    if((am = get (id)) != NULL)
    {
        remove(am);
        return true;
    }
    return false;
}

bool ModuleManager::remove (const QString &name)
{
    AbstractModule* am;
    if((am = get (name)) != NULL)
    {
        remove(am);
        return true;
    }
    return false;
}


AbstractModule* ModuleManager::get (int id)
{
    list_type::iterator it(items->begin());
    while(it != items->end())
    {
        AbstractModule* bm = (*it);
        if(bm->getId() == id)
        {
            return *it;
        }
        it++;
    }
    return NULL;
}

AbstractModule* ModuleManager::get (const QString &name)
{
    list_type::iterator it(items->begin());
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

void ModuleManager::applySettings(QSettings* newSettings)
{
    list_type::iterator it(items->begin());
    while(it != items->end())
    {
        AbstractModule* bm = (*it);
        bm->applySettings(newSettings);
        it++;
    }
}

void ModuleManager::saveSettings(QSettings* settings)
{
    list_type::iterator it(items->begin());
    while(it != items->end())
    {
        AbstractModule* bm = (*it);
        bm->saveSettings(settings);
        it++;
    }
}

void ModuleManager::setModuleName (AbstractModule *m, const QString &name) {
    if (m->getName () == name || get (name)) // no change or name already exists
        return;

    QString oldname = m->getName ();
    m->setName (name);
    emit moduleNameChanged (m, oldname);
}

ThreadBuffer<uint32_t>* ModuleManager::createBuffer(uint32_t _size, uint32_t _chunkSize, int _id)
{
    return new ThreadBuffer<uint32_t>(_size,_chunkSize,_id);
}

void ModuleManager::registerModuleType (const QString &type, ModuleFactory fac) {
    if (registry.contains (type)) {
        std::cout << "Double registration of module type " << type.toStdString () << ". Ignoring!" << std::endl;
        return;
    }
    ModuleTypeDesc desc = {fac};
    registry.insert (type, desc);
}


AbstractModule *ModuleManager::create (const QString &type, const QString &name) {
    if (get (name)) { // name already exists
        return NULL;
    }

    if (registry.contains (type)) {
        AbstractModule *m = (*registry.value (type).fac) (getNextId (), name);
        m->setTypeName (type);
        items->push_back (m);
        emit moduleAdded (m);
        return m;
    } else
        return NULL;
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
