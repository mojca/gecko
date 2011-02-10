#ifndef BASEINTERFACE_H
#define BASEINTERFACE_H

#include "abstractinterface.h"

class BaseInterface : public AbstractInterface {
public:
    BaseInterface (int id, QString name)
    : id_ (id)
    , name_ (name)
    {
    }

    int getId () const { return id_; }
    const QString& getName () const { return name_; }
    QString getTypeName () const { return type_; }

protected:
    void setName (QString newName) { name_ = newName; }
    void setTypeName (QString newType) { type_ = newType; }
private:
    int id_;
    QString name_;
    QString type_;
};

#endif // BASEINTERFACE_H
