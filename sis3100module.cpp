#include "sis3100module.h"
#include "modulemanager.h"

#include <iostream>

static ModuleRegistrar registrar ("sis3100", Sis3100Module::create, AbstractModule::TypeInterface);

Sis3100Module::Sis3100Module(int _id, QString _name)
    : BaseInterfaceModule(_id, _name)
{
    deviceOpen = false;

    devicePath = tr("/dev/sis1100_00remote");
    controlPath = tr("/dev/sis1100_00ctrl");

    // Create channels container
    // Create user interface
    setUI (new Sis3100UI(this));
    std::cout << "Instantiated Sis3100 Module" << std::endl;
}

Sis3100Module::~Sis3100Module()
{
    if(isOpen()) this->close();
}

int Sis3100Module::open()
{
    Sis3100UI* ui = dynamic_cast<Sis3100UI*>(getUI ());

    m_device = ::open(devicePath.toStdString().c_str(), O_RDWR, 0);
    if(m_device < 0)
    {
        ui->outputText("failed to open SIS1100/3104\n");
        return -1;
    }
    else
    {
        ui->outputText("open and init SIS1100/3104 OK\n");
        deviceOpen = true;
    }

    c_device = ::open(controlPath.toStdString().c_str(), O_RDWR, 0);
    if(c_device < 0)
    {
        ui->outputText("failed to open SIS1100/3104 control\n");
        return -1;
    }
    else
    {
        ui->outputText("opened control device for SIS1100/3104\n");

    }

    // Set BERR timeout to 100 us
    //s3100_control_write(m_device,0x100,(1<<15));
    //s3100_control_write(m_device,SIS3104_VME_MASTER,vme_berr_timer);

    return 0;
}

bool Sis3100Module::isOpen()
{
    return deviceOpen;
}

int Sis3100Module::close()
{
    ::close(m_device);
    ::close(c_device);
    Sis3100UI* ui = dynamic_cast<Sis3100UI*>(getUI());
    ui->outputText("closed SIS1100/3104\n");
    deviceOpen = false;
    return 0;
}

int Sis3100Module::setOutput1(bool enable)
{
    int ret = 0;
    if(enable) ret = s3100_control_write(m_device,SIS3104_IO,sis3104_set_nim_out1);
    else ret = s3100_control_write(m_device,SIS3104_IO,sis3104_clear_nim_out1);
    return ret;
}

int Sis3100Module::setOutput2(bool enable)
{
    int ret = 0;
    if(enable) ret =  s3100_control_write(m_device,SIS3104_IO,sis3104_set_nim_out2);
    else ret =  s3100_control_write(m_device,SIS3104_IO,sis3104_clear_nim_out2);
    return ret;
}

int Sis3100Module::vmeSingleRead(const uint32_t addr, uint32_t* data)
{
    Sis3100UI* ui = dynamic_cast<Sis3100UI*>(getUI());

    if(addrMode == "A32")
    {
        if(dataMode == "D32")
        {
            ui->outputText("readA32D32\n");
            return readA32D32(addr,data);
        }
        if(dataMode == "D16")
        {
            uint16_t data16 = 0xffff & (*data);
            ui->outputText("readA32D16\n");
            int ret = readA32D16(addr,&data16);
            (*data) = 0x0000ffff & data16;
            return ret;
        }
    }
    else
    {
        return 0xDEADBEEF;
    }
    return -1;
}

int Sis3100Module::vmeSingleWrite(const uint32_t addr, const uint32_t data)
{
    if(addrMode == "A32")
    {
        if(dataMode == "D32") return writeA32D32(addr,data);
        if(dataMode == "D16") return writeA32D16(addr,data);
    }
    else
    {
        return 0xDEADBEEF;
    }
    return -1;
}

int Sis3100Module::readA32D32(const uint32_t addr, uint32_t* data)
{
    return sis3100_vme_A32D32_read(m_device,addr,data);
}

int Sis3100Module::readA32D16(const uint32_t addr, uint16_t* data)
{
    return sis3100_vme_A32D16_read(m_device,addr,data);
}

int Sis3100Module::writeA32D32(const uint32_t addr, const uint32_t data)
{
    return sis3100_vme_A32D32_write(m_device,addr,data);
}

int Sis3100Module::writeA32D16(const uint32_t addr, const uint16_t data)
{
    return sis3100_vme_A32D16_write(m_device,addr,data);
}

int Sis3100Module::readA32FIFO(const uint32_t addr, uint32_t* dma_buffer,
                                uint32_t request_nof_words, uint32_t* got_nof_words)
{
    return sis3100_vme_A32DMA_D32FIFO_read(m_device, addr, dma_buffer, request_nof_words, got_nof_words);
}

int Sis3100Module::readA32BLT32(const uint32_t addr, uint32_t* dma_buffer,
                                uint32_t request_nof_words, uint32_t* got_nof_words)
{
    return sis3100_vme_A32BLT32_read(m_device, addr, dma_buffer, request_nof_words, got_nof_words);
}

int Sis3100Module::readA32MBLT64(const uint32_t addr, uint32_t* dma_buffer,
                                uint32_t request_nof_words, uint32_t* got_nof_words)
{
    return sis3100_vme_A32MBLT64_read(m_device, addr, dma_buffer, request_nof_words, got_nof_words);
}

int Sis3100Module::readA322E(const uint32_t addr, uint32_t* dma_buffer,
                             uint32_t request_nof_words, uint32_t* got_nof_words)
{
    return sis3100_vme_A32_2EVME_read(m_device, addr, dma_buffer, request_nof_words, got_nof_words);
}

int Sis3100Module::acquire()
{
    return -1;
}
