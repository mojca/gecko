#include "pluginconnector.h"
#include "baseplugin.h"

#include <stdexcept>
#include <iostream>

PluginConnector::PluginConnector(BasePlugin* _plugin, ScopeCommon::ConnectorType _type, QString _name, DataType _dt)
        : plugin(_plugin), type(_type), otherSide(NULL), name(_name), dtype (_dt)
{

}

PluginConnector::~PluginConnector()
{
    disconnect ();
}

void PluginConnector::connectTo(PluginConnector* _otherSide)
{
    // precondition check
    if (type == _otherSide->type)
        throw std::invalid_argument (std::string ("trying to connect two") + (type == ScopeCommon::in ? "ins" : "outs"));
    if (dtype != _otherSide->dtype)
        throw std::invalid_argument (std::string ("connectors have different data types"));

    if (hasOtherSide())
        return;

    otherSide = _otherSide;

    // Reverse connection
    otherSide->connectTo(this);

    QString from;

    if(plugin != NULL)
        from = plugin->getName();
    else
        from = "root";

    std::cout << "Connected " << from.toStdString()
              << " to " << getConnectedPluginName().toStdString() << std::endl;

    this->getPlugin()->updateDisplayedConnections();
}

void PluginConnector::disconnect()
{
    if(hasOtherSide())
    {
        std::cout << "Disconnecting " << getName().toStdString()
                  << " from " << getConnectedPluginName().toStdString() << std::endl;

        PluginConnector* tmp = otherSide;
        otherSide = NULL;
        tmp->disconnect ();

        this->getPlugin()->updateDisplayedConnections();
    }
}

bool PluginConnector::hasOtherSide() const
{
    return (otherSide != NULL);
}

QString PluginConnector::getConnectedPluginName() const
{
    if(hasOtherSide()) return otherSide->getPlugin()->getName();
    else return "";
}

BasePlugin* PluginConnector::getConnectedPlugin() const
{
    if(hasOtherSide()) return otherSide->getPlugin();
    else return NULL;
}

QString PluginConnector::getOthersideName() const
{
    if(hasOtherSide()) return otherSide->getName();
    else return "";
}
