#ifndef CACHESIGNALPLUGIN_H
#define CACHESIGNALPLUGIN_H

#include <QFileInfo>
#include "basecacheplugin.h"

class BaseCachePlugin;
class BasePlugin;

class CacheSignalPlugin : public virtual BaseCachePlugin
{

protected:
    std::vector<double> signal;
    std::vector<double> curData;

public:
    CacheSignalPlugin(int _id, QString _name);
    static BasePlugin *create (int _id, const QString &_name, const Attributes &_attrs) {
        Q_UNUSED (_attrs);
        BaseCachePlugin* bcp = new CacheSignalPlugin (_id, _name);
        return dynamic_cast<BasePlugin*>(bcp);
    }
    
    virtual void userProcess();
};

#endif // CACHESIGNALPLUGIN_H
