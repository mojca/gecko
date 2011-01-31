#include "scopechannel.h"

ScopeChannel::ScopeChannel(BaseDAqModule* _module, QString _name, ScopeCommon::ChannelType _type, unsigned int _maxLen, unsigned int _dataWidth)
    : module(_module), name(_name), type(_type), maxLen(_maxLen), dataWidth(_dataWidth), enabled(false)
{

}
