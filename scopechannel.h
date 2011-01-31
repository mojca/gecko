#ifndef SCOPECHANNEL_H
#define SCOPECHANNEL_H

#include "basedaqmodule.h"
#include <QString>

namespace ScopeCommon
{
    enum ChannelType{eventBuffer,trace,logic,trigger,interrupt};
}

class BaseDAqModule;

class ScopeChannel
{
    BaseDAqModule* module;
    QString name;
    ScopeCommon::ChannelType type;
    unsigned int maxLen;
    unsigned int dataWidth;
    bool enabled;

public:
    ScopeChannel(BaseDAqModule* _module, QString _name, ScopeCommon::ChannelType _type, unsigned int _maxLen, unsigned int _dataWidth);
    ScopeCommon::ChannelType getType() { return type; }
    QString getName() { return name; }
    BaseDAqModule* getModule() { return module; }
    bool isEnabled() { return enabled; }
    void setEnabled(bool _enabled) { enabled = _enabled; }
};

#endif // SCOPECHANNEL_H
