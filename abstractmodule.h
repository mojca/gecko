#ifndef ABSTRACTMODULE_H
#define ABSTRACTMODULE_H

#include <QString>

/*! Abstract base class for modules.
 *  To create a new module, please inherit from the more specialised classes BaseInterfaceModule or BaseDAqModule.
 */
class AbstractModule
{
public:
    /*! Module type enumeration. Modules are either VME interfaces or daq modules controlling VME modules via an interface */
    enum Type {TypeInterface, TypeDAq};

    virtual ~AbstractModule() {}

    /*! return the module's id as assigned by the ModuleManager. */
    virtual int getId() = 0;

    /*! return the module's name */
    virtual QString getName() = 0;

    /*! return the module's type */
    virtual Type getModuleType () = 0;
 };

#endif // ABSTRACTMODULE_H
