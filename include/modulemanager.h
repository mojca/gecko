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

    /*! returns a list of all registered interface and daq modules. */
    const QList<AbstractModule*>* list() { return items; }

    /*! removes all modules. */
    void clear();

    /*! \overload get(const QString&) */
    AbstractModule* get (int id);
    /*! returns the module with the given name, or NULL if no such module exists. */
    AbstractModule* get (const QString &);

    /*! assigns a new name to the given module. */
    void setModuleName (AbstractModule *m, const QString &name);

    /*! Iterates through all registered modules and calls BaseModule::applySettings on each of them. */
    void applySettings(QSettings*);

    /*! Iterates through all registered modules and calls BaseModule::saveSettings on each of them. */
    void saveSettings(QSettings*);

    /*! Creates a new ThreadBuffer with the given parameters. \deprecated */
    ThreadBuffer<uint32_t>* createBuffer(uint32_t _size, uint32_t _chunkSize, int _id);

    // module instantiation facilities
    /*! Registers a new module type. Each type has to be registered to be visible to users.
     *  \param type   name that will be used to identify the type
     *  \param fac    method to create instances of the type
     *  \param mclass defines whether this type is an interface or a daq type
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

    const QSet<const EventSlot*>& getMandatorySlots () const { return mandatoryslots; }

    /*! Adds the given module to the list of triggers. A data available signal from this module will cause an event to be generated. */
    void setTrigger (AbstractModule* mod, bool istrg) { if (istrg) triggers.insert (mod); else triggers.remove (mod); }
    /*! Removes the given module from the trigger list. */
    bool isTrigger (AbstractModule* mod) const { return triggers.contains (mod); }

    const QSet<AbstractModule*>& getTriggers () const { return triggers; }

signals:
    void moduleAdded (AbstractModule *); /*!< signalled when a module is added. */
    void moduleRemoved (AbstractModule *); /*!< signalled when a module is removed. */
    void moduleNameChanged (AbstractModule *, QString); /*! signalled when a module's name is changed. */

private:
    ModuleManager();

    static ModuleManager *inst;

    int getNextId ();

private:
    list_type* items;
    QMap<QString, ModuleTypeDesc> registry;
    QSet<AbstractModule*> triggers;
    QSet<const EventSlot*> mandatoryslots;

private: // no copying
	ModuleManager(const ModuleManager &);
	ModuleManager &operator=(const ModuleManager &);
};

class ModuleRegistrar {
public:
    ModuleRegistrar (const QString &type, ModuleManager::ModuleFactory fac) {
        ModuleManager::ref ().registerModuleType (type, fac);
    }

private:
    ModuleRegistrar (const ModuleRegistrar &);
    ModuleRegistrar &operator=(const ModuleRegistrar &);
    void *operator new (size_t);
};

#endif // MODULEMANAGER_H
