#ifndef CACHESIGNALPLUGIN_H
#define CACHESIGNALPLUGIN_H

#include <QFileInfo>
#include "basecacheplugin.h"

class BaseCachePlugin;
class BasePlugin;

class CacheSignalPlugin : public virtual BaseCachePlugin
{

private:
    QVector<double> signal;

public:
    CacheSignalPlugin(int _id, QString _name);
    static AbstractPlugin *create (int _id, const QString &_name, const Attributes &_attrs) {
        Q_UNUSED (_attrs);
        AbstractPlugin* bcp = new CacheSignalPlugin (_id, _name);
        return bcp;
    }
    
    virtual void userProcess();
    virtual void runStartingEvent();
};

#endif // CACHESIGNALPLUGIN_H
