#ifndef PLUGINCONNECTORQUEUED_H
#define PLUGINCONNECTORQUEUED_H

#include "pluginconnector.h"
#include <QQueue>

#include <cassert>

class BasePlugin;

template<typename T>
class PluginConnectorQueued : public PluginConnector
{
public:
    PluginConnectorQueued(BasePlugin* _plugin, ScopeCommon::ConnectorType _type, QString _name)
        : PluginConnector (_plugin, _type, _name, TypeToDataType<T>::data_type)
    {
    }

    ~PluginConnectorQueued () {
        q.clear();
    }

    void setData (const T* _data) {
        assert(getType() == ScopeCommon::out);
        q.enqueue(_data);
    }

    void setData(const void* _data)
    {
        setData (reinterpret_cast<const T*> (_data));
    }

    // may only be called from input connectors
    const void* getData()
    {
        if(getType() == ScopeCommon::in)
        {
            if(hasOtherSide()) return getOtherSide()->getData();
            else return NULL;
        }
        else
        {
            if(!q.empty()) return q.head();
            else return NULL;
        }
    }

    const void* getDataDummy()
    {
        return NULL;
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
        std::cout << getName().toStdString() << "PluginConnector reset " << std::endl;
        q.clear();
    }

protected:
    QQueue<const T*> q;
};

typedef PluginConnectorQueued< std::vector<uint32_t> > PluginConnectorQVUint;
typedef PluginConnectorQueued< std::vector<double> > PluginConnectorQVDouble;

#endif // PLUGINCONNECTORQUEUED_H
