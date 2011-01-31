#ifndef SIS3150CONTROL_H
#define SIS3150CONTROL_H

#include <QObject>
#include <cstdio>
#include <sis3150usb_vme.h>
#include <cstring>
#include <inttypes.h>
//#include "sis3150usb_vme_calls.h"
#include "../local_3350readout/sis3350.h"
#include "../local_3350readout/pc_vme_interface_defines.h"

#define MAXNUMDEV 10

class Sis3150control : public QObject
{
    Q_OBJECT

public:
    Sis3150control();

    int open();
    int close();

    bool isOpen();

    HANDLE getDevice() { return this->m_device; }

private:
    // Device variables
    struct SIS3150USB_Device_Struct info[MAXNUMDEV];
    HANDLE m_device;
    unsigned int addr,data;

    bool deviceOpen;
    void out(QString);

signals:
    void outputText(QString&);

};

#endif // SIS3150CONTROL_H
