#include "pluginconnectorthreadbuffered.h"

#include <cassert>

PluginConnectorThreadBuffered::PluginConnectorThreadBuffered(AbstractPlugin* _plugin, QString _name,
                                                             int _chunkSize, int _bufferSize, int _id)
: PluginConnector(_plugin,ScopeCommon::out,_name,PluginConnector::VectorUint32)
, buffer(_bufferSize,_chunkSize,_id, QVariant ())
, data()
, valid(false)
{

}

void PluginConnectorThreadBuffered::reset()
{
    valid = false;
    data.clear ();
    buffer.reset();
}

void PluginConnectorThreadBuffered::setData(QVariant _data)
{
    assert(getType() == ScopeCommon::out);
    buffer.write(&_data,1);
}

QVariant PluginConnectorThreadBuffered::getData()
{
    if(!valid)
    {
        std::vector< QVariant > tmp(1, QVariant ());
        buffer.read(tmp,1);
        data = tmp.front();
        valid = true;
        return data;
    }
    else
    {
        return data;
    }
}

QVariant PluginConnectorThreadBuffered::getDataDummy()
{
    getData ();
    return QVariant ();
}

int PluginConnectorThreadBuffered::dataAvailable()
{
    //std::cout << getName() << "PluginConnectorThreadBuffered Data available: " << buffer.available() << std::endl;
    return buffer.available();
}

bool PluginConnectorThreadBuffered::useData()
{
    //std::cout << getName() << ": Trying to delete Data" << std::endl;
    if(valid)
    {
        valid = false;
        if(!data.isNull())
        {
            //std::cout << getName() << ": Deleting Data" << std::endl;
            data.clear ();
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}
