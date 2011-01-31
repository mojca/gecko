#include "sis3150control.h"

Sis3150control::Sis3150control()
{
    deviceOpen = false;
}

int Sis3150control::open()
{
    int status;
    unsigned int found;

    //! Find connected devices
    status = FindAll_SIS3150USB_Devices(info, &found, 1);
    fprintf(stdout,"Found %d devices with status %d \n",found,status);
    out(tr("Found %1 devices with status %2 \n").arg(found).arg(status));
    if(status != 0)
    {
        fprintf(stderr,"No device found!\n");
        out(tr("No device found!\n"));
        return 1;
    }

    //! Open the device
    if(found >= 1)
    {
        status = Sis3150usb_OpenDriver((PCHAR)info[0].cDName, &m_device);
        if(status != 0)
        {
            fprintf(stderr,"ERROR: %d\n",status);
            out(tr("ERROR: %d\n").arg(status));
            return status;
        }
    }
    else
    {
        fprintf(stderr,"No device found!\n");
        out(tr("No device found!\n"));
        return 1;
    }
    deviceOpen = true;
    fprintf(stdout,"Device 0x%08lx opened.\n",(long int)m_device);
    out(tr("Device 0x%1 opened.\n").arg((long int)m_device,8,16));

    return 0;
}

bool Sis3150control::isOpen()
{
    return deviceOpen;
}

int Sis3150control::close()
{
    int status;

    //! Close the device
    status = Sis3150usb_CloseDriver(m_device);
    deviceOpen = false;
    fprintf(stdout,"Device closed\n");
    out(tr("Device closed\n"));

    return 0;
}

void Sis3150control::out(QString text)
{
    emit outputText(text);
    text.clear();
}
