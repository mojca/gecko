#ifndef ABSTRACTMODULE_H
#define ABSTRACTMODULE_H

#include <QObject>
#include <QString>
#include <stdint.h>

class QSettings;
template<typename T> class QVector;
template<typename T> class ThreadBuffer;

class AbstractInterface;
class BaseUI;
class EventSlot;
class OutputPlugin;
class PluginConnector;
class ScopeChannel;

/*! Base class for data acquisition modules.
 *  This class is used by all modules that receive data from VME modules.
 *  All daq modules feature an output plugin that provides output PluginConnectors
 *  for plugins to connect to. All modules are polled by the RunThread using the #acquire method.
 *  They should retrieve new data from their VME module and pass it back to the runthread using the supplied Event object.
 *  This is done by calling Event::put with the EventSlot object the data belongs to. The PluginThread will make the data
 *  available on the output connectors of the output plugin.
 *
 *  All daq modules communicate via an interface and have the VME base address of their VME module.
 *
 *  Implementors must fill the #channels list with at least as many ScopeChannels as there are output connectors.
 *  Their names will be shown in the Run Setup tab. There should also be a trigger channel so the RunThread calls
 *  the module's #acquire function.
 */
class AbstractModule : public QObject
{
    Q_OBJECT
public:
    virtual ~AbstractModule() {}

    /*! Return the module's id, as assigned by the module manager. */
    virtual int getId() const = 0;

    /*! Retrieve the module's name. The module is referenced by this name in the configuration file. */
    virtual const QString& getName() const = 0;

    /*! return the module's type as a string. */
    virtual QString getTypeName () const = 0;

    /*! return a pointer the ui set via #setUI. */
    virtual BaseUI* getUI() const = 0;

    /*! retrieve the list of slots made available by this module. */
    virtual QVector<const EventSlot*> getSlots() const = 0;

    /*! return a pointer to this module's output plugin. */
    virtual OutputPlugin* getOutputPlugin() const = 0;

    /*! retrieve the module used for VME communication */
    virtual AbstractInterface *getInterface () const = 0;

    /*! Set the interface to be used by this module for VME communication. */
    virtual void setInterface (AbstractInterface *ifa) = 0;

    /*! Save the module settings to the given QSettings object.
     *  The implementation should read the subsection named like the module instance
     *  and save all settings inside to a local data structure, because lifetime of the settings object
     *  is not guaranteed to be longer than the lifetime of this object.
     *  \sa #applySettings, ModuleManager::saveSettings
     */
    virtual void saveSettings(QSettings*) = 0;

    /*! Load the module settings from the given QSettings object.
     *  The implementation should create a new subsection named like the module instance
     *  and save all settings inside this section.
     *  \sa #saveSettings, ModuleManager::applySettings
     */
    virtual void applySettings(QSettings*) = 0;

    /*! Retrieve data from the vme module and put it into the buffer.
     *  This function is called repeatedly from the RunThread to get the data as soon as it is available.
     */
    virtual int acquire() = 0;

    /*! Return whether data is available for retrieval.
     *  This function is called repeatedly from the RunThread to determine whether new data is available.
     */
    virtual bool dataReady() = 0;

    /*! Perform a soft reset on the device
     *  \sa #configure()
     */
    virtual int reset() = 0;

    /*! Configure the device using the information supplied by #applySettings.
     *  Implementors should use the local configuration data structures to initialise the vme module.
     */
    virtual int configure() = 0;

    /*! Set the module's VME base address to the given address */
    virtual void setBaseAddress (uint32_t baddr) = 0;

    /*! Retrieve the module's current VME base address */
    virtual uint32_t getBaseAddress () const = 0;

public slots:
    /*! give subclasses the opportunity to prepare before the next call to #acquire() */
    virtual void prepareForNextAcquisition() = 0;

signals:
    void triggered(ScopeChannel*);

protected:
    /*! set the module's UI object. This object is shown in the main window when the module's tree entry is selected. */
    virtual void setUI (BaseUI *_ui) = 0;

    // TODO: Do this The Right Way (tm)
    virtual void setName (QString newName) = 0;
    virtual void setTypeName (QString newTypeName) = 0;

    friend class ModuleManager;
 };

#endif // ABSTRACTMODULE_H
