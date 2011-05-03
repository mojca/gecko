#include "pluginmanager.h"

#include "abstractmodule.h"

struct PluginTypeDesc {
    PluginManager::PluginFactory fac;
    AbstractPlugin::Group group;
    BasePlugin::AttributeMap attrs;

    bool operator == (const PluginTypeDesc & b) const {
        return (this->fac == b.fac && this->group == b.group && this->attrs == b.attrs);
    }
};

PluginManager *PluginManager::inst = NULL;

PluginManager *PluginManager::ptr () {
	if (inst == NULL)
		inst = new PluginManager ();
	return inst;
}

PluginManager &PluginManager::ref () {
	return *ptr ();
}

PluginManager::PluginManager()
{
    mmgr = ModuleManager::ptr ();
    items = new QList<AbstractPlugin*>;
    roots = new QList<PluginConnector*>;

    createPluginGroups();
}

PluginManager::~PluginManager()
{
    clear();
    roots->clear();
    delete items;
    delete roots;
}

void PluginManager::clear()
{
    while (!items->empty ())
        remove (items->first ());
}

bool PluginManager::remove (AbstractPlugin* rmItem)
{
    QList<AbstractPlugin*>::iterator it(items->begin());
    while(it != items->end())
    {
        if((*it) == rmItem)
        {
            items->erase(it);
            emit pluginRemoved (rmItem);
            rmItem->deleteLater ();
            return true;
        }
        it++;
    }
    return false;
}

bool PluginManager::remove (int id)
{
    AbstractPlugin* am;
    if((am = get (id)) != NULL)
    {
        remove(am);
        return true;
    }
    return false;
}

bool PluginManager::remove (const QString &name)
{
    AbstractPlugin* am;
    if((am = get (name)) != NULL)
    {
        remove(am);
        return true;
    }
    return false;
}


AbstractPlugin* PluginManager::get (int id)
{
    QList<AbstractPlugin*>::iterator it(items->begin());
    while(it != items->end())
    {
        AbstractPlugin* bm = (*it);
        if(bm->getId() == id)
        {
            return *it;
        }
        it++;
    }
    return NULL;
}


AbstractPlugin* PluginManager::get (const QString &_name)
{
    QList<AbstractPlugin*>::iterator it(items->begin());
    while(it != items->end())
    {
        if((*it)->getName() == _name)
        {
            return *it;
        }
        it++;
    }
    return NULL;
}

void PluginManager::registerPluginType (const QString &type, PluginFactory fac, const AbstractPlugin::Group group, const AbstractPlugin::AttributeMap & attrs) {
    if (registry.contains (type)) {
        std::cout << "Double registration of plugin type " << type.toStdString () << "! Ignoring." << std::endl;
        return;
    }

    PluginTypeDesc desc = {fac, group, attrs};
    registry.insert (type, desc);
}

AbstractPlugin *PluginManager::create(const QString &type, const QString &name, const AbstractPlugin::Attributes &attrs) {
    if (get (name)) // name already exists
        return NULL;

    if (registry.contains (type)) {
        AbstractPlugin *p = (*registry.value (type).fac) (getNextId (), name, attrs);
        p->setTypeName (type);
        items->push_back (p);
        emit pluginAdded (p);
        return p;
    } else {
        return NULL;
    }
}

QStringList PluginManager::getAvailableTypes () const {
    return registry.keys ();
}

QStringList PluginManager::getAvailableTypesOfGroup (AbstractPlugin::Group group) const {
    QStringList ret;

    foreach(PluginTypeDesc typeDesc, registry.values()) {
        if(typeDesc.group == group && typeDesc.fac != 0) ret.append(registry.key(typeDesc));
    }
    return ret;
}

AbstractPlugin::Group PluginManager::getGroupFromType (QString type) const {
    return registry.value(type).group;
}

QString PluginManager::getGroupName (AbstractPlugin::Group group) const {
    foreach(PluginTypeDesc typeDesc, registry.values()) {
        if(typeDesc.group == group) return registry.key(typeDesc);
    }

    return QString("Unknown");
}

BasePlugin::AttributeMap PluginManager::getAttributeMap (const QString &type) {
    if (registry.contains (type))
        return registry.value (type).attrs;
    else
        return BasePlugin::AttributeMap ();
}

int PluginManager::getNextId () {
    int newId = 0;
    bool found = false;
    while(!found)
    {
        if(get (newId) == NULL)
        {
            found = true;
        }
        else
        {
            newId++;
        }
    }

    return newId;
}

void PluginManager::createPluginGroups() {
    groupMap.insert(AbstractPlugin::GroupCache ,"Cache"      );
    groupMap.insert(AbstractPlugin::GroupDemux ,"Demux"      );
    groupMap.insert(AbstractPlugin::GroupDSP   ,"DSP"        );
    groupMap.insert(AbstractPlugin::GroupOutput,"Output"     );
    groupMap.insert(AbstractPlugin::GroupPack  ,"Packing"    );
    groupMap.insert(AbstractPlugin::GroupPlot  ,"Plot"       );
    groupMap.insert(AbstractPlugin::GroupAux   ,"Aux");
    groupMap.insert(AbstractPlugin::GroupUnspecified  ,"Misc");
}

void PluginManager::applySettings(QSettings* newSettings)
{
    QList<AbstractPlugin*>::iterator it(items->begin());
    while(it != items->end())
    {
        AbstractPlugin* bm = (*it);
        bm->applySettings(newSettings);
        it++;
    }
}

void PluginManager::saveSettings(QSettings* newSettings)
{
    QList<AbstractPlugin*>::iterator it(items->begin());
    while(it != items->end())
    {
        AbstractPlugin* bm = (*it);
        bm->saveSettings(newSettings);
        it++;
    }
}

void PluginManager::setPluginName (AbstractPlugin *p, const QString &name) {
    if (p->getName () == name || get (name)) // either no change or name already exists
        return;

    QString oldname = p->getName ();
    p->setName (name);
    emit pluginNameChanged (p, oldname);
}
