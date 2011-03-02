#ifndef INTTODOUBLEPLUGIN_H
#define INTTODOUBLEPLUGIN_H

#include "baseplugin.h"

#include <vector>

class IntToDoublePlugin : public BasePlugin
{
Q_OBJECT
public:
    IntToDoublePlugin(int id, QString name, const Attributes &attrs);
    static AbstractPlugin *create (int id, const QString &name, const Attributes &attrs) {
        return new IntToDoublePlugin (id, name, attrs);
    }

    void createSettings (QGridLayout *);

    AttributeMap getAttributeMap () const;
    Attributes getAttributes () const;
    static AttributeMap getIntToDoubleAttributeMap ();
    virtual void process();
    virtual void userProcess () {}

    virtual void applySettings(QSettings*) {}
    virtual void saveSettings(QSettings*) {}

private:
    std::vector< std::vector<double> > outData_;
    Attributes attrs_;

    int nofChannels_;
};

#endif // INTTODOUBLEPLUGIN_H
