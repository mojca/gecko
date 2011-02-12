#ifndef BASEMODULE_H
#define BASEMODULE_H

#include <QString>
#include <QList>

#include "abstractmodule.h"
#include "abstractinterface.h"
#include "abstractplugin.h"

class BaseUI;
class ScopeChannel;
class ModuleManager;

/*! base class for all modules.
 *  Each module is registered with the module manager to allow generalised access.
 *  To implement a new DAQ module inherit from this class.
 *  You will need to call #setUI in your constructor because the main window needs the UI right after construction of the module.
 */
class BaseModule : public AbstractModule
{
    Q_OBJECT

public:
    BaseModule(int _id, QString _name = "Base Module")
    : iface (NULL)
    , id (_id)
    , name (_name)
    , ui (NULL)
    {
    }

    virtual ~BaseModule() {}

    int getId () const { return id; }
    const QString& getName() const { return name; }
    QString getTypeName () const {return typename_; }

    BaseUI* getUI() const { return ui; }

    virtual void saveSettings(QSettings*) {}
    virtual void applySettings(QSettings*) {}

    AbstractInterface *getInterface () const { return iface; }
    void setInterface (AbstractInterface *ifa) {
        if (iface)
            disconnect (iface, SIGNAL (destroyed()), this, SLOT (interfaceRemoved ()));
        iface = ifa;
        connect (iface, SIGNAL (destroyed()), SLOT (interfaceRemoved ()));
    }

    QList<ScopeChannel*>* getChannels () { return &channels_; }

    ThreadBuffer<uint32_t>* getBuffer () { return NULL; }

    AbstractPlugin* getOutputPlugin () const { return output; }
    PluginConnector *getRootConnector () const {return output->getOutputs ()->first (); }

public slots:
    void prepareForNextAcquisition () {}

protected:
    void setUI (BaseUI *_ui) { ui = _ui; }
    void setName (QString newName) { name = newName; }
    void setTypeName (QString newTypeName) { typename_ = newTypeName; }

protected:
    // TODO: DO this The Right Way (tm)
    AbstractPlugin *output;

private:
    AbstractInterface *iface;
    int id;
    QString name;
    QString typename_;
    BaseUI *ui;
    QList<ScopeChannel*> channels_;
};

#endif // BASEMODULE_H
