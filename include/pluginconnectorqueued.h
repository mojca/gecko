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

#ifndef PLUGINCONNECTORQUEUED_H
#define PLUGINCONNECTORQUEUED_H

#include "pluginconnector.h"
#include <QQueue>
#include <iostream>

#include <cassert>

class BasePlugin;

/*! A plugin connector that uses QQueue to queue outgoing data.
 *
 */
template<typename T>
class PluginConnectorQueued : public PluginConnector
{
public:
    PluginConnectorQueued(AbstractPlugin* _plugin, ScopeCommon::ConnectorType _type, QString _name)
        : PluginConnector (_plugin, _type, _name, TypeToDataType<T>::data_type)
    {
    }

    ~PluginConnectorQueued () {
        q.clear();
    }

    void setData (QVariant _data) {
        assert(getType() == ScopeCommon::out);
        q.enqueue(_data);
    }

    // may only be called from input connectors
    QVariant getData()
    {
        if(getType() == ScopeCommon::in)
        {
            if(hasOtherSide()) return getOtherSide()->getData();
            else return QVariant ();
        }
        else
        {
            if(!q.empty()) return q.head();
            else return QVariant ();
        }
    }

    bool useData()
    {
        if(getType() == ScopeCommon::in)
        {
            if(hasOtherSide()) return getOtherSide()->useData();
            else return false;
        }
        else
        {
            if(!q.empty())
            {
                //printf("%s dequeueing 1 element, %d remaining\n",getName().c_str(),q.size());
                q.dequeue();
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    int dataAvailable()
    {
        if(getType() == ScopeCommon::in)
        {
            if(hasOtherSide())
            {
                return getOtherSide()->dataAvailable();
            }
            else
            {
                return 0;
            }
        }
        else
        {
            //std::cout << getName() << "PluginConnector Data available: " << q.size() << std::endl;
            return q.size();
        }
    }

    void reset()
    {
        //std::cout << getName().toStdString() << "PluginConnector reset " << std::endl;
        q.clear();
    }

protected:
    QQueue< QVariant > q;
};

typedef PluginConnectorQueued< QVector<uint32_t> > PluginConnectorQVUint;
typedef PluginConnectorQueued< QVector<double> > PluginConnectorQVDouble;

#endif // PLUGINCONNECTORQUEUED_H
