#ifndef PLUGINCONNECTORPLAIN_H
#define PLUGINCONNECTORPLAIN_H

#include "pluginconnector.h"
#include <assert.h>

class PluginConnectorPlain : public PluginConnector {
public:
    PluginConnectorPlain (AbstractPlugin* _plugin, ScopeCommon::ConnectorType _type, QString _name, DataType _dt)
    : PluginConnector (_plugin, _type, _name, _dt)
    , data_ ()
    {
    }

    void setData (QVariant d) {
        assert (getType () == ScopeCommon::out);
        data_ = d;
    }

    QVariant getData () {
        if (getType() == ScopeCommon::in)
            return hasOtherSide () ? getOtherSide ()->getData () : QVariant ();
        else
            return data_;
    }

    QVariant getDataDummy() {
        return QVariant ();
    }

    bool useData () {
        if (getType () == ScopeCommon::in && hasOtherSide())
            return getOtherSide ()->useData ();
        else {
            bool ret = (!data_.isNull ());
            data_.clear ();
            return ret;
        }
    }

    int dataAvailable () {
        if (getType () == ScopeCommon::in)
            return hasOtherSide () ? getOtherSide ()->dataAvailable() : 0;
        else
            return !data_.isNull () ? 1 : 0;
    }

    void reset () {
        data_.clear();
    }

private:
    QVariant data_;
};

#endif // PLUGINCONNECTORPLAIN_H
