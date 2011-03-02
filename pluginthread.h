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
#include <vector>


#include "pluginmanager.h"
#include "modulemanager.h"
#include "pluginconnectorthreadbuffered.h"

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
 *  The thread then walks through each layer calling the BasePlugin::process function for each plugin
 *  if all the plugin's input connectors contain data. If not, the plugin is skipped. The process functions
 *  for plugins in the same layer might be called in parallel.
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
    int nofAcqsWaiting;
    QSet<int> actChans;
    QWaitCondition cond;

    QList<PluginConnectorThreadBuffered*> bufferList;
    QList<PluginConnector*> unconnectedList;

    QMap<AbstractPlugin*, int> processList;
    int maxDepth;

    void addBuffers();
    void createProcessList();
    void addChildrenToProcessList();
    void execProcessList();
};

#endif // PLUGINTHREAD_H
