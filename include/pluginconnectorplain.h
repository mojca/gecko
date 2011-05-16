#ifndef PLUGINCONNECTORPLAIN_H
#define PLUGINCONNECTORPLAIN_H

#include "pluginconnector.h"
#include <assert.h>

/*! A simple plugin connector that performs no buffering at all.
 *  Use this connector type for input connectors because it has the smallest memory footprint.
 */
class PluginConnectorPlain : public PluginConnector {
public:
    PluginConnectorPlain (AbstractPlugin* _plugin, ScopeCommon::ConnectorType _type, QString _name, DataType _dt)
    : PluginConnector (_plugin, _type, _name, _dt)
    , data_ ()
    , valid_ (false)
    {
    }

    void setData (QVariant d) {
        assert (getType () == ScopeCommon::out);
        data_ = d;
        valid_ = !data_.isNull ();
    }

    QVariant getData () {
        if (getType() == ScopeCommon::in)
            return hasOtherSide () ? getOtherSide ()->getData () : QVariant ();
        else
            return valid_ ? data_ : QVariant ();
    }

    bool useData () {
        if (getType () == ScopeCommon::in && hasOtherSide())
            return getOtherSide ()->useData ();
        else {
            bool ret = valid_;
            valid_ = false;
            return ret;
        }
    }

    int dataAvailable () {
        if (getType () == ScopeCommon::in)
            return hasOtherSide () ? getOtherSide ()->dataAvailable() : 0;
        else
            return valid_ ? 1 : 0;
    }

    void reset () {
        data_.clear();
        valid_ = false;
    }

private:
    QVariant data_;
    bool valid_;
};

#endif // PLUGINCONNECTORPLAIN_H
