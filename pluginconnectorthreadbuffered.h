#ifndef PLUGINCONNECTORTHREADBUFFERED_H
#define PLUGINCONNECTORTHREADBUFFERED_H

#include "pluginconnector.h"
#include "threadbuffer.h"

class PluginConnectorThreadBuffered : public virtual PluginConnector
{

public:
    PluginConnectorThreadBuffered(AbstractPlugin* _plugin, QString _name,
                                  int chunkSize, int bufferSize, int id);

    virtual void setData(QVariant);
    virtual QVariant getData();
    virtual QVariant getDataDummy();
    virtual const ThreadBuffer< QVariant >* getBuffer() { return &buffer; }
    virtual int dataAvailable();
    virtual int elementsFree () const { return buffer.free (); }
    virtual bool useData();
    virtual void reset();

private:
    ThreadBuffer< QVariant > buffer;
    QVariant data;
    bool valid;
};

#endif // PLUGINCONNECTORTHREADBUFFERED_H
