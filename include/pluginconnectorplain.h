/*
Copyright 2011 Bastian Loeher, Roland Wirth

This file is part of GECKO.

GECKO is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

GECKO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
