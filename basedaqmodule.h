#ifndef BASEDAQMODULE_H
#define BASEDAQMODULE_H

#include "basemodule.h"
#include "abstractinterface.h"
#include "scopechannel.h"
#include "threadbuffer.h"
#include "dummyplugin.h"

/*! Base class for data acquisition modules.
 *  This class is used by all modules that receive data from VME modules.
 *  All daq modules feature an output plugin that provides output PluginConnectors
 *  for plugins to connect to. All modules are polled by the RunThread using the #acquire method.
 *  They should retrieve new data from their VME module and pass it to their output plugin (and call the plugin's
 *  \link BasePlugin::userProcess userProcess \endlink method), so the data becomes available on the output connectors.
 *  The output connectors have to be of type PluginConnectorThreadBuffered.
 *
 *  All daq modules communicate via an interface and have the VME base address of their VME module.
 *
 *  Implementors must fill the #channels list with at least as many ScopeChannels as there are output connectors.
 *  Their names will be shown in the Run Setup tab. There should also be a trigger channel so the RunThread calls
 *  the module's #acquire function.
 */
class BaseDAqModule : public BaseModule
{
    Q_OBJECT
public:
    BaseDAqModule (int _id, QString _name);

    /*! retrieve the list of channels made available by this module. */
    QList<ScopeChannel*>* getChannels() { return &channels; }

    /*! return a pointer to this module's output plugin. */
    BasePlugin* getOutputPlugin() { return output; }
    PluginConnector* getRootConnector() { return output->getOutputs()->first(); }

    /*! retrieve the module used for VME communication */
    AbstractInterface *getInterface () const { return iface; }

    /*! Set the interface to be used by this module for VME communication. */
    void setInterface (AbstractInterface *ifa) {
        if (iface)
            disconnect (iface, SIGNAL (destroyed()), this, SLOT (interfaceRemoved ()));
        iface = ifa;
        connect (iface, SIGNAL (destroyed()), SLOT (interfaceRemoved ()));
    }

    // Virtual methods
    /*! return the ThreadBuffer the module writes its data to. \deprecated */
    virtual ThreadBuffer<uint32_t>* getBuffer() = 0;

    /*! Store the channel descriptions for the channels used by this module in the #channels list.
     *  Implementors should call this function from the constructor, as it is not yet called automatically
     */
    virtual void setChannels() = 0;
    //virtual void isTriggered() = 0;

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

    /*! Create the plugin that is used for outputting the collected data to the plugin chain.
     *  A pointer to the plugin should be stored in the #output member.
     */
    virtual void createOutputPlugin() = 0;

    /*! Set the module's VME base address to the given address */
    virtual void setBaseAddress (uint32_t baddr) = 0;

    /*! Retrieve the module's current VME base address */
    virtual uint32_t getBaseAddress () const = 0;
public slots:
    /*! force a trigger to be sent from this module */
    void emitTrigger();

    void interfaceRemoved () { iface = NULL; }

    /*! give subclasses the opportunity to prepare before the next call to #acquire() */
    virtual void prepareForNextAcquisition() = 0;

signals:
    void triggered(ScopeChannel*);

protected:
    AbstractInterface *iface; /*!< The interface used for communication. */
    BasePlugin* output; /*!< The output plugin used by this module. */
    QList<ScopeChannel*> channels; /*!< The list of ScopeChannels provided by this module. */
};

#endif // BASEDAQMODULE_H
