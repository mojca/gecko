#ifndef ABSTRACTMANAGER_H
#define ABSTRACTMANAGER_H

#include <QList>
#include <QString>

template <class T>
class AbstractManager
{
public:
    virtual ~AbstractManager() {}

    virtual int add(T*) = 0;
    virtual bool remove(T*) = 0;
    virtual const QList<T*>* list() = 0;
    virtual T* getById(int) = 0;
    virtual T* getByName(QString) = 0;
};

#endif // ABSTRACTMANAGER_H
