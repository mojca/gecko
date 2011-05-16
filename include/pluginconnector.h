#ifndef PLUGINCONNECTOR_H
#define PLUGINCONNECTOR_H

#include <stdint.h>

#include <QMetaType>
#include <QVariant>
#include <QVector>

class AbstractPlugin;

namespace ScopeCommon
{
    enum ConnectorType{in,out}; /*!< The direction of data flow for a PluginConnector */
}

/*! A source or sink for data transferred between plugins.
 *  All data handling is performed by output connectors. Input connectors only
 *  pass commands to the output they are connected to.
 */
class PluginConnector
{
public:
    /*! The type of data the connector accepts */
    enum DataType { Uint32, Double, VectorUint32, VectorDouble };

public:
    PluginConnector(AbstractPlugin* _plugin, ScopeCommon::ConnectorType _type, QString _name, DataType _dt);
    virtual ~PluginConnector();

    /*! Returns the type of data the connector sends or accepts. */
    DataType getDataType () const { return dtype; }
    /*! Returns whether this connector is an input or an output. */
    ScopeCommon::ConnectorType getType() const { return type; }
    /*! Returns the plugin which this connector belongs to. */
    AbstractPlugin* getPlugin() const { return plugin; }
    /*! Returns the connector name. */
    QString getName() const { return name; }
    /*! Returns the name of the plugin which this connector is connected to. */
    QString getConnectedPluginName() const;
    /*! Returns the plugin which this connector is connected to. */
    AbstractPlugin* getConnectedPlugin() const;
    /*! Returns the name of the connector which this connector is connected to. */
    QString getOthersideName() const;
    /*! Returns whether the connector is connected or disconnected. */
    bool hasOtherSide() const;

    /*! Connects the connector to the specified connector. Only one input may be connected to an output and DataTypes must match.
     *  \throws std::invalid_argument when trying to connect two inputs or outputs or if the DataTypes do not match
     */
    void connectTo(PluginConnector* _otherSide);

    /*! Disconnects the connector. */
    void disconnect();

    /*! Push data into the connector. The data becomes visible on the input connector connected to this connector.
     *  \note This function may only be called for output connectors
     */
    virtual void setData(QVariant) = 0;

    /*! Get data data from the connector.
     *  \note This function may only be called for input connectors
     */
    virtual QVariant getData() = 0;

    /*! Tell the connector that the data retrieved by getData has been used and may now be discarded.
     *  \note This function may only be called for input connectors
     */
    virtual bool useData() = 0;

    /*! Check whether there is data available on this connector. */
    virtual int dataAvailable() = 0;

    /*! Release all data queued inside the connector. */
    virtual void reset() = 0;

protected:
    /*! Returns the connector connected to this one. */
    PluginConnector* getOtherSide() { return otherSide; }

private:
    AbstractPlugin* plugin;
    ScopeCommon::ConnectorType type;
    PluginConnector* otherSide;
    QString name;
    DataType dtype;
};

Q_DECLARE_METATYPE (PluginConnector*);
Q_DECLARE_METATYPE (QVector<uint32_t>);
Q_DECLARE_METATYPE (QVector<double>);

/*! Traits class to convert type names to members of the PluginConnector::DataType enum. */
template<typename T>
class TypeToDataType {
};
template<>
class TypeToDataType<uint32_t> {
public:
    static const PluginConnector::DataType data_type = PluginConnector::Uint32;
};
template<>
class TypeToDataType<double> {
public:
    static const PluginConnector::DataType data_type = PluginConnector::Double;
};
template<>
class TypeToDataType< QVector<uint32_t> > {
public:
    static const PluginConnector::DataType data_type = PluginConnector::VectorUint32;
};
template<>
class TypeToDataType< QVector<double> > {
public:
    static const PluginConnector::DataType data_type = PluginConnector::VectorDouble;
};

#endif
