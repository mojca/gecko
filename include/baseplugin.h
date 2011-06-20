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

#ifndef BASEPLUGIN_H
#define BASEPLUGIN_H

#include <QList>

#include "abstractplugin.h"

class PluginConnector;
class PluginManager;

class QSettings;
class QListWidget;
class QGridLayout;
class QListWidgetItem;
class QMenu;

/*! Base class for all plugins.
 *  Plugins perform post-processing of data collected by daq modules. They have a set of input and
 *  output terminals through which they receive and send data.
 *
 *  Processing is done event-based:
 *  For every event, the #userProcess function is called to process data from the inputs and make
 *  the processed data available on the output terminals of the plugin. The function is called directly from the PluginThread,
 *  which manages the execution of all plugins, or from a thread pool.
 *
 *  \sa PluginManager, PluginThread
 */
class BasePlugin : public AbstractPlugin
{
    Q_OBJECT

public:
    typedef QList<PluginConnector*> ConnectorList;

    BasePlugin(int _id, QString _name, QWidget* _parent = 0);
    virtual ~BasePlugin();

    /*! Return the plugin id */
    int getId() const { return id; }

    /*! Return the plugin name */
    QString getName() const { return name; }

    /*! return the plugin's type name as string */
    const QString &getTypeName () const { return typename_; }

    /*! return the group of the plugin */
    virtual AbstractPlugin::Group getPluginGroup () const;

    /*! return the list of input connectors */
    ConnectorList* getInputs() { return inputs; }
    /*! return the list of output connectors */
    ConnectorList* getOutputs() { return outputs; }

    /*! Save plugin-specific settings to the given QSettings object.
     *  Implementors should create a group with the name of the plugin
     *  and store their settings there.
     *
     *  \sa #applySettings
     */
    virtual void saveSettings(QSettings*) = 0;

    /*! Load plugin-specific settings from the given QSettings object.
     *  Implementors should read from the group with the name of the plugin.
     *
     *  \sa #saveSettings
     */
    virtual void applySettings(QSettings*) = 0;

    /*! Return a map of attribute names to attribute types.
     *  The attribute map is used to present a list of attributes to the user
     *  when creating an instance of the plugin. These attributes can then be used to create a user-defined
     *  number of input and output connectors, as these can not be created later on.
     *
     *  The values entered by the user are passed to the create method on instantiation of the plugin.
     *  They are also stored in the configuration INI file.
     *
     *  The default implementation returns an empty map.
     *
     *  \sa #getAttributes
     */
    virtual AttributeMap getAttributeMap() const { return AttributeMap (); }

    /*! Return the list of currently active atrributes.
     *  This function is currently used to store the configured attributes to the configuration INI file.
     */
    virtual Attributes getAttributes () const { return Attributes (); }

    /*! update the connections displayed in the input and output boxes. */
    void updateDisplayedConnections();

    /*! enable or disable those UI elements that should not be accessible during a run. */
    void setConfigEnabled (bool enabled);

    /*! perform actions prior to starting a run, eg. clearing statistics, resetting spectra.
     *  The default implementation does nothing.
     */
    void runStartingEvent ();

public slots:
    /*! Do processing.
     *  This function checks whether there is data available on all input connectors and then calls the userProcess function.
     *  Afterwards, it releases the data from the input connectors.
     */
    virtual void process();

    /*! the plugin's work function.
     *  Implementors should get their input data from the input connectors via PluginConnector::getData:
     *  \code
     *    // retrieve a vector of uint32 data from the first input
     *    QVector<uint32_t> pdata = inputs->at (0)->getData ().value< QVector<uint32_t> > ();
     *  \endcode
     *
     *  Output data can then be handed down to the next plugin:
     *  \code
     *    outputs->at (0)->setData (QVariant::fromValue (outData))
     *  \endcode
     *  \sa PluginConnector::setData, PluginConnector::getData
     */
    virtual void userProcess() = 0;

protected:
    ConnectorList* inputs; /*!< the plugin's inputs */
    ConnectorList* outputs; /*!< the plugin's inputs */
    QGridLayout* settingsLayout; /*!< the UI's layout */

    /*! Create the plugin-specific UI elements.
     *  Implementors should create their UI elements inside this function and add them to the given QGridLayout
     *  \note As of now, this function has to be called from the plugin constructor like this
     *    \code createSetings (settingsLayout); \endcode
     */
    virtual void createSettings(QGridLayout*) = 0;

    /*! Enable or disable configuration UI elements.
     *  Implementors may use this function to disable UI elements that should not
     *  be accessible during a run.
     */
    virtual void setUserConfigEnabled (bool enabled) { Q_UNUSED (enabled); }

    /*! Add a new connector to the connector list.
     *  Implementors should use this function to add inputs and outputs to the plugin
     *  during construction.
     *
     *  \warning Later addition or removal of connectors results in undefined behaviour.
     */
    void addConnector(PluginConnector*);

protected:
    void setName (QString newName) { name = newName; }
    void setTypeName (QString newType) {typename_ = newType; }

private slots:
    void displayInputConnectionPopup (const QPoint &);
    void displayOutputConnectionPopup (const QPoint &);
    void itemDblClicked (QListWidgetItem*);

private:
    void createUI();
    QWidget* createInbox();
    QWidget* createOutbox();
    QWidget* createSetbox();

    int updateConnList (ConnectorList *lst, QListWidget *w);
    void createPluginSubmenu (QMenu *popup, PluginConnector::DataType dt, AbstractPlugin *p, ConnectorList *(AbstractPlugin::*type) ());

private:
    QString name;

    /*! \brief The plugin's typename
     *  The plugin's typename, as registered with the plugin manager.
     *  The plugin manager fills this value when the plugin is instantiated.
     *
     * \sa PluginManager::registerPluginType, PluginRegistrar
     */
    QString typename_;
    int id;

    int nofInputs;
    int nofConnectedInputs;
    int nofConnectedOutputs;
    int nofOutputs;

    QListWidget* inputList;
    QListWidget* outputList;

    friend class PluginManager;
};

#endif // BASEPLUGIN_H
