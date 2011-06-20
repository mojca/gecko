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

#ifndef DSPADCPLUGIN_H
#define DSPADCPLUGIN_H

#include <algorithm>
#include <vector>
#include <cstdio>
#include <stdint.h>
#include <samdsp.h>
#include "baseplugin.h"

class BasePlugin;

struct DspAdcPluginConfig
{
};

class DspAdcPlugin : public virtual BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    DspAdcPluginConfig conf;

public:
    DspAdcPlugin(int _id, QString _name);
    static AbstractPlugin *create (int id, const QString &name, const Attributes &attrs) {
        Q_UNUSED (attrs);
        return new DspAdcPlugin (id, name);
    }
    virtual void userProcess();

    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:

};

#endif // DSPADCPLUGIN_H
