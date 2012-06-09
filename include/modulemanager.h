/*
Copyright 2011 Bastian Loeher, Roland Wirth

This file is part of GECKO.

GECKO is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

GECKO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include <QObject>
#include <iostream>
#include <QSettings>
#include <QList>
#include <QMap>
#include <QSet>

#include <stdint.h>

class AbstractInterface;
struct ModuleTypeDesc;
class AbstractModule;
class EventSlot;
class ScopeMainWindow;

template<typename T> class ThreadBuffer;


/*! Manager singleton for daq modules. */
class ModuleManager : public QObject
{
    Q_OBJECT
public:
    typedef AbstractModule *(*ModuleFactory) (int id, const QString &name);
    typedef QList<AbstractModule*> list_type;

    ~ModuleManager();

    static ModuleManager *ptr (); /*!< Returns a pointer to the ModuleManager singleton. */
    static ModuleManager &ref (); /*!< Returns a reference to the ModuleManager singleton */

    void identify();

    /*! returns a list of all registered daq modules. */
    const QList<AbstractModule*>* list() { return items; }

    /*! removes all modules. */
    void clear();

    /*! \overload get(const QString&) */
    AbstractModule* get (int id);
    /*! returns the module with the given name, or NULL if no such module exists. */
    AbstractModule* get (const QString &);

    /*! assigns a new name to the given module. */
    void setModuleName (AbstractModule *m, const QString &name);

    /*! Iterates through all registered modules and calls AbstractModule::applySettings on each of them. */
    void applySettings(QSettings*);

    /*! Iterates through all registered modules and calls AbstractModule::saveSettings on each of them. */
    void saveSettings(QSettings*);

    // module instantiation facilities
    /*! Registers a new module type. Each type has to be registered to be visible to users.
     *  \param type   name that will be used to identify the type
     *  \param fac    method to create instances of the type
     *  \sa ModuleRegistrar
     */
    void registerModuleType (const QString &type, ModuleFactory fac);

    /*! returns a list of available module types. */
    QStringList getAvailableTypes () const;

    /*! creates a new module with the given type. */
    AbstractModule *create (const QString &type, const QString &name);

    /*! removes the given module. */
    bool remove (AbstractModule *);
    bool remove (int id); /*!< \overload remove(BaseModule*) */
    bool remove (const QString &name); /*!< \overload remove(BaseModule*) */

    /*! Makes a slot mandatory. If this slot does not contain any data, the event is discarded. */
    void setMandatory (const EventSlot* sl, bool mandatory) { if (mandatory) mandatoryslots.insert (sl); else mandatoryslots.remove (sl); }
    /*! Returns whether the given channel is mandatory. */
    bool isMandatory (const EventSlot* sl) const { return mandatoryslots.contains (sl); }

    /*! returns a set of all mandatory slots. */
    const QSet<const EventSlot*>& getMandatorySlots () const { return mandatoryslots; }

    /*! Adds the given module to the list of triggers. A data available signal from this module will cause an event to be generated. */
    void setTrigger (AbstractModule* mod, bool istrg) { if (istrg) triggers.insert (mod); else triggers.remove (mod); }
    /*! Removes the given module from the trigger list. */
    bool isTrigger (AbstractModule* mod) const { return triggers.contains (mod); }

    /*! returns a set of all modules that act as triggers. */
    const QSet<AbstractModule*>& getTriggers () const { return triggers; }

    void setMainWindow (ScopeMainWindow *mw) { mainWindow = mw; }
    ScopeMainWindow *getMainWindow() const { return mainWindow; }

signals:
    void moduleAdded (AbstractModule *); /*!< signalled when a module is added. */
    void moduleRemoved (AbstractModule *); /*!< signalled when a module is removed. */
    void moduleNameChanged (AbstractModule *, QString); /*! signalled when a module's name is changed. */

private:
    ModuleManager();

    static ModuleManager *inst;

    int getNextId ();

private:
    ScopeMainWindow *mainWindow;
    list_type* items;
    QMap<QString, ModuleTypeDesc> registry;
    QSet<AbstractModule*> triggers;
    QSet<const EventSlot*> mandatoryslots;

private: // no copying
	ModuleManager(const ModuleManager &);
	ModuleManager &operator=(const ModuleManager &);
};

/*! The ModuleRegistrar class is a helper class to facilitate the registering of modules on application startup.
 *  It is meant to be used as static object in global scope and calls ModuleManager::registerModuleType on
 *  construction. The module type registered that way is usable from the start of the program.
 *
 *  To use it, add the following line to the module's cpp file
 *  \code static ModuleRegistrar reg ("MyModule", MyModule::create); \endcode
 *
 *  \sa ModuleManager::registerModuleType
*/
class ModuleRegistrar {
public:
    /*! Constructor, calls ModuleManager::registerModuleType with the given arguments.
        \param type name of the module type, should be human-friendly
        \param fac factory method for the module type
    */
    ModuleRegistrar (const QString &type, ModuleManager::ModuleFactory fac) {
        ModuleManager::ref ().registerModuleType (type, fac);
    }

private: // no copy, no new
    ModuleRegistrar (const ModuleRegistrar &);
    ModuleRegistrar &operator=(const ModuleRegistrar &);
    void *operator new (size_t);
};

#endif // MODULEMANAGER_H
