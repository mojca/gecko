#ifndef BASEUI_H
#define BASEUI_H

#include <QWidget>
#include <QString>

class BaseModule;
class BaseInterfaceModule;

/*! Base class for module UIs.
 *  Derive from this class to create a user interface for your modules.
 */
class BaseUI : public QWidget
{
protected:
    QString name;
    BaseModule *module; /*!< reference to the module instance. */

    // Virtual methods
    /*! create the user interface. This function must be called from the UI's constructor */
    virtual void createUI() = 0;

public:
    BaseUI(BaseModule* _module);
    BaseUI(BaseInterfaceModule* _module);
    ~BaseUI() {}
    virtual void applySettings() = 0;
};

#endif // BASEUI_H
