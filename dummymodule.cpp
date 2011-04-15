#include "dummymodule.h"
#include "modulemanager.h"
#include "scopechannel.h"

static ModuleRegistrar registrar ("dummy", DummyModule::create);

DummyModule::DummyModule(int _id, QString _name)
        : BaseModule (_id, _name)
{
    // Test data
    data = new QByteArray("0123456789");
    triggered = false;

    setChannels();
    createBuffer ();

    // Create user interface
    setUI (new DummyUI(this));

    createOutputPlugin ();

    std::cout << "Instantiated Dummy Module" << std::endl;
}

void DummyModule::setChannels()
{
    // Setup channels
    addSlot ("Dummy Channel", PluginConnector::VectorUint32);
}

void DummyModule::setPollTriggerFlag()
{
    triggered = true;
}

bool DummyModule::dataReady()
{
    if(triggered)
    {
        triggered = false;
        return true;
    }
    return false;
}

void DummyModule::createBuffer()
{
    buffer = ModuleManager::ptr()->createBuffer(4*100, 100, getId());
}

int DummyModule::acquire(Event *)
{
    std::cout << thread()->currentThreadId() << ": data = " << QString((*data)).toStdString() << std::endl;
    uint32_t out[251];
    int ret;
    for(int i = 0; i < 251; i++)
    {
        out[i] =     (i           & 0xff)
                 | (((i+1) <<  8) & 0xff00)
                 | (((i+2) << 16) & 0xff0000)
                 | (((i+3) << 24) & 0xff000000) ;
    }
    for(int i = 0; i < 1000; i++)
    {
        //std::cout << "dummy module writing..." << std::endl;
        ret = buffer->write(&(out[0]),251);
    }
    return ret;
}

void DummyModule::saveSettings(QSettings* settings)
{
    if(settings == NULL)
    {
        std::cout << "dummy module: no settings file" << std::endl;
        return;
    }
    else
    {
        std::cout << "dummy module saving settings...";
        settings->beginGroup(getName());
        settings->setValue("dummyProperty",dummyProperty);
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void DummyModule::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
    set = "dummyProperty";   if(settings->contains(set)) dummyProperty = settings->value(set).toInt();
    settings->endGroup();
}
