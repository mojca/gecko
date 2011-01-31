#include "pluginmanager.h"

#include "basedaqmodule.h"

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
    items = new QList<BasePlugin*>;
    roots = new QList<PluginConnector*>;
    inBuffers = new QList<ThreadBuffer<uint32_t>*>;

    createPluginGroups();
}

PluginManager::~PluginManager()
{
    clear();
    roots->clear();
    inBuffers->clear();
    delete items;
    delete roots;
    delete inBuffers;
}

void PluginManager::clear()
{
    while (!items->empty ())
        remove (items->first ());
}

bool PluginManager::remove (BasePlugin* rmItem)
{
    QList<BasePlugin*>::iterator it(items->begin());
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
    BasePlugin* am;
    if((am = get (id)) != NULL)
    {
        remove(am);
        return true;
    }
    return false;
}

bool PluginManager::remove (const QString &name)
{
    BasePlugin* am;
    if((am = get (name)) != NULL)
    {
        remove(am);
        return true;
    }
    return false;
}


BasePlugin* PluginManager::get (int id)
{
    QList<BasePlugin*>::iterator it(items->begin());
    while(it != items->end())
    {
        BasePlugin* bm = (*it);
        if(bm->getId() == id)
        {
            return *it;
        }
        it++;
    }
    return NULL;
}


BasePlugin* PluginManager::get (const QString &_name)
{
    QList<BasePlugin*>::iterator it(items->begin());
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

void PluginManager::registerPluginType (const QString &type, PluginFactory fac, const AbstractPlugin::Group group, const BasePlugin::AttributeMap & attrs) {
    if (registry.contains (type)) {
        std::cout << "Double registration of plugin type " << type.toStdString () << "! Ignoring." << std::endl;
        return;
    }

    PluginTypeDesc desc = {fac, group, attrs};
    registry.insert (type, desc);
}

void PluginManager::registerPluginType (const QString &type, const AbstractPlugin::Group group, const BasePlugin::AttributeMap & attrs) {
    if (registry.contains (type)) {
        std::cout << "Double registration of plugin type " << type.toStdString () << "! Ignoring." << std::endl;
        return;
    }

    PluginTypeDesc desc = {0, group, attrs};
    registry.insert (type, desc);
}

BasePlugin *PluginManager::create(const QString &type, const QString &name, const AbstractPlugin::Attributes &attrs) {
    if (get (name)) // name already exists
        return NULL;

    if (registry.contains (type)) {
        BasePlugin *p = (*registry.value (type).fac) (getNextId (), name, attrs);
        p->typename_ = type;
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

AbstractPlugin::Group PluginManager::getGroupFromType (QString &type) const {
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

int PluginManager::addBuffer(ThreadBuffer<uint32_t>* _inBuffer)
{
    int threadId = _inBuffer->getModuleId();
    BasePlugin* newRootPlugin = mmgr->getDAq (threadId)->getOutputPlugin();

    std::cout << "Adding buffer from module " << threadId << std::endl;
    std::cout << "Adding plugin " << newRootPlugin->getName().toStdString() << " as root" << std::endl;

    roots->append(newRootPlugin->outputs->first());
    inBuffers->append(_inBuffer);

    return roots->size();
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
    QList<BasePlugin*>::iterator it(items->begin());
    while(it != items->end())
    {
        BasePlugin* bm = (*it);
        bm->applySettings(newSettings);
        it++;
    }
}

void PluginManager::saveSettings(QSettings* newSettings)
{
    QList<BasePlugin*>::iterator it(items->begin());
    while(it != items->end())
    {
        BasePlugin* bm = (*it);
        bm->saveSettings(newSettings);
        it++;
    }
}

void PluginManager::setPluginName (BasePlugin *p, const QString &name) {
    if (p->name == name || get (name)) // either no change or name already exists
        return;

    QString oldname = p->name;
    p->name = name;
    emit pluginNameChanged (p, oldname);
}
