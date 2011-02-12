#ifndef ABSTRACTPLUGIN_H
#define ABSTRACTPLUGIN_H

#include <QWidget>
#include <QString>
#include <QMap>
#include <QVariant>

#include "pluginconnector.h"

class PluginConnector;
class PluginManager;
class QSettings;

/*! Abstract base class for plugins. */
class AbstractPlugin : public QWidget
{
    Q_OBJECT
public:
    typedef QMap<QString, QVariant::Type> AttributeMap;
    typedef QMap<QString, QVariant> Attributes;

    /*! The plugin groups */
    enum Group {GroupDSP, GroupCache, GroupPack, GroupPlot, GroupOutput, GroupDemux, GroupAux, GroupUnspecified};

    AbstractPlugin (QWidget *_parent) : QWidget (_parent) {}
    virtual ~AbstractPlugin() {}

    /*! Return the plugin's id as assigned by the PluginManager. */
    virtual int getId() const = 0;
    /*! Return the plugin's name. */
    virtual QString getName() const = 0;
    /*! return the plugin's type name as string */
    virtual const QString &getTypeName () const = 0;

    /*! Save the plugin settings to the given QSettings object.
     *  The implementation should read the subsection named like the plugin instance
     *  and save all settings inside to a local data structure, because lifetime of the settings object
     *  is not guaranteed to be longer than the lifetime of this object.
     *  \sa #applySettings, PluginManager::saveSettings
     */
    virtual void saveSettings(QSettings*) = 0;

    /*! Load the plugin settings from the given QSettings object.
     *  The implementation should create a new subsection named like the plugin instance
     *  and save all settings inside this section.
     *  \sa #saveSettings, PluginManager::applySettings
     */
    virtual void applySettings(QSettings*) = 0;

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

    /*! update the connections displayed in the input and output boxes. */
    virtual void updateDisplayedConnections() = 0;

    /*! enable or disable those UI elements that should not be accessible during a run. */
    virtual void setConfigEnabled (bool enabled) = 0;

    /*! Make the plugin initialise its UI. */
    virtual void createUI() = 0;

signals:
    void jumpToPluginRequested (AbstractPlugin *);

protected:

    // TODO: Do this The Right Way (tm)
    virtual void setName (QString newName) = 0;
    virtual void setTypeName (QString newType) = 0;

    friend class PluginManager;
};

#endif // ABSTRACTPLUGIN_H
