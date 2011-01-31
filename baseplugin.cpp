#include "baseplugin.h"
#include "pluginmanager.h"

#include <stdint.h>
#include <vector>
#include <typeinfo>
#include <ctime>

#include <cstdio>
#include <QSettings>
#include <QListWidget>
#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QMenu>

BasePlugin::BasePlugin(int _id, QString _name, QWidget* _parent)
        : QWidget(_parent), name(_name), id(_id)
{
    inputs  = new QList<PluginConnector*>;
    outputs = new QList<PluginConnector*>;

    createUI();
    updateDisplayedConnections ();
    //std::cout << "Instantiated Base Plugin" << std::endl;
}

BasePlugin::~BasePlugin()
{
    while (!inputs->empty()) {
        delete inputs->first ();
        inputs->removeFirst ();
    }

    while (!outputs->empty()) {
        delete outputs->first ();
        outputs->removeFirst ();
    }

    delete inputs;
    delete outputs;
}

AbstractPlugin::Group BasePlugin::getPluginGroup () {
    return PluginManager::ref().getGroupFromType(typename_);
}

void BasePlugin::addConnector(PluginConnector* newConnector)
{
    if(newConnector->getType() == ScopeCommon::in)
    {
        inputs->append(newConnector);
        nofInputs = inputs->size();
    }
    else
    {
        outputs->append(newConnector);
        nofOutputs = outputs->size();
    }
    updateDisplayedConnections ();
}

void BasePlugin::createUI()
{
    QGridLayout* l = new QGridLayout;
    QGridLayout* boxL = new QGridLayout;

    QGroupBox* box = new QGroupBox;
    box->setTitle(name);

    boxL->addWidget(createInbox(),0,0,1,1);
    boxL->addWidget(createOutbox(),0,1,1,1);
    boxL->addWidget(createSetbox(),1,0,1,2);
    boxL->setRowStretch(1,5);
    boxL->addWidget(new QWidget,0,2,1,2);
    boxL->setRowStretch(2,5);

    box->setLayout(boxL);

    inputList->setContextMenuPolicy (Qt::CustomContextMenu);
    connect (inputList, SIGNAL (customContextMenuRequested(QPoint)), SLOT (displayInputConnectionPopup (const QPoint &)));
    outputList->setContextMenuPolicy (Qt::CustomContextMenu);
    connect (outputList, SIGNAL (customContextMenuRequested(QPoint)), SLOT (displayOutputConnectionPopup (const QPoint &)));

    connect (inputList, SIGNAL (itemDoubleClicked(QListWidgetItem*)), SLOT (itemDblClicked(QListWidgetItem*)));
    connect (outputList, SIGNAL (itemDoubleClicked(QListWidgetItem*)), SLOT (itemDblClicked(QListWidgetItem*)));

    l->addWidget(box,0,0,1,1);
    this->setLayout(l);
}

QWidget* BasePlugin::createInbox()
{
    QGroupBox* box = new QGroupBox("Inputs:");
    QGridLayout* l = new QGridLayout;

    inputList = new QListWidget();
    inputList->setMaximumHeight(100);
    l->addWidget(inputList);
    box->setLayout(l);

    return box;
}

QWidget* BasePlugin::createOutbox()
{
    QGroupBox* box = new QGroupBox("Outputs:");
    QGridLayout* l = new QGridLayout;

    outputList = new QListWidget();
    outputList->setMaximumHeight(100);
    l->addWidget(outputList);
    box->setLayout(l);

    return box;
}

QWidget* BasePlugin::createSetbox()
{
    QGroupBox* box = new QGroupBox("Settings:");
    settingsLayout = new QGridLayout;

    box->setLayout(settingsLayout);

    return box;
}

int BasePlugin::updateConnList (ConnectorList *lst, QListWidget *w) {
    int cnt = 0;
    w->clear ();
    w->setEnabled (true);

    if (lst->empty ())
    {
        w->setEnabled (false);
        return cnt;
    }

    foreach(PluginConnector* pc, (*lst))
    {
        if(pc->hasOtherSide())
        {
            QString itemText;
            itemText = pc->getName () + "->" + pc->getConnectedPluginName() + ": " + pc->getOthersideName();
            QListWidgetItem *it = new QListWidgetItem (itemText, w);
            it->setData(Qt::UserRole, QVariant::fromValue (pc));
            w->addItem(it);
            cnt++;
        }
        else
        {
            QListWidgetItem *it = new QListWidgetItem (pc->getName () + " " + tr("<not connected>"), w);
            it->setData(Qt::UserRole, QVariant::fromValue (pc));
            w->addItem(it);
        }
    }
    return cnt;
}

void BasePlugin::updateDisplayedConnections()
{
    //std::cout << name.toStdString() << " Updating connections...";
    if (inputs)
        nofConnectedInputs = updateConnList (inputs, inputList);
    if (outputs)
        nofConnectedOutputs = updateConnList (outputs, outputList);
    //std::cout << "done" << std::endl;
}

void BasePlugin::itemDblClicked(QListWidgetItem *item) {
    PluginConnector *pc = item->data (Qt::UserRole).value<PluginConnector*> ();
    if (pc && pc->hasOtherSide ()) {
        // check whether the connector belongs to a daq module, demux plugins have no inputs
        if (pc->getConnectedPlugin ()->getInputs ()->size () == 0)
            return;

        emit jumpToPluginRequested (pc->getConnectedPlugin ());
    }
}

void BasePlugin::displayInputConnectionPopup (const QPoint &p) {
    const QList<BasePlugin*>* plugins = PluginManager::ref().list ();

    if (inputList->itemAt (p))
        inputList->setCurrentItem (inputList->itemAt (p));
    else if (!inputList->currentItem())
        return;

    PluginConnector *thisSide = inputList->currentItem ()->data(Qt::UserRole).value<PluginConnector*> ();

    QMenu popup;
    popup.addAction (tr("<none>"))->setData (QVariant::fromValue (static_cast<PluginConnector*> (NULL)));
    foreach (BaseDAqModule *m, (*ModuleManager::ref().listDaqModules())) {
        createPluginSubmenu (&popup, thisSide->getDataType(), m->getOutputPlugin(), &BasePlugin::getOutputs);
    }

    popup.addSeparator ();
    foreach (BasePlugin *p, (*plugins)) {
        if (p != this)
            createPluginSubmenu (&popup, thisSide->getDataType(), p, &BasePlugin::getOutputs);
    }

    QAction *act = popup.exec (inputList->mapToGlobal(p));
    if (act) {
        PluginConnector *newOtherSide = act->data ().value<PluginConnector*> ();
        thisSide->disconnect ();
        if (newOtherSide)
            thisSide->connectTo (newOtherSide);
        updateDisplayedConnections ();
    }
}

void BasePlugin::displayOutputConnectionPopup (const QPoint &p) {
    const QList<BasePlugin*>* plugins = PluginManager::ref().list ();

    if (outputList->itemAt (p))
        outputList->setCurrentItem (outputList->itemAt (p));
    else if (!outputList->currentItem())
        return;

    PluginConnector *thisSide = outputList->currentItem ()->data(Qt::UserRole).value<PluginConnector*> ();

    QMenu popup;
    popup.addAction (tr("<none>"))->setData (QVariant::fromValue (static_cast<PluginConnector*> (NULL)));
    foreach (BasePlugin *p, (*plugins)) {
        if (p != this)
            createPluginSubmenu (&popup, thisSide->getDataType(), p, &BasePlugin::getInputs);
    }

    QAction *act = popup.exec (outputList->mapToGlobal(p));
    if (act) {
        PluginConnector *newOtherSide = act->data ().value<PluginConnector*> ();
        thisSide->disconnect ();
        if (newOtherSide)
            thisSide->connectTo (newOtherSide);
        updateDisplayedConnections ();
    }
}

void BasePlugin::createPluginSubmenu (QMenu *popup, PluginConnector::DataType dt, BasePlugin *p, ConnectorList *(BasePlugin::*type) ()) {
    if ((p->*type) ()->empty ())
        return;

    ConnectorList conns (*(p->*type) ());
    for (ConnectorList::iterator i = conns.begin (); i != conns.end ();)
        if ((*i)->getDataType () != dt)
            i = conns.erase (i);
        else
            ++i;

    if (conns.empty ())
        return;

    QMenu *sub = popup->addMenu (p->getName());
    foreach (PluginConnector *c, conns) {
        sub->addAction (c->getName () + (c->hasOtherSide () ? "*" : ""))->setData (QVariant::fromValue(c));
    }
}

void BasePlugin::process()
{
    int cnt = 0;
    foreach(PluginConnector * in, (*inputs))
    {
        if(in->dataAvailable() > 0) cnt++;
        //std::cout << in->getPlugin()->getName().toStdString() << ": " << in->getName() << "::process: dataAvailable " << in->dataAvailable() << std::endl;
    }
    if(cnt == nofConnectedInputs)
    {
        //clock_t start = clock();
        userProcess();
        //clock_t elapsed = clock() - start;

        //std::cout << this->name.toStdString() << " time: " << std::dec << (int)(elapsed/10000) << std::endl;

        foreach(PluginConnector * in, (*inputs))
        {
            bool ok = in->useData();
            Q_UNUSED(ok);
            //if(!ok) printf("dequeueing 1 element from %s, input %s failed\n",getName().toStdString().c_str(),in->getName().c_str());
        }
    }
    else
    {
        //printf("%s: No process, because only %d buffers of %d were ready.\n",this->getName().toStdString().c_str(),cnt,nofConnectedInputs);
        return;
    }
}

void BasePlugin::setConfigEnabled (bool enabled) {
    inputList->setEnabled (nofInputs && enabled);
    outputList->setEnabled (nofOutputs && enabled);
}
