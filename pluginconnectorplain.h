#ifndef PLUGINCONNECTORPLAIN_H
#define PLUGINCONNECTORPLAIN_H

#include "pluginconnector.h"

class PluginConnectorPlain : public PluginConnector {
public:
    PluginConnectorPlain (AbstractPlugin* _plugin, ScopeCommon::ConnectorType _type, QString _name, DataType _dt)
    : PluginConnector (_plugin, _type, _name, _dt)
    , data_ (NULL)
    {
    }

    void setData (const void* d) {
        assert (getType () == ScopeCommon::out);
        data_ = d;
    }

    const void *getData () {
        if (getType() == ScopeCommon::in)
            return hasOtherSide () ? getOtherSide ()->getData () : NULL;
        else
            return data_;
    }

    const void *getDataDummy() {
        return NULL;
    }

    bool useData () {
        if (getType () == ScopeCommon::in && hasOtherSide())
            return getOtherSide ()->useData ();
        else {
            bool ret = (data != NULL);
            data_ = NULL;
            return ret;
        }
    }

    int dataAvailable () {
        if (getType () == ScopeCommon::in)
            return hasOtherSide () ? getOtherSide ()->dataAvailable() : 0;
        else
            return data_ != NULL ? 1 : 0;
    }

    void reset () {
        data_ = NULL;
    }

private:
    const void* data_;
};

#endif // PLUGINCONNECTORPLAIN_H
