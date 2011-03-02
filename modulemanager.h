#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include <QObject>
#include <iostream>
#include <QSettings>
#include <QList>
#include <QMap>

#include <stdint.h>

class AbstractInterface;
struct ModuleTypeDesc;
class AbstractModule;

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
