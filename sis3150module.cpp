#include <iostream>
#include "sis3150module.h"
#include "modulemanager.h"

static ModuleRegistrar registrar ("sis3150", Sis3150Module::create, AbstractModule::TypeInterface);

Sis3150Module::Sis3150Module(int _id, QString _name)
    : BaseInterface(_id, _name)
    , id (getId ())
    , name (getName ())
{
    deviceOpen = false;

    // Setup
    conf.base_addr = 0x20000000;

    // Create user interface
    setUI (new Sis3150UI(this));

    std::cout << "Instantiated Sis3150 Module" << std::endl;
}

Sis3150Module::~Sis3150Module()
{
    if(isOpen()) this->close();
}

int Sis3150Module::open()
{
    int status;
    unsigned int found;

    Sis3150UI* ui = dynamic_cast<Sis3150UI*>(getUI());

    //! Find connected devices
    status = FindAll_SIS3150USB_Devices(info, &found, 1);
    ui->outputText("Found "
                   + QString("%1").arg(found,2)
                   + " devices with status "
                   + QString("%1").arg(status,2,16) + "\n");
    if(status != 0)
    {
        ui->outputText("No device found!\n");
        return 1;
    }

    //! Open the device
    if(found >= 1)
    {
        status = Sis3150usb_OpenDriver((PCHAR)info[0].cDName, &m_device);
        if(status != 0)
        {
            ui->outputText("ERROR: "
                   + QString("%1").arg(status,2,16)
                   + " \n");
            return status;
        }
    }
    else
    {
        ui->outputText("No device found!\n");
        return 1;
    }
    deviceOpen = true;
    ui->outputText("Device opened.\n");
    ui->moduleOpened ();

    return 0;
}

bool Sis3150Module::isOpen()
{
    return deviceOpen;
}

int Sis3150Module::close()
{
    int status;

    Sis3150UI* ui = dynamic_cast<Sis3150UI*>(getUI());

    //! Close the device
    status = Sis3150usb_CloseDriver(m_device);
    deviceOpen = false;
    ui->outputText("Device closed\n");
    ui->moduleClosed ();

    return 0;
}

int Sis3150Module::readA32D32(const uint32_t addr, uint32_t* data)
{
    //return vme_A32D32_read(m_device,addr,data);
    return sis3150_vme_A32D32_read(m_device,addr,data);
}

int Sis3150Module::writeA32D32(const uint32_t addr, const uint32_t data)
{
    //return vme_A32D32_write(m_device,addr,data);
    return sis3150_vme_A32D32_write(m_device,addr,data);
}

int Sis3150Module::readA32D16(const uint32_t addr, uint16_t* data)
{
    //return vme_A32D16_read(m_device,addr,data);
    return sis3150_vme_A32D16_read(m_device,addr,data);
}

int Sis3150Module::writeA32D16(const uint32_t addr, const uint16_t data)
{
    //return vme_A32D16_write(m_device,addr,data);
    return sis3150_vme_A32D16_write(m_device,addr,data);
}

int Sis3150Module::readA32FIFO(const uint32_t addr, uint32_t* dma_buffer,
                                uint32_t request_nof_words, uint32_t* got_nof_words)
{
    return sis3150Usb_Vme_Dma_Read(m_device, addr, 0x9, 4, 1, dma_buffer, request_nof_words, got_nof_words);
}

int Sis3150Module::readA32BLT32(const uint32_t addr, uint32_t* dma_buffer,
                                uint32_t request_nof_words, uint32_t* got_nof_words)
{
    return sis3150Usb_Vme_Dma_Read(m_device, addr, 0xf, 4, 0, dma_buffer, request_nof_words, got_nof_words);
}

int Sis3150Module::readA32MBLT64(const uint32_t addr, uint32_t* dma_buffer,
                                uint32_t request_nof_words, uint32_t* got_nof_words)
{
    return sis3150Usb_Vme_Dma_Read(m_device, addr, 0x8, 8, 0, dma_buffer, request_nof_words & 0xfffffffe, got_nof_words);
}

int Sis3150Module::readA322E(const uint32_t addr, uint32_t* dma_buffer,
                             uint32_t request_nof_words, uint32_t* got_nof_words)
{
    //return sis3150Usb_Vme_Dma_Read(m_device, addr, 0x20, 4, 0, dma_buffer, request_nof_words, got_nof_words);
    return sis3150Usb_Vme_Dma_Read(m_device, addr, 0x8, 8, 0, dma_buffer, request_nof_words & 0xfffffffe, got_nof_words);
}

/*!
\page sis3150if SIS 3150 USB-to-VME Interface
<b>Module name:</b> \c sis3150

\section desc Module Description
Provides access to a VME crate via a USB interface.

\section Configuration Panel
The configuration panel provides basic test functionality and the means for sending arbitrary commands over the VME bus.
*/

