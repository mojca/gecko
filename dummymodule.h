#ifndef DUMMYMODULE_H
#define DUMMYMODULE_H

#include "basemodule.h"
#include "dummyui.h"
#include "pluginmanager.h"

class DummyModule : public BaseModule
{
    Q_OBJECT

protected:
    ThreadBuffer<uint32_t> *buffer;
    QByteArray *data;
    int dummyProperty;
    bool triggered;

    void createBuffer();

public:
    DummyModule(int id, QString name = "no name");
    // Factory method
    static AbstractModule *create (int id, const QString &name) {
        return new DummyModule (id, name);
    }

    void setChannels();
    int acquire();
    void createSettings();

    virtual bool dataReady();
    virtual int reset() {return -1; }
    virtual int configure() {return -1; }

    virtual uint32_t getBaseAddress () const { return 0; }
    virtual void setBaseAddress (uint32_t) {}

    virtual void saveSettings(QSettings*);
    virtual void applySettings(QSettings*);
    virtual void prepareForNextAcquisition() {};
    virtual void createOutputPlugin() { output = PluginManager::ref().create ("dummy", "dummy output"); }

    ThreadBuffer<uint32_t> *getBuffer () { return buffer; }

public slots:
    void setPollTriggerFlag();
};

#endif // DUMMYMODULE_H
