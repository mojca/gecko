#include "pluginconnectorthreadbuffered.h"

#include <cassert>

PluginConnectorThreadBuffered::PluginConnectorThreadBuffered(BasePlugin* _plugin, QString _name,
                                                             int _chunkSize, int _bufferSize, int _id)
: PluginConnector(_plugin,ScopeCommon::out,_name,PluginConnector::VectorUint32)
, buffer(_bufferSize,_chunkSize,_id)
, data(NULL)
, valid(false)
{

}

void PluginConnectorThreadBuffered::reset()
{
    valid = false;
    if(data)
    {
        data->clear();
        //delete data;
        data = NULL;
    }
    buffer.reset();
}

void PluginConnectorThreadBuffered::setData(std::vector<uint32_t>* _data)
{
    assert(getType() == ScopeCommon::out);
    buffer.write(&_data,1);
}

void PluginConnectorThreadBuffered::setData(const void * _data) {
    setData ((std::vector<uint32_t>*) _data); // FIXME!
}

const void* PluginConnectorThreadBuffered::getData()
{
    if(!valid)
    {
        std::vector<std::vector<uint32_t> *> tmp(1, static_cast<std::vector<uint32_t> *> (NULL));
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

const void* PluginConnectorThreadBuffered::getDataDummy()
{
    if(!valid)
    {
        std::vector<std::vector<uint32_t> *> tmp(1, static_cast<std::vector<uint32_t> *> (NULL));
        buffer.read(tmp,1);
        data = tmp.front ();
        valid = true;
        return NULL;
    }
    else
    {
        return NULL;
    }
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
        if(data)
        {
            //std::cout << getName() << ": Deleting Data" << std::endl;
            data->clear();
            delete data;
            data = NULL;
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
