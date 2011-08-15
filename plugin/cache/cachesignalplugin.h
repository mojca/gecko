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
