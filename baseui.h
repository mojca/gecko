#ifndef BASEUI_H
#define BASEUI_H

#include <QWidget>

/*! Base class for module UIs.
 *  Derive from this class to create a user interface for your modules.
 */
class BaseUI : public QWidget
{
public:
    BaseUI (QWidget *parent = NULL)
        : QWidget (parent) {}

    virtual ~BaseUI () {}
    /*! Called by the module whenever the configuration changes. Implementors should update the UI to reflect the new configuration. */
    virtual void applySettings() = 0;
};

#endif // BASEUI_H
