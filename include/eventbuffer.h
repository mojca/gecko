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

#ifndef EVENTBUFFER_H
#define EVENTBUFFER_H

#include <stddef.h>
#include <QMap>
#include <QList>
#include <QSet>
#include <QVariant>

#include "pluginconnector.h"

class EventSlot;
class Event;
class AbstractModule;
template<typename T> class ThreadBuffer;

class EventBuffer {
public:
    /*! Construct an event buffer containing at most \c size events */
    EventBuffer (size_t size);
    ~EventBuffer ();

    bool empty () const; /*!< Returns whether the event buffer is empty. */
    bool full () const; /*!< Returns whether the event buffer is full. */
    size_t size () const; /*!< Returns the maximum size of the event buffer. */
    size_t level () const; /*!< Returns the current number of events in the buffer. */

    /*! Changes the size of the underlying queue to contain \c newsz elements.
        This function should never be called while the buffer is in use (eg. during a run) because
        all data in the buffer will be lost and and undefined behaviour could occur if the buffer is accessed
        while its size is changed.
        \param newsz the new size of the event buffer.
     */
    void setSize (size_t newsz);

    /*! Create a new event. The object has to be returned via #releaseEvent when it is not used anymore. */
    Event* createEvent ();

    /*! Releases an event object obtained via #createEvent. The object is scheduled for reuse. */
    void releaseEvent (Event *);

    /*! Queues an event in the buffer. The buffer takes ownership of the event.
        This call is synchronous. It waits until there is enough room inside the buffer to queue the event.
     */
    bool queue (Event *ev);

    /*! Returns the first event in the buffer. The caller takes ownership of the event object.
        This call is non-blocking. The function will return immediately if no data is available.
     */
    Event* dequeue ();

    // EventSlot management
    /*! Register an event slot with the event buffer.
        Data may only be sent through and retrieved from the event buffer through an event slot.
        \param owner The module owning the slot. Data associated with this slot will come from this module.
        \param name  Name of the slot, should be somewhat descriptive as it is shown to the user when configuring the experiment.
        \param type  The data type of the slot. See PluginConnector::DataType for details.

        \returns a new event slot.
    */
    EventSlot *registerSlot (const AbstractModule *owner, QString name, PluginConnector::DataType type);

    /*! Retrieves a registered event slot with the specified name belonging to the specified owner. */
    EventSlot *getEventSlot (const AbstractModule *owner, QString name) const;

    /*! Returns all slots registered by the given module. */
    const QList<EventSlot *>* getEventSlots (const AbstractModule *owner) const;

    /*! Deletes the given slot. */
    void destroyEventSlot (EventSlot* slot);

private:
    typedef QList<EventSlot*> SlotSet;
    typedef QMap< const AbstractModule*, SlotSet* > SlotMap;
    SlotMap Slots_;

    ThreadBuffer<Event*>* Buffer_;
    ThreadBuffer<Event*>* UnusedQ_;
};

class Event {
public:
    Event (EventBuffer *buffer);
    ~Event ();

    void put (const EventSlot *, QVariant data);
    QVariant get (const EventSlot *) const;
    void clear ();

    QSet<const EventSlot *> getOccupiedSlots () const;

    EventBuffer *getBuffer () const;

private:
    typedef QMap<const EventSlot *, QVariant > DataMap;
    DataMap Data_;
    EventBuffer* EvBuf_;
};

class EventSlot {
public:
    EventSlot (const AbstractModule* owner, QString name, PluginConnector::DataType dtype)
    : Owner_ (owner)
    , Name_ (name)
    , Dtype_ (dtype)
    {}

    const AbstractModule* getOwner () const { return Owner_; }
    QString getName () const { return Name_; }
    PluginConnector::DataType getDataType () const { return Dtype_; }

private:
    const AbstractModule *Owner_;
    QString Name_;
    PluginConnector::DataType Dtype_;
};

#endif // EVENTBUFFER_H
