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

#ifndef PLUGINTHREAD_H
#define PLUGINTHREAD_H

#include <cstdio>

#include <QMutex>
#include <QSet>
#include <QThread>
#include <QWaitCondition>
#include <QCoreApplication>
#include <iostream>
#include <QMap>
#include <QMetaType>
#include <QAtomicInt>
#include <vector>


#include "pluginmanager.h"
#include "modulemanager.h"

/*! Thread for plugin processing.
 *  The plugin enumerates all configured plugins and sorts them into layers:
 *  Each plugin is assigned to the layer number of its highest-layer input connector, incremented by one.
 *  The output plugins reside in layer 0 as they can not depend on other plugins. A layer structure
 *  might look like this (the arrows denote data flow between modules and plugins respectively):
 *  \dot
 *  digraph {
 *    rankdir = "LR";
 *    subgraph {rank = same; Daq1; Daq2; Daq3; "Layer 0" [shape=none];}
 *    subgraph {rank = same; Plugin1; "Layer 1" [shape=none];}
 *    subgraph {rank = same; Plugin2; Plugin3; "Layer 2" [shape=none];}
 *    subgraph {rank = same; Plugin4;  "Layer 3" [shape=none];}
 *    Daq1 -> Plugin1;
 *    Daq2 -> Plugin1;
 *    Plugin1 -> Plugin2;
 *    Plugin1 -> Plugin3;
 *    Daq3 -> Plugin3;
 *    Plugin2 -> Plugin4;
 *    edge [style=invis]
 *    "Layer 0" -> "Layer 1" -> "Layer 2" -> "Layer 3";
 *  }
 *  \enddot
 *  The thread then walks through each layer calling the AbstractPlugin::process function for each plugin.
 *  The process functions for plugins in the same layer might be called in parallel.
 */
class PluginThread : public QThread
{
    Q_OBJECT


protected:
    void run();

public:
    PluginThread(PluginManager*,ModuleManager*);
    ~PluginThread();

public slots:
    void stop();
    void process();
    void acquisitionDone ();

private:
    bool abort;
    PluginManager* pmgr;
    ModuleManager* mmgr;
    QMutex mutex;
    QAtomicInt nofAcqsWaiting;
    QWaitCondition cond;

    QList<PluginConnector*> unconnectedList;

    QList< QList<AbstractPlugin*> > levelList;

    void createProcessList();
    void addChildrenToProcessList(QMap<AbstractPlugin*, int>& processList, int& maxDepth);
    void execProcessList();
};

#endif // PLUGINTHREAD_H
