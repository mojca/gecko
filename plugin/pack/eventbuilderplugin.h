#ifndef EVENTBUILDERPLUGIN_H
#define EVENTBUILDERPLUGIN_H

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

class EventBuilderPlugin : public BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);

    QVector<uint32_t> outData;
    Attributes attribs_;

public:
    EventBuilderPlugin(int _id, QString _name, const Attributes &_attrs);

    static AbstractPlugin *create (int id, const QString &name, const Attributes &attrs) {
        return new EventBuilderPlugin (id, name, attrs);
    }

    AttributeMap getAttributeMap () const;
    Attributes getAttributes () const;
    static AttributeMap getEventBuilderAttributeMap ();

    virtual void userProcess();
    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);
};

#endif // EVENTBUILDERPLUGIN_H
