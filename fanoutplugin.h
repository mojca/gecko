#ifndef FANOUTPLUGIN_H
#define FANOUTPLUGIN_H


#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QFile>
#include <QFileDialog>
#include <QDateTime>
#include <iostream>
#include <vector>

#include "baseplugin.h"
#include "pluginmanager.h"
#include "pluginconnectorqueued.h"

class BasePlugin;

template<class T>
class FanOutPlugin : public virtual BasePlugin
{
protected:

    virtual void createSettings(QGridLayout*);
    std::vector<T> outData;
    Attributes attribs_;

public:
    FanOutPlugin(int _id, QString _name, const Attributes &_attrs);
    static BasePlugin *create (int _id, const QString &_name, const Attributes &_attrs) {
        return new FanOutPlugin (_id, _name, _attrs);
    }
    ~FanOutPlugin();

    int getNofOutputs();
    AttributeMap getAttributeMap () const;
    Attributes getAttributes () const;
    static AttributeMap getFanoutAttributeMap ();
    virtual void userProcess();

    virtual void applySettings(QSettings*) {}
    virtual void saveSettings(QSettings*) {}

public slots:
};

template<class T>
FanOutPlugin<T>::FanOutPlugin(int _id, QString _name, const Attributes &_attrs)
            : BasePlugin(_id, _name)
            , attribs_ (_attrs)
{
    bool ok;
    int _nofOutputs = _attrs.value ("nofOutputs", QVariant (4)).toInt (&ok);
    if (!ok || _nofOutputs <= 0) {
        _nofOutputs = 4;
        std::cout << _name.toStdString () << ": nofOutputs invalid. Setting to 4" << std::endl;
    }

    //std::cout << _name.toStdString () << ": adding " << _nofOutputs << " outputs." << std::endl;

    addConnector(new PluginConnectorQueued< std::vector<T> >(this,ScopeCommon::in,"in"));

    for(int n = 0; n < _nofOutputs; n++)
    {
        addConnector(new PluginConnectorQueued< std::vector<T> >(this,ScopeCommon::out,QString("out %1").arg(n)));
        outData.push_back(NULL);
    }

    createSettings(settingsLayout);

    std::cout << "Instantiated FanOutPlugin" << std::endl;
}

template<class T>
FanOutPlugin<T>::~FanOutPlugin()
{
    outData.clear();
}

template<class T>
AbstractPlugin::AttributeMap FanOutPlugin<T>::getFanoutAttributeMap () {
    AbstractPlugin::AttributeMap attrs;
    attrs.insert ("nofOutputs", QVariant::Int);
    return attrs;
}

template<class T>
AbstractPlugin::AttributeMap FanOutPlugin<T>::getAttributeMap () const {
    return getFanoutAttributeMap ();
}

template<class T>
AbstractPlugin::Attributes FanOutPlugin<T>::getAttributes () const {
    return attribs_;
}

template<class T>
void FanOutPlugin<T>::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        QLabel* label = new QLabel(tr("Number of outputs:"));
        QLineEdit* lineEdit = new QLineEdit(tr("%1").arg(outputs->size(),1,10));
        lineEdit->setReadOnly(true);

        cl->addWidget(label,0,0,1,1);
        cl->addWidget(lineEdit,0,1,1,1);

        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

template<class T>
void FanOutPlugin<T>::userProcess()
{
    const std::vector<T>* pdata = reinterpret_cast<const std::vector<T>*>(inputs->first()->getData());

    outData.assign((*pdata).begin(),(*pdata).end());

    foreach(PluginConnector* out, (*outputs))
    {
        out->setData(&outData);
    }
}


#endif // FANOUTPLUGIN_H
