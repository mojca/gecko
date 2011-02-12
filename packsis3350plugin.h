#ifndef PACKSIS3350PLUGIN_H
#define PACKSIS3350PLUGIN_H

#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <iostream>
#include <algorithm>
#include <vector>

#include "baseplugin.h"

class BasePlugin;

class PackSis3350Plugin : public BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    std::vector<uint32_t> outData;

public:
    PackSis3350Plugin(int _id, QString _name);

    /*! return the type of the plugin */
    virtual AbstractPlugin::Group getPluginGroup () { return AbstractPlugin::GroupPack; }

    static AbstractPlugin *create (int id, const QString &name, const Attributes &attrs) {
        Q_UNUSED (attrs);
        return new PackSis3350Plugin (id, name);
    }

    virtual void userProcess();
    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
};

#endif // PACKSIS3350PLUGIN_H
