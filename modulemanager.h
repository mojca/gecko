#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include <QObject>
#include <iostream>
#include <QSettings>
#include <QList>
#include <QMap>

//#include "abstractmanager.h"
#include "basemodule.h"
#include "basedaqmodule.h"
#include "threadbuffer.h"

class BaseModule;
class BaseDAqModule;
class BaseInterfaceModule;
struct ModuleTypeDesc;


/*! Manager singleton for interface and daq modules. */
class ModuleManager : public QObject
{
    Q_OBJECT
public:
    typedef BaseModule *(*ModuleFactory) (int id, const QString &name);

    ~ModuleManager();

    static ModuleManager *ptr (); /*!< Returns a pointer to the ModuleManager singleton. */
    static ModuleManager &ref (); /*!< Returns a reference to the ModuleManager singleton */

    void identify();

    /*! returns a list of all registered interface modules. */
    const QList<BaseInterfaceModule*>* listInterfaces() { return interfaceItems; }

    /*! returns a list of all registered daq modules. */
    const QList<BaseDAqModule*>* listDaqModules() { return daqItems; }

    /*! returns a list of all registered interface and daq modules. */
    const QList<BaseModule*>* list() { return items; }

    /*! removes all modules. */
    void clear();

    /*! sets the main interface. The main interface module's output 1 is set during data acquisition.
     *  This can be used to generate a veto signal for all modules, so no additional data is collected during readout.
     *  \sa #getMainInterface
     */
    void setMainInterface(BaseInterfaceModule* _mainInterface) { mainInterface = _mainInterface; }

    /*! returns the currently set main interface or NULL if none. */
    BaseInterfaceModule* getMainInterface() { return mainInterface; }

    /*! \overload get(const QString&) */
    BaseModule* get (int id);
    /*! returns the module with the given name, or NULL if no such module exists. */
    BaseModule* get (const QString &);


    /*! \overload getDAq(const QString&) */
    BaseDAqModule *getDAq (int id);
    /*! returns the daq module with the given name, or NULL if no such module exists. */
    BaseDAqModule *getDAq (const QString &name);

    /*! \overload getIface(const QString&) */
    BaseInterfaceModule *getIface (int id);
    /*! returns the interface module with the given name, or NULL if no such module exists. */
    BaseInterfaceModule *getIface (const QString &name);

    /*! assigns a new name to the given module. */
    void setModuleName (BaseModule *m, const QString &name);

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
    void registerModuleType (const QString &type, ModuleFactory fac, AbstractModule::Type mclass);

    /*! returns whether the given type is an interface or a daq type. */
    AbstractModule::Type getModuleTypeClass (const QString &type) const;

    /*! returns a list of available module types. */
    QStringList getAvailableTypes () const;

    /*! creates a new module with the given type. */
    BaseModule *create (const QString &type, const QString &name);

    /*! removes the given module. */
    bool remove (BaseModule *);
    bool remove (int id); /*!< \overload remove(BaseModule*) */
    bool remove (const QString &name); /*!< \overload remove(BaseModule*) */

signals:
    void moduleAdded (BaseModule *); /*!< signalled when a module is added. */
    void moduleRemoved (BaseModule *); /*!< signalled when a module is removed. */
    void moduleNameChanged (BaseModule *, QString); /*! signalled when a module's name is changed. */

private:
	ModuleManager();

	static ModuleManager *inst;

    int getNextId ();

private:
    QList<BaseModule*>* items;
    QList<BaseDAqModule*>* daqItems;
    QList<BaseInterfaceModule*>* interfaceItems;
    QMap<QString, ModuleTypeDesc> registry;
    BaseInterfaceModule* mainInterface;

private: // no copying
	ModuleManager(const ModuleManager &);
	ModuleManager &operator=(const ModuleManager &);
};

class ModuleRegistrar {
public:
    ModuleRegistrar (const QString &type, ModuleManager::ModuleFactory fac, AbstractModule::Type mclass) {
        ModuleManager::ref ().registerModuleType (type, fac, mclass);
    }

private:
    ModuleRegistrar (const ModuleRegistrar &);
    ModuleRegistrar &operator=(const ModuleRegistrar &);
    void *operator new (size_t);
};

#endif // MODULEMANAGER_H
