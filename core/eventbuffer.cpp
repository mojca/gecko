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

#include "eventbuffer.h"
#include "threadbuffer.h"

#include <QAtomicInt>

EventBuffer::EventBuffer (size_t size)
: Buffer_ (new ThreadBuffer<Event*> (size, 1, -1))
, UnusedQ_ (new ThreadBuffer<Event*> (size, 1, -1))
{
}

bool EventBuffer::empty () const {
    return Buffer_->available () == 0;
}

bool EventBuffer::full () const {
    return Buffer_->free () == 0;
}

size_t EventBuffer::size () const {
    return Buffer_->getSize ();
}

size_t EventBuffer::level () const {
    return Buffer_->available ();
}

void EventBuffer::setSize (size_t newsz) {
    ThreadBuffer<Event*>* newbuf = new ThreadBuffer<Event*> (newsz, 1, -1);
    ThreadBuffer<Event*>* newq = new ThreadBuffer<Event*> (newsz, 1, -1);
    ThreadBuffer<Event*>* oldbuf = Buffer_;
    ThreadBuffer<Event*>* oldq = UnusedQ_;

    // prepare the event pool
    std::vector<Event*> evs;
    oldq->readAvailable (evs);

    while (evs.size() > newsz) {
        Event *b = evs.back ();
        evs.pop_back ();
        delete b;
    }
    newq->write (evs.data(), evs.size());

    Buffer_ = newbuf;
    UnusedQ_ = newq;
    delete oldbuf;
    delete oldq;
}

Event* EventBuffer::createEvent () {
    std::vector<Event*> rd (1, NULL);
    if (UnusedQ_->read (rd, 1) != 1) {
        return new Event (this);
    }

    return rd.front ();
}

void EventBuffer::releaseEvent (Event *ev) {
    if (UnusedQ_->free() != 0) {
        ev->clear ();
        UnusedQ_->write(&ev, 1);
    } else {
        delete ev;
    }
}

bool EventBuffer::queue (Event *ev) {
    return Buffer_->write(&ev, 1) == 1;
}

Event* EventBuffer::dequeue () {
    std::vector<Event*> rd (1, NULL);
    if (Buffer_->read(rd, 1) < 1)
        return NULL;

    return rd.front ();
}

EventSlot *EventBuffer::registerSlot (const AbstractModule *owner, QString name, PluginConnector::DataType type) {
    EventSlot *slot = new EventSlot (owner, name, type);
    if (Slots_.find (owner) == Slots_.end ()) // owning module not yet in registry
        Slots_.insert (owner, new SlotSet ());
    Slots_.value (owner)->push_back (slot);
    return slot;
}

EventSlot *EventBuffer::getEventSlot (const AbstractModule *owner, QString name) const {
    SlotMap::const_iterator i;
    if ((i = Slots_.find (owner)) != Slots_.end ()) {
        SlotSet* s = i.value ();
        for (SlotSet::const_iterator j = s->begin (); j != s->end (); ++j) {
            if ((*j)->getName () == name)
                return *j;
        }
    }
    return NULL;
}

const EventBuffer::SlotSet* EventBuffer::getEventSlots (const AbstractModule *owner) const {
    SlotMap::const_iterator i = Slots_.find (owner);
    return i != Slots_.end () ? i.value () : NULL;
}

void EventBuffer::destroyEventSlot (EventSlot *slot) {
    SlotMap::iterator i = Slots_.find (slot->getOwner ());
    if (i != Slots_.end ()) {
        SlotSet* s = i.value ();
        s->removeOne (slot);
        if (s->empty ()) { // remove the module entry from the registry
            Slots_.erase (i);
            delete s;
        }
    }
}

EventBuffer::~EventBuffer () {
    for (SlotMap::iterator i = Slots_.begin (); i != Slots_.end ();) {
        SlotSet *s = i.value ();
        i = Slots_.erase (i);

        for (SlotSet::iterator j = s->begin (); j != s->end ();) {
            EventSlot* slot = *j;
            j = s->erase (j);
            delete slot;
        }
        delete s;
    }

    std::vector<Event*> rd;
    UnusedQ_->readAvailable (rd);
    for (std::vector<Event*>::iterator i = rd.begin (); i != rd.end (); ++i)
        delete *i;

    delete Buffer_;
    delete UnusedQ_;
}

Event::Event (EventBuffer *buffer)
: EvBuf_ (buffer)
{
}

Event::~Event ()
{
}

void Event::put (const EventSlot *slot, QVariant data) {
    Data_.insert (slot, data);
}

QVariant Event::get(const EventSlot *slot) const {
    DataMap::const_iterator i = Data_.find (slot);
    if (i != Data_.end ())
        return i.value ();
    return QVariant ();
}

QSet<const EventSlot *> Event::getOccupiedSlots () const {
    QSet<const EventSlot *> ret;
    ret.reserve (Data_.size ());

    for (DataMap::const_iterator i = Data_.begin (); i != Data_.end (); ++i)
        if (!i.value ().isNull ())
            ret.insert (i.key ());
    return ret;
}

void Event::clear () {
    for (DataMap::iterator i = Data_.begin (); i != Data_.end (); ++i) {
        if (i.value ().canConvert< QVector<uint32_t> > ()) {
            QVector<uint32_t> v = i.value().value< QVector<uint32_t> > ();
            v.reserve (v.size ());
            v.resize (0);
            i.value () = QVariant::fromValue (v);
        }

        i.value ().clear ();
    }
}

EventBuffer *Event::getBuffer () const {
    return EvBuf_;
}
