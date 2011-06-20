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

#ifndef SIS3100MODULE_H
#define SIS3100MODULE_H

//#include <sis1100_var.h>
#include "../sis3100_calls/sis3100_vme_calls.h"  // Contains all function definitions to use the driver
#include <fcntl.h>              // Contains open and close

#include "baseinterface.h"
#include "sis3100ui.h"

/* Registers for SIS3104 */
#define SIS3104_IDENTIFICATION      0x000
#define SIS3104_OPTICAL_STATUS      0x004
#define SIS3104_OPTICAL_CONTROL     0x008
#define SIS3104_IO                  0x080
#define SIS3104_IO_LATCH_IRQ        0x084
#define SIS3104_IO_LEVEL_CONTROL    0x088
#define SIS3104_VME_MASTER          0x100
#define SIS3104_IRQ                 0x104
#define SIS3104_DMA_WRITE_COUNTER   0x204
/* bits in in_out for SIS3104 */
#define sis3104_set_nim_out1 (1<<0)
#define sis3104_set_nim_out2 (1<<1)
#define sis3104_set_ttl_out1 (1<<2)
#define sis3104_set_ttl_out2 (1<<3)
#define sis3104_clear_nim_out1 (1<<16)
#define sis3104_clear_nim_out2 (1<<17)
#define sis3104_clear_ttl_out1 (1<<18)
#define sis3104_clear_ttl_out2 (1<<19)
#define sis3104_io_nim_pulse1 (1<<24)
#define sis3104_io_nim_pulse2 (1<<25)
#define sis3104_io_ttl_pulse1 (1<<26)
#define sis3104_io_ttl_pulse2 (1<<27)
/* bits in vme_master_sc */
#define vme_system_controller_enable (1<<0)
#define vme_sys_reset         (1<<1)
#define vme_power_on_reset    (1<<3)
#define vme_request_level     (3<<4)
#define vme_requester_type    (1<<6)
#define vme_disable_retry     (1<<9)
#define vme_force_dearbit     (1<<10)
#define vme_long_timer        (3<<12)
#define vme_berr_timer        (3<<14)
#define vme_system_controller (1<<16)


class QSettings;

class Sis3100Module : public BaseInterface
{
    Q_OBJECT

protected:
    int id;
    const QString& name;
    int m_device;
    int c_device;
    bool deviceOpen;
    void out(QString);
    QString devicePath;
    QString controlPath;

private:
    Sis3100Module(int _id, QString name = "SIS 3100");

public:
    ~Sis3100Module();

    // Factory method
    static AbstractInterface *create (int id, const QString &name) {
        return new Sis3100Module (id, name);
    }

    virtual int open();
    virtual int close();
    virtual bool isOpen() const ;

    virtual int setOutput1(bool);
    virtual int setOutput2(bool);

    virtual void saveSettings(QSettings*) {}
    virtual void applySettings(QSettings*) {}

    // VME access
    int readA32D32(const uint32_t addr, uint32_t* data);
    int readA32D16(const uint32_t addr, uint16_t* data);
    int readA32DMA32(const uint32_t addr, uint32_t* dma_buffer, uint32_t request_nof_words, uint32_t* got_nof_words);
    int readA32FIFO(const uint32_t addr, uint32_t* dma_buffer, uint32_t request_nof_words, uint32_t* got_nof_words);
    int readA32BLT32(const uint32_t addr, uint32_t* dma_buffer, uint32_t request_nof_words, uint32_t* got_nof_words);
    int readA32MBLT64(const uint32_t addr, uint32_t* dma_buffer, uint32_t request_nof_words, uint32_t* got_nof_words);
    int readA322E(const uint32_t addr, uint32_t* dma_buffer, uint32_t request_nof_words, uint32_t* got_nof_words);
    int writeA32D32(const uint32_t addr, const uint32_t data);
    int writeA32D16(const uint32_t addr, const uint16_t data);

    bool isBusError (int err) const { return err == 0x211; }

    int acquire();
};

#endif // SIS3100MODULE_H
