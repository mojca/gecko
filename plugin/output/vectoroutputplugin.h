#ifndef VECTOROUTPUTPLUGIN_H
#define VECTOROUTPUTPLUGIN_H


#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <iostream>
#include <vector>

#include "baseplugin.h"

class BasePlugin;
class VectorOutputConfig;

class VectorOutputPlugin : public virtual BasePlugin
{
    Q_OBJECT

protected:
    VectorOutputConfig *cfg;

    QString fileName;

    QLabel* pathLabel;
    QLineEdit* prefixLineEdit;

    virtual void createSettings(QGridLayout*);

public:
    VectorOutputPlugin(int _id, QString _name);

    static AbstractPlugin *create (int id, const QString &name, const Attributes &attrs) {
        Q_UNUSED (attrs);
        return new VectorOutputPlugin (id, name);
    }

    virtual void userProcess();
    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

    virtual void runStartingEvent();

public slots:
    void prefixChanged();
};

#endif // VECTOROUTPUTPLUGIN_H
