#ifndef INTERFACEMANAGER_H
#define INTERFACEMANAGER_H

#include <QString>
#include <QObject>
#include <QMap>

class AbstractInterface;
class QSettings;
struct InterfaceTypeDesc;

/*! Singleton class for managing interfaces. */
class InterfaceManager : public QObject{
    Q_OBJECT
public:
    typedef AbstractInterface *(*InterfaceFactory) (int id, const QString &name);

    static InterfaceManager *ptr (); /*!< Returns a pointer to the ModuleManager singleton. */
    static InterfaceManager &ref (); /*!< Returns a reference to the ModuleManager singleton */

    /*! returns a list of all registered interfaces. */
    const QList<AbstractInterface*>* list () const { return items_; }

    /*! removes all modules. */
    void clear();

    /*! sets the main interface. The main interface module's output 1 is set during data acquisition.
     *  This can be used to generate a veto signal for all modules, so no additional data is collected during readout.
     *  \sa #getMainInterface
     */
    void setMainInterface(AbstractInterface* mi) { mainInterface_ = mi; }

    /*! returns the currently set main interface or NULL if none. */
    AbstractInterface* getMainInterface() { return mainInterface_; }

    /*! \overload get(const QString&) */
    AbstractInterface* get (int id);
    /*! returns the interface with the given name, or NULL if no such module exists. */
    AbstractInterface* get (const QString &);

    /*! assigns a new name to the given module. */
    void setInterfaceName (AbstractInterface *m, const QString &name);

    /*! Iterates through all registered interfaces and calls BaseModule::applySettings on each of them. */
    void applySettings(QSettings*);

    /*! Iterates through all registered interfaces and calls BaseModule::saveSettings on each of them. */
    void saveSettings(QSettings*);

    /*! Registers a new interface type. Each type has to be registered to be visible to users.
     *  \param type   name that will be used to identify the type
     *  \param fac    method to create instances of the type
     *  \sa InterfaceRegistrar
     */
    void registerInterfaceType (const QString &type, InterfaceFactory fac);

    /*! returns a list of available module types. */
    QStringList getAvailableTypes () const;

    /*! creates a new module with the given type. */
    AbstractInterface *create (const QString &type, const QString &name);

    /*! removes the given module. */
    bool remove (AbstractInterface *);
    bool remove (int id); /*!< \overload remove(AbstractInterface*) */
    bool remove (const QString &name); /*!< \overload remove(AbstractInterface*) */

signals:
    void interfaceAdded (AbstractInterface *); /*!< signalled when an interface is added. */
    void interfaceRemoved (AbstractInterface *); /*!< signalled when an interface is removed. */

    /*! signalled when an interface's name is changed.
     *  \param intf    Pointer to the interface
     *  \param newname Old name of the interface
     */
    void interfaceNameChanged (AbstractInterface *intf, QString newname);

private:
    InterfaceManager();
    ~InterfaceManager();

    static InterfaceManager *inst;

    int getNextId ();

private:
    QList<AbstractInterface*>* items_;
    QMap<QString, InterfaceTypeDesc> registry_;
    AbstractInterface* mainInterface_;

private: // no copying
    InterfaceManager (const InterfaceManager &);
    InterfaceManager &operator=(const InterfaceManager &);

};

class InterfaceRegistrar {
public:
    InterfaceRegistrar (const QString &type, InterfaceManager::InterfaceFactory fac) {
        InterfaceManager::ref ().registerInterfaceType (type, fac);
    }

private:
    InterfaceRegistrar (const InterfaceRegistrar &);
    InterfaceRegistrar &operator=(const InterfaceRegistrar &);
    void *operator new (size_t);
};

#endif // INTERFACEMANAGER_H
