/*
Copyright 2011 Bastian Loeher, Roland Wirth

This file is part of GECKO.

GECKO is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

GECKO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QSettings>
#include <QMap>

#include "baseplugin.h"
#include "pluginconnector.h"
#include "modulemanager.h"

class AbstractPlugin;

class DummyPlugin;
class PluginConnector;
class PluginThread;
struct PluginTypeDesc;


/*! Manages the plugins for data processing.
 *  The PluginManager is the central registry for all plugins and plugin types.
 *  It creates and deletes plugins and performs configuration updates.
 *
 *  \sa ModuleManager
 */
class PluginManager : public QObject
{
    Q_OBJECT
public:
    /*! Typedef for Plugin factory methods.
     *  These methods are used to create instances of registered plugin types
     */
    typedef AbstractPlugin *(*PluginFactory) (int id, const QString &name, const AbstractPlugin::Attributes &attrs);
    ~PluginManager();

    static PluginManager *ptr (); /*!< return a pointer to the singleton instance. */
    static PluginManager &ref (); /*!< return a reference to the singleton instance. */

    /*! return a list of all plugins. */
    const QList<AbstractPlugin*>* list() { return items; }

    /*! delete all plugins. */
    void clear();

    /*! get the plugin with the specified id. */
    AbstractPlugin* get (int id);
    AbstractPlugin* get (const QString &name); /*!< \overload */

    /*! change the name of the given plugin */
    void setPluginName (AbstractPlugin *p, const QString &name);

    /*! make all plugins load their settings from the specified QSettings object */
    void applySettings(QSettings* s);

    /*! make all plugins save their settings to the specified QSettings object */
    void saveSettings(QSettings* s);

    /*! register a plugin type with the PluginManager.
     *  All but the demux plugins must be registered with the manager to be usable.
     *
     *  \param type  The name by which the plugin type will be known
     *  \param fac   The factory method that returns an instance of the plugin when called
     *  \param group The group under which the plugin will be shown
     *  \param attrs The attributes that are meaningful for the plugin and their types
     *
     *  \sa PluginRegistrar
     */
    void registerPluginType (const QString &type, PluginFactory fac, const AbstractPlugin::Group, const AbstractPlugin::AttributeMap & attrs);

    /*! create a plugin of the specified type with the given attributes. */
    AbstractPlugin *create (const QString &type, const QString &name, const AbstractPlugin::Attributes &attrs = AbstractPlugin::Attributes ());

    /*! list the attributes that are meaningful for the given plugin type. */
    AbstractPlugin::AttributeMap getAttributeMap (const QString &type);

    /*! list the available plugin types. */
    QStringList getAvailableTypes () const;

    /*! list the available plugin types of one group. */
    QStringList getAvailableTypesOfGroup (AbstractPlugin::Group) const;

    /*! Return all available group names as QStringList*/
    QStringList getAvailableGroups () const {
        return groupMap.values();
    }

    /*! Return the group for a specific type. */
    AbstractPlugin::Group getGroupFromType (QString type) const;

    /*! Return the name of the group */
    QString getGroupName (AbstractPlugin::Group group) const;

    /*! Return the Group from parsing a string */
    AbstractPlugin::Group getGroupFromString (QString group) const {
        return groupMap.key(group,AbstractPlugin::GroupUnspecified);
    }

    /*! delete the given plugin. All references to this plugin become invalid. */
    bool remove (AbstractPlugin*);
    bool remove (const QString &name); /*!< \overload */
    bool remove (int id); /*!< \overload */

    friend class PluginThread;

signals:
    void pluginAdded (AbstractPlugin *); /*!< signaled when a plugin has been added */
    void pluginRemoved (AbstractPlugin *);  /*!< signaled when a plugin has been removed */
    void pluginNameChanged (AbstractPlugin *, QString);  /*!< signaled when a plugin name has been changed */

private:
    PluginManager();

    int getNextId ();

    /*! Here the names of the plugin groups are assigned to AbstractPlugin::Group enums
     *  and saved in groupMap
     *
     * \sa groupMap
     */
    void createPluginGroups();

    ModuleManager* mmgr;

    static PluginManager *inst;

    QList<AbstractPlugin*>* items;
    QList<PluginConnector*>* roots;
    QMap<QString, PluginTypeDesc> registry;
    QMap<AbstractPlugin::Group,QString> groupMap;

private: //no copying
	PluginManager(const PluginManager &);
	PluginManager &operator= (const PluginManager &);
};

/*! Convenience class to facilitate plugin registration.
 *  This class registers a plugin when it is constructed. For automatic registration of a plugin, add a line like the
 *  following to the plugin's cpp file:
 *  \code static PluginRegistrar reg ("MyPlugin", MyPlugin::create, AbstractPlugin::theGroup); \endcode
 *
 *  For plugins with attributes, the situation is a little more complicated as QMap does not allow in-place addition of elements to temporaries.
 *  In that case, add a static method to your plugin class that returns the attribute map:
 *  \code static PluginRegistrar reg ("MyPlugin", MyPlugin::create, AbstractPlugin::theGroup, MyPlugin::myPluginAttrs ()); \endcode
 */
class PluginRegistrar {
public:
    PluginRegistrar (const QString &type, PluginManager::PluginFactory fac, const AbstractPlugin::Group group = AbstractPlugin::GroupUnspecified, const AbstractPlugin::AttributeMap & attrs = AbstractPlugin::AttributeMap ()) {
        PluginManager::ref ().registerPluginType (type, fac, group, attrs);
    }

private: // no copy, no dynamic alloc
    PluginRegistrar (const PluginRegistrar &);
    PluginRegistrar &operator=(const PluginRegistrar &);
    void *operator new (size_t);
};

#endif // PLUGINMANAGER_H
