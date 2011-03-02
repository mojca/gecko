#ifndef PLUGINCONNECTORTHREADBUFFERED_H
#define PLUGINCONNECTORTHREADBUFFERED_H

#include "pluginconnector.h"
#include "threadbuffer.h"

class PluginConnectorThreadBuffered : public virtual PluginConnector
{

public:
    PluginConnectorThreadBuffered(AbstractPlugin* _plugin, QString _name,
                                  int chunkSize, int bufferSize, int id);

    virtual void setData(std::vector<uint32_t>*);
    virtual void setData(const void *);
    virtual const void* getData();
    virtual const void* getDataDummy();
    virtual const ThreadBuffer<std::vector<uint32_t>*>* getBuffer() { return &buffer; }
    virtual int dataAvailable();
    virtual int elementsFree () const { return buffer.free (); }
    virtual bool useData();
    virtual void reset();

private:
    ThreadBuffer<std::vector<uint32_t>*> buffer;
    std::vector<uint32_t>* data;
    bool valid;
};

#endif // PLUGINCONNECTORTHREADBUFFERED_H
