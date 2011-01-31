#ifndef ABSTRACTPLUGIN_H
#define ABSTRACTPLUGIN_H

#include <QString>
#include <QMap>
#include <QVariant>

#include "pluginconnector.h"

class PluginConnector;
class PluginManager;

/*! Abstract base class for plugins. */
class AbstractPlugin
{

public:
    typedef QMap<QString, QVariant::Type> AttributeMap;
    typedef QMap<QString, QVariant> Attributes;

    /*! The plugin groups */
    enum Group {GroupDSP, GroupCache, GroupPack, GroupPlot, GroupOutput, GroupDemux, GroupAux, GroupUnspecified};

    virtual ~AbstractPlugin() {}

    /*! Return the plugin's id as assigned by the PluginManager. */
    virtual int getId() const = 0;
    /*! Return the plugin's name. */
    virtual QString getName() const = 0;
    /*! Return a list of the plugin's input connectors. */
    virtual QList<PluginConnector*>* getInputs() = 0;
    /*! Return a list of the plugin's output connectors. */
    virtual QList<PluginConnector*>* getOutputs() = 0;
    /*! return the plugin's type */
    virtual Group getPluginGroup () = 0;

    /*! Make the plugin process an event. */
    virtual void process() = 0;

    /*! Add a connector to the plugin. */
    virtual void addConnector(PluginConnector*) = 0;

    /*! Return a map of the plugin's attributes and value types. */
    virtual AttributeMap getAttributeMap () const = 0;
    /*! Return a map of the plugin's attributes and values. */
    virtual Attributes getAttributes () const = 0;

    /*! Make the plugin initialise its UI. */
    virtual void createUI() = 0;

};

#endif // ABSTRACTPLUGIN_H
