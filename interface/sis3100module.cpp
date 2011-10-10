/*
Copyright 2011 Bastian Loeher, Roland Wirth

This file is part of GECKO.

GECKO is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

GECKO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "sis3100module.h"
#include "interfacemanager.h"

#include <iostream>

static InterfaceRegistrar registrar ("sis3100", Sis3100Module::create);

Sis3100Module::Sis3100Module(int _id, QString _name)
    : BaseInterface(_id, _name)
    , id (getId ())
    , name (getName ())
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

    // Set disable VME IRQ
    s3100_control_write(m_device,SIS3104_IRQ,(1 << 16));

    // Read Type/Version
    uint32_t opt_vme_type_version = 0;
    s3100_control_read(m_device,0x0,&opt_vme_type_version);
    ui->outputText(tr("opt/vme type/version: 0x%1\n").arg(opt_vme_type_version,8,16));

    // Read Master status
    uint32_t opt_vme_master_status = 0;
    s3100_control_read(m_device,0x100,&opt_vme_master_status);
    ui->outputText(tr("opt/vme master status: 0x%1\n").arg(opt_vme_master_status,8,16));

    // Read interrupt status
    uint32_t opt_vme_interrupt_status = 0;
    s3100_control_read(m_device,SIS3104_IRQ,&opt_vme_interrupt_status);
    ui->outputText(tr("opt/vme interrupt status: 0x%1\n").arg(opt_vme_interrupt_status,8,16));

    // Set BERR timeout to 100 us
    //s3100_control_write(m_device,0x100,(1<<15));
    //s3100_control_write(m_device,SIS3104_VME_MASTER,vme_berr_timer);

    return 0;
}

bool Sis3100Module::isOpen() const
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

int Sis3100Module::readA32DMA32(const uint32_t addr, uint32_t* dma_buffer,
                                uint32_t request_nof_words, uint32_t* got_nof_words)
{
    return sis3100_vme_A32DMA_D32_read(m_device, addr, dma_buffer, request_nof_words, got_nof_words);
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

/*!
\page sis3100if SIS 3100 PCIe-to-VME Interface
<b>Module name:</b> \c sis3100

\section desc Module Description
Provides access to a VME crate via a PCIe interface card.

\section Configuration Panel
The configuration panel provides basic test functionality and the means for sending arbitrary commands over the VME bus.
*/

