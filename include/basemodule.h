#ifndef BASEMODULE_H
#define BASEMODULE_H

#include <QString>
#include <QVector>

#include "abstractmodule.h"
#include "abstractinterface.h"
#include "pluginconnector.h"
#include "runmanager.h"
#include "eventbuffer.h"
#include "outputplugin.h"

class BaseUI;
class ScopeChannel;
class ModuleManager;

/*! base class for all modules.
 *  Each module is registered with the module manager to allow generalised access.
 *  To implement a new DAQ module inherit from this class.
 *  You will need to call #setUI in your constructor because the main window needs the UI right after construction of the module.
 *
 *  The OutputPlugin created by the #createOutputPlugin method uses the registered EventSlots to derive the naming and
 *  number of its output connectors. Therefore you should register all EventSlots (using #addSlot) prior to calling #createOutputPlugin.
 *  This should also happen inside the constructor.
 */
class BaseModule : public AbstractModule
{
    Q_OBJECT

public:
    BaseModule(int _id, QString _name)
    : iface (NULL)
    , output (NULL)
    , id (_id)
    , name (_name)
    , ui (NULL)
    {
    }

    virtual ~BaseModule() {
        EventBuffer *evbuf = RunManager::ref ().getEventBuffer ();
        QList<EventSlot*> s (*evbuf->getEventSlots(this));
        for (QList<EventSlot*>::const_iterator i = s.begin (); i != s.end (); ++i)
            evbuf->destroyEventSlot (*i);

        if (output)
            delete output;
        output = NULL;
    }

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

    /*! Returns a list of all slots belonging to this module. */
    QList<const EventSlot*> getSlots () const {
        const QList<EventSlot*>* s = RunManager::ref ().getEventBuffer ()->getEventSlots(this);
        QList<const EventSlot*> out;
#if QT_VERSION >= 0x040700
        out.reserve (s->size ());
#endif
        for (QList<EventSlot*>::const_iterator i = s->begin (); i != s->end (); ++i)
            out.append (*i);

        return out;
    }


    OutputPlugin* getOutputPlugin () const { return output; }

public slots:
    virtual void prepareForNextAcquisition () {}

private slots:
    void interfaceRemoved () { iface = NULL; }

protected:
    /*! Adds an event buffer slot to the module. */
    const EventSlot* addSlot (QString name, PluginConnector::DataType dtype) {
        return RunManager::ref ().getEventBuffer()->registerSlot (this, name, dtype);
    }

    /*! Create the output plugin for this module. The output plugin forms the
        conduit between the module and the plugin part of the program. It makes the slots exported by the module
        available to other plugins. It also handles the transition between the run thread and the plugin thread.

        \remarks only call this function after registering all the slots the module will export as the number
        and names of connectors are derived from them. The plugin will be deleted when the module is destroyed.
     */
    void createOutputPlugin () {
        output = new OutputPlugin (this);
    }

    void setUI (BaseUI *_ui) { ui = _ui; }
    void setName (QString newName) { name = newName; }
    void setTypeName (QString newTypeName) { typename_ = newTypeName; }

private:
    AbstractInterface *iface;
    OutputPlugin *output;

    int id;
    QString name;
    QString typename_;
    BaseUI *ui;
};

#endif // BASEMODULE_H
