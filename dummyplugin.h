#ifndef DUMMYPLUGIN_H
#define DUMMYPLUGIN_H

#include "baseplugin.h"

class BasePlugin;

class DummyPlugin : public virtual BasePlugin
{
public:
    DummyPlugin(int _id, QString _name);

    static AbstractPlugin *create (int _id, const QString &_name, const Attributes &_attrs) {
        Q_UNUSED (_attrs);
        return new DummyPlugin (_id, _name);
    }

    virtual void createSettings(QGridLayout*);
    virtual void userProcess();

    virtual void applySettings(QSettings*) {}
    virtual void saveSettings(QSettings*) {}
};

#endif // DUMMYPLUGIN_H
