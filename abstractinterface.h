#ifndef ABSTRACTINTERFACE_H
#define ABSTRACTINTERFACE_H

#include <stdint.h>

#include <QObject>
#include <QString>

/*! Base class for all VME interfaces. */
class AbstractInterface : public QObject {
public:
    virtual ~AbstractInterface () = 0;

    /*! returns the interface name. */
    virtual const QString& getName () const = 0;

    /*! returns the interface type. */
    virtual QString getTypeName () const = 0;

    /*! returns the interface id */
    virtual int getId () const = 0;

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
    /*! read 32-bit words from the specified address.
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
