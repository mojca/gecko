#ifndef EVENTBUFFER_H
#define EVENTBUFFER_H

#include <stddef.h>
#include <map>
#include <vector>

#include "pluginconnector.h"
#include "threadbuffer.h"

class EventSlot;
class Event;
class AbstractModule;

class EventBuffer {
public:
    /*! Construct an event buffer containing at most \c size events */
    EventBuffer (size_t size);
    ~EventBuffer ();

    bool empty () const; /*!< Returns whether the event buffer is empty. */
    bool full () const; /*!< Returns whether the event buffer is full. */
    size_t size () const; /*!< Returns the maximum size of the event buffer. */
    size_t level () const; /*!< Returns the current number of events in the buffer. */

    /*! Create a new event. The object has to be deleted when it is not used anymore. */
    Event* createEvent () const;

    /*! Queues an event in the buffer. The buffer takes ownership of the event. */
    bool queue (Event *ev);

    /*! Returns the first event in the buffer, or NULL if the buffer is empty. The caller takes ownership of the event object. */
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
    std::vector<const EventSlot *> getEventSlots (const AbstractModule *owner) const;

    /*! Deletes the given slot. */
    void destroyEventSlot (EventSlot** slot);

private:
    typedef std::map< const AbstractModule*, std::vector<EventSlot*> > SlotMap;
    SlotMap Slots_;

    ThreadBuffer<Event*> Buffer_;
};

class Event {
public:
    Event (EventBuffer *buffer);
    ~Event ();

    void put (const EventSlot *, const void *data);
    const void* get (const EventSlot *) const;

private:
    typedef std::map<const EventSlot *, const void *> DataMap;
    DataMap Data_;
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
