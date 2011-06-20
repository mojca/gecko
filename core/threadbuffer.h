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

#ifndef THREADBUFFER_H
#define THREADBUFFER_H

#include <QObject>
#include <QReadWriteLock>
#include <QSemaphore>
#include <stdint.h>
#include <vector>
#include <iostream>
#include <QThread>

/*! Base class for signals of the ThreadBuffer template class. Provides signals. */
class ThreadBufferSignals : public QObject
{
    Q_OBJECT

public:
    ThreadBufferSignals() {;}

signals:
    void dataAvailable(int); /*!< signalled when data is available in the buffer. Deprecated? */
};

/*! Ring buffer for fast thread safe access.
 *  The ThreadBuffer class uses a ring buffer to store its contents. This ring buffer is protected by
 *  two semaphores that allow read access as long as data is available in the buffer and write access as long as the
 *  buffer is not full. Locking only takes place when one of these conditions is met. Calls that cause
 *  locking will block until either data is available or the buffer is not full anymore.
 */
template<class T>
class ThreadBuffer : public ThreadBufferSignals
{
public:
    /*! Create a ThreadBuffer.
     *  \param size number of elements the buffer can hold.
     *  \param chunkSize preferred read size.
     *  \param moduleId id of the module the ThreadBuffer belongs to.
     *  \param defaultValue the value that should be assigned to unused ringbuffer elements.
     *
     *  \todo Is moduleId needed anymore ?
     */
    ThreadBuffer(uint32_t size, uint32_t chunkSize, int moduleId, T defaultValue = NULL);
    ~ThreadBuffer();

    /*! Write data to the buffer.
     *  Writes \c len elements of data from the array pointed to by \c data to the buffer.
     *  \param data pointer to array containing the data.
     *  \param len  length of array
     */
    uint32_t write(T* data, uint32_t len);

    /*! read data from the buffer.
     *  Reads \c len elements from buffer and stores them to the vector \c data
     *  \param[out] data storage for the elements read from the buffer
     *  \param len amount of data to read
     */
    uint32_t read(std::vector<T> & data, uint32_t len);

    /*! read a chunk of data from the buffer.
     *  Reads all available elements from the buffer, at most \c chunkSize.
     */
    uint32_t readAvailable(std::vector<T> & data);

    /*! Returns the number of elements available for reading. */
    uint32_t available() const ;

    /*! Returns the space left in the buffer */
    uint32_t free () const;

    /*! Return the buffer size. */
    uint32_t getSize() const { return size; }
    /*! Return the chunk size. */
    uint32_t getchunkSize() const { return chunkSize; }
    /*! Return the id of the module the buffer belongs to. */
    int getModuleId() const { return moduleId; }
    /*! Resets the buffer.
     *  All elements in the buffer are discarded. After resetting, there are \c size elements available for writing
     *  and zero elements available for reading.
     */
    void reset();

private:
    QString name;
    mutable QReadWriteLock lock;
    QSemaphore* freeBytes;
    QSemaphore* usedBytes;
    T* buffer;
    uint32_t wpos, rpos;
    uint32_t size;
    uint32_t chunkSize;
    int moduleId;
    T defval;
};


template<class T>
ThreadBuffer<T>::ThreadBuffer(uint32_t _size, uint32_t _chunkSize, int _moduleId, T defaultValue)
        : size(_size), chunkSize(_chunkSize), moduleId(_moduleId), defval (defaultValue)
{
    buffer = new T[size];

    for(unsigned int i=0; i<size; i++)
    {
        buffer[i] = defval;
    }

    wpos = 0;
    rpos = 0;
    freeBytes = new QSemaphore(size);
    usedBytes = new QSemaphore(0);
}

template<class T>
ThreadBuffer<T>::~ThreadBuffer()
{
    delete [] buffer;
    buffer = NULL;
    delete freeBytes;
    freeBytes = NULL;
    delete usedBytes;
    usedBytes = NULL;
}

template<class T>
void ThreadBuffer<T>::reset()
{
    QWriteLocker locker (&lock);

    for(unsigned int i=0; i<size; i++)
    {
        buffer[i] = defval;
    }
    wpos = 0;
    rpos = 0;
    delete freeBytes;
    freeBytes = new QSemaphore(size);
    delete usedBytes;
    usedBytes = new QSemaphore(0);
}

template<class T>
uint32_t ThreadBuffer<T>::available() const
{
    QReadLocker locker (&lock);
    return usedBytes->available();
}

template<class T>
uint32_t ThreadBuffer<T>::free() const {
    QReadLocker locker (&lock);
    return freeBytes->available ();
}

template<class T>
uint32_t ThreadBuffer<T>::write(T* data, uint32_t len)
{
    uint32_t wordsWritten = 0;
    uint32_t dpos = 0;
    uint32_t toAcquire = 0;

    while(dpos < len)
    {
        //std::cout << thread()->currentThreadId() << ": tb write: Trying to acquire" << std::endl;

        if(len-dpos < chunkSize) toAcquire = len - dpos;
        else toAcquire = chunkSize;

        {
            QReadLocker locker (&lock);
            freeBytes->acquire(toAcquire);
            //std::cout << "tb write: Writing " << toAcquire << " words." << std::endl;
            uint32_t i = 0;
            while(i < toAcquire)
            {
                if(wpos == size) wpos = 0;
                buffer[wpos] = data[dpos + i];
                //std::cout << "tb write: Writing to " << wpos << " from " << dpos + i << std::endl;
                wpos++;
                i++;
            }
            dpos += toAcquire;
            //std::cout << "tb write: Releasing" << std::endl;
            usedBytes->release(toAcquire);
        }
        //std::cout << "tb write: Done" << std::endl;

        emit dataAvailable(moduleId);
    }
    wordsWritten = dpos;

    return wordsWritten;
}

template<class T>
uint32_t ThreadBuffer<T>::read(std::vector<T> & data, uint32_t len)
{
    QReadLocker locker (&lock);
    uint32_t wordsRead = 0;

    //std::cout << thread()->currentThreadId() << ": tb read : Trying to acquire" << std::endl;
    if(usedBytes->tryAcquire(len))
    {
        //std::cout << "tb read : Reading" << std::endl;
        for(uint32_t i = 0; i < len; i++)
        {
            if(rpos == size) rpos = 0;
            data[i] = buffer[rpos];
            //std::cout << "tb read : Reading from " << rpos << std::endl;
            rpos++;
        }
        //std::cout << "tb read : Releasing" << std::endl;
        freeBytes->release(len);
        //std::cout << "tb read : Done" << std::endl;

        wordsRead = len;
    }
    else
    {
        wordsRead = 0;
    }
    return wordsRead;
}

template<class T>
uint32_t ThreadBuffer<T>::readAvailable(std::vector<T> & data)
{
    QReadLocker locker (&lock);
    uint32_t wordsRead = 0;
    uint32_t wordsAvailable = 0;
    uint32_t bytesToRead = 0;

    if(usedBytes->tryAcquire(1,10))
    {
        usedBytes->release(1);
        wordsAvailable = usedBytes->available();
        //std::cout << "available : " << wordsAvailable << " words." << std::endl;
        if(wordsAvailable > chunkSize && false) bytesToRead = chunkSize;
        else bytesToRead = wordsAvailable;

        data.resize(bytesToRead);
        wordsRead = read(data, bytesToRead);
    }
    //std::cout << "read : " << wordsRead << " words." << std::endl;
    return wordsRead;
}


#endif // THREADBUFFER_H
