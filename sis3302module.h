#ifndef SIS3302MODULE_H
#define SIS3302MODULE_H

#include <QObject>
#include <QMap>
#include <QTime>
#include <cstdio>
#include <ctime>
#include "sys/time.h"
#include <cstring>
#include <cerrno>
#include <vector>
#include <inttypes.h>

#include "basedaqmodule.h"
#include "sis3302ui.h"
#include "sis3302.h"
#include "baseplugin.h"
#include "demuxsis3350plugin.h"
#include "pluginmanager.h"

using namespace std;

class Sis3302config
{
public:

};

class Sis3302Module : public virtual BaseDAqModule
{
public:
    Sis3302Module(int _id, QString _name);
    ~Sis3302Module();

    // Configuration class
    Sis3302config conf;

    // Factory method
    static BaseModule *create (int id, const QString &name) {
        return new Sis3302Module (id, name);
    }

    // Mandatory to implement
    virtual void saveSettings(QSettings*) {};
    virtual void applySettings(QSettings*) {};
    void setChannels() {};
    ThreadBuffer<uint32_t> *getBuffer () { return buffer; }
    virtual int acquire() {return 0;}
    virtual bool dataReady() {return false;}
    virtual int configure() {return 0;}
    virtual int reset() {return 0;}
    virtual uint32_t getBaseAddress () const {return 0xffff0000;}
    virtual void setBaseAddress (uint32_t baddr) {};
public slots:
    virtual void prepareForNextAcquisition() {};

private:
    ThreadBuffer<uint32_t> *buffer;
    virtual void createOutputPlugin() {};
};

#endif // SIS3302MODULE_H
