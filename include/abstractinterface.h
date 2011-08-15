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

#ifndef ABSTRACTINTERFACE_H
#define ABSTRACTINTERFACE_H

#include <stdint.h>

#include <QObject>
#include <QString>

class BaseUI;
class QSettings;

/*! Abstract base class for all VME interfaces.
 * To implement a new interface please derive from BaseInterface which already handles ids, names and types. */
class AbstractInterface : public QObject {
public:
    virtual ~AbstractInterface () {}

    /*! returns the interface name. */
    virtual const QString& getName () const = 0;

    /*! returns the interface type. */
    virtual QString getTypeName () const = 0;

    /*! returns the UI for the interface. */
    virtual BaseUI* getUI () const = 0;

    /*! returns the interface id */
    virtual int getId () const = 0;

    /*! Save the interface settings to the given QSettings object.
     *  The implementation should read the subsection named like the interface instance
     *  and save all settings inside to a local data structure, because lifetime of the settings object
     *  is not guaranteed to be longer than the lifetime of this object.
     *  \sa #applySettings, InterfaceManager::saveSettings
     */
    virtual void saveSettings(QSettings*) = 0;

    /*! Load the interface settings from the given QSettings object.
     *  The implementation should create a new subsection named like the interface instance
     *  and save all settings inside this section.
     *  \sa #saveSettings, InterfaceManager::applySettings
     */
    virtual void applySettings(QSettings*) = 0;

    /*! check if the interface is open. */
    virtual bool isOpen() const = 0;

    /*! open the interface. After opening, the interface is ready to perform VME communication */
    virtual int open() = 0;

    /*! close the interface. */
    virtual int close() = 0;

    /*! set the first NIM output on the interface board to the specified logic level, if available */
    virtual int setOutput1(bool) = 0;

    /*! set the second NIM output on the interface board to the specified logic level, if available */
    virtual int setOutput2(bool) = 0;

    /*! read a 32-bit word from the specified address. */
    virtual int readA32D32(const uint32_t addr, uint32_t* data) = 0;
    /*! read a 16-bit word from the specified address. */
    virtual int readA32D16(const uint32_t addr, uint16_t* data) = 0;
    /*! read multiple 32-bit words, starting with the specified address.
     *  This function continuously reads words into the dma_buffer, increasing the address after each successful read.
     *  If a bus error occurs reading is stopped and the number of words read is returned in got_nof_words.
     *
     *  This function uses D32 requests to access the VME module.
     */
    virtual int readA32DMA32(const uint32_t addr, uint32_t* dma_buffer, uint32_t request_nof_words, uint32_t* got_nof_words) = 0;
    /*! read multiple 32-bit words from the specified address.
     *  This function performs at most \c request_nof_words reads on address \c addr, storing
     *  the words into the dma_buffer. If a bus error occurs, the reading stops and the number of words read
     *  is stored in \c got_nof_words.
     *  \sa readA32BLT32, readA32MBLT64
     */
    virtual int readA32FIFO(const uint32_t addr, uint32_t* dma_buffer, uint32_t request_nof_words, uint32_t* got_nof_words) = 0;
    /*! read 32-bit words in BLT32 mode, beginning from the specified address.
     *  This function continuously reads words into the dma_buffer, increasing the address after each successful read.
     *  If a bus error occurs reading is stopped and the number of words read is returned in got_nof_words.
     */
    virtual int readA32BLT32(const uint32_t addr, uint32_t* dma_buffer, uint32_t request_nof_words, uint32_t* got_nof_words) = 0;
    /*! read 32-bit words in MBLT64 mode, beginning from the specified address.
     *  This function continuously reads words into the dma_buffer, increasing the address after each successful read.
     *  If a bus error occurs reading is stopped and the number of words read is returned in got_nof_words.
     */
    virtual int readA32MBLT64(const uint32_t addr, uint32_t* dma_buffer, uint32_t request_nof_words, uint32_t* got_nof_words) = 0;
    /*! read 32-bit words in 2E mode, beginning from the specified address.
     *  This function continuously reads words into the dma_buffer, increasing the address after each successful read.
     *  If a bus error occurs reading is stopped and the number of words read is returned in got_nof_words.
     */
    virtual int readA322E(const uint32_t addr, uint32_t* dma_buffer, uint32_t request_nof_words, uint32_t* got_nof_words) = 0;
    /*! write a 32-bit word to the specified address. */
    virtual int writeA32D32(const uint32_t addr, const uint32_t data) = 0;
    /*! write a 16-bit word to the specified address. */
    virtual int writeA32D16(const uint32_t addr, const uint16_t data) = 0;

    /*! Return whether the given error code is a bus error or not. */
    virtual bool isBusError (int err) const = 0;

protected:
    /*! Called by the interface manager when a name change is requested. */
    virtual void setName (QString newName) = 0;

    /*! Called by the interface manager to set the type for later retrieval. */
    virtual void setTypeName (QString newtype) = 0;

    friend class InterfaceManager;
};

#endif // ABSTRACTINTERFACE_H
