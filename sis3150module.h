#ifndef SIS3150MODULE_H
#define SIS3150MODULE_H

//#include "../sis3150_calls/sis3150usb_vme.h"
//#include "../sis3150_calls/sis3150usb_vme_calls.h" // Contains all function definitions to use the driver
#include <sis3150usb_vme.h>
#include <sis3150usb_vme_calls.h>

#include "baseinterface.h"
#include "sis3150ui.h"

#define MAXNUMDEV 10

class Sis3150config
{
public:
    unsigned int base_addr;
};

class QSettings;

class Sis3150Module : public BaseInterface
{
    Q_OBJECT

protected:
    int id;
    const QString& name;
    struct SIS3150USB_Device_Struct info[MAXNUMDEV];
    HANDLE m_device;
    bool deviceOpen;
    void out(QString);

public:
    Sis3150Module(int _id, QString name = "SIS 3150");
    ~Sis3150Module();

    // Factory method
    static AbstractInterface *create (int id, const QString &name) {
        return new Sis3150Module (id, name);
    }

    Sis3150config conf;

    virtual int open();
    virtual int close();
    virtual bool isOpen() const;

    virtual int setOutput1(bool){ return -1;}
    virtual int setOutput2(bool){ return -1;}

    virtual void saveSettings(QSettings*) {}
    virtual void applySettings(QSettings*) {}

    // VME access
    int readA32D32(const uint32_t addr, uint32_t* data);
    int writeA32D32(const uint32_t addr, const uint32_t data);
    int readA32D16(const uint32_t addr, uint16_t* data);
    int writeA32D16(const uint32_t addr, const uint16_t data);
    int readA32FIFO(const uint32_t addr, uint32_t* dma_buffer, uint32_t request_nof_words, uint32_t* got_nof_words);
    int readA32BLT32(const uint32_t addr, uint32_t* dma_buffer, uint32_t request_nof_words, uint32_t* got_nof_words);
    int readA32MBLT64(const uint32_t addr, uint32_t* dma_buffer, uint32_t request_nof_words, uint32_t* got_nof_words);
    int readA322E(const uint32_t addr, uint32_t* dma_buffer, uint32_t request_nof_words, uint32_t* got_nof_words);

    bool isBusError (int err) const { return err == 0x211; }

};

#endif // SIS3150MODULE_H
