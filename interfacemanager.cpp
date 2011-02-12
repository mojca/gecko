#include "interfacemanager.h"
#include "abstractinterface.h"

#include <QStringList>
#include <iostream>

InterfaceManager *InterfaceManager::inst;

struct InterfaceTypeDesc {
    InterfaceManager::InterfaceFactory fac;
};

InterfaceManager *InterfaceManager::ptr () {
        if (inst == NULL)
                inst = new InterfaceManager ();
        return inst;
}

InterfaceManager &InterfaceManager::ref () {
        return *ptr ();
}

InterfaceManager::InterfaceManager ()
{
    items_ = new QList<AbstractInterface*>;

    std::cout << "Instantiated InterfaceManager" << std::endl;
    mainInterface_ = NULL;
}

InterfaceManager::~InterfaceManager()
{
    clear();
    delete items_;
}

void InterfaceManager::clear()
{
    while (!items_->empty ())
        remove (items_->first ());
}

bool InterfaceManager::remove(AbstractInterface* rmIf)
{
    QList<AbstractInterface*>::iterator it(items_->begin());
    while(it != items_->end())
    {
        if((*it) == rmIf)
        {
            items_->erase(it);
            emit interfaceRemoved (rmIf);
            rmIf->deleteLater ();
            return true;
        }
        it++;
    }
    return false;
}

bool InterfaceManager::remove (int id)
{
    AbstractInterface* ai;
    if((ai = get (id)) != NULL)
    {
        remove(ai);
        return true;
    }
    return false;
}

bool InterfaceManager::remove (const QString &name)
{
    AbstractInterface* ai;
    if((ai = get (name)) != NULL)
    {
        remove(ai);
        return true;
    }
    return false;
}


AbstractInterface* InterfaceManager::get (int id)
{
    QList<AbstractInterface*>::iterator it(items_->begin());
    while(it != items_->end())
    {
        AbstractInterface* ai = (*it);
        if(ai->getId() == id)
        {
            return ai;
        }
        it++;
    }
    return NULL;
}

AbstractInterface* InterfaceManager::get (const QString &name)
{
    QList<AbstractInterface*>::iterator it(items_->begin());
    while(it != items_->end())
    {
        if((*it)->getName() == name)
        {
            return *it;
        }
        it++;
    }
    return NULL;
}

void InterfaceManager::applySettings(QSettings* newSettings)
{
    QList<AbstractInterface*>::iterator it(items_->begin());
    while(it != items_->end())
    {
        AbstractInterface* ai = (*it);
        ai->applySettings(newSettings);
        it++;
    }
}

void InterfaceManager::saveSettings(QSettings* settings)
{
    QList<AbstractInterface*>::iterator it(items_->begin());
    while(it != items_->end())
    {
        AbstractInterface* ai = (*it);
        ai->saveSettings(settings);
        it++;
    }
}

void InterfaceManager::setInterfaceName (AbstractInterface *i, const QString &name) {
    if (i->getName () == name || get (name)) // no change or name already exists
        return;

    QString oldname = i->getName ();
    i->setName (name);
    emit interfaceNameChanged (i, oldname);
}

void InterfaceManager::registerInterfaceType (const QString &type, InterfaceFactory fac) {
    if (registry_.contains (type)) {
        std::cout << "Double registration of module type " << type.toStdString () << ". Ignoring!" << std::endl;
        return;
    }
    InterfaceTypeDesc desc = {fac};
    registry_.insert (type, desc);
}


AbstractInterface *InterfaceManager::create (const QString &type, const QString &name) {
    if (get (name)) { // name already exists
        return NULL;
    }

    if (registry_.contains (type)) {
        AbstractInterface *i = (*registry_.value (type).fac) (getNextId (), name);
        i->setTypeName (type);
        if (items_->empty ()) // this is the first interface, make it the main interface
            setMainInterface (i);
        items_->push_back (i);
        emit interfaceAdded (i);
        return i;
    } else {
        return NULL;
    }
}

QStringList InterfaceManager::getAvailableTypes () const {
    return registry_.keys ();
}

int InterfaceManager::getNextId () {
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
