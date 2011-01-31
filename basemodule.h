#ifndef BASEMODULE_H
#define BASEMODULE_H

#include <QString>
#include <QThread>
#include <QSettings>

#include "abstractmodule.h"
#include "baseui.h"

class ScopeChannel;
class BaseUI;
class ModuleManager;

/*! base class for all modules.
 *  Each module is registered with the module manager to allow generalised access.
 *  To implement a new module, do not inherit from this class but from either BaseDAqModule or BaseInterfaceModule.
 *  You will need to call #setUI in your constructor because the main window needs the UI right after construction of the module.
 */
class BaseModule : public QObject, public virtual AbstractModule
{
    Q_OBJECT

public:
    BaseModule(int _id, QString _name = "Base Module");
    virtual ~BaseModule() {}

    /*! Return the module's id, as assigned by the module manager. */
    int getId() { return id; }

    /*! Retrieve the module's name. The module is referenced by this name in the configuration file. */
    QString getName() { return name; }

    /*! return the module's type as a string. */
    QString getTypeName () {return typename_; }

    /*! return a pointer the ui set via #setUI. */
    BaseUI* getUI() { return ui; }

    /*! Save the module settings to the given QSettings object.
     *  The implementation should read the subsection named like the module instance
     *  and save all settings inside to a local data structure, because lifetime of the settings object 
     *  is not guaranteed to be longer than the lifetime of this object.
     *  \sa #applySettings, ModuleManager::saveSettings
     */
    virtual void saveSettings(QSettings*) {}

    /*! Load the module settings from the given QSettings object.
     *  The implementation should create a new subsection named like the module instance
     *  and save all settings inside this section.
     *  \sa #saveSettings, ModuleManager::applySettings
     */
    virtual void applySettings(QSettings*) {}

protected:
    /*! set the module's UI object. This object is shown in the main window when the module's tree entry is selected. */
    void setUI (BaseUI *_ui) { ui = _ui; }

private:
    int id;
    QString name;
    QString typename_;
    BaseUI *ui;

    friend class ModuleManager;
};

#endif // BASEMODULE_H
