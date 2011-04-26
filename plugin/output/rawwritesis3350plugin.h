#ifndef RAWWRITESIS3350PLUGIN_H
#define RAWWRITESIS3350PLUGIN_H

#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <iostream>
#include <vector>

#include "baseplugin.h"

class BasePlugin;

class RawWriteSis3350Plugin : public BasePlugin
{
    Q_OBJECT

protected:
    QString filePath;
    QString fileName;
    QString prefix;

    QLineEdit* filePathLineEdit;
    QPushButton* filePathButton;

    virtual void createSettings(QGridLayout*);

public:
    RawWriteSis3350Plugin(int _id, QString _name);

    static AbstractPlugin *create (int id, const QString &name, const Attributes &attrs) {
        Q_UNUSED (attrs);
        return new RawWriteSis3350Plugin (id, name);
    }

    QString getFilePath() { return filePath; }

    void setFilePath(QString _filePath);

    virtual void userProcess();
    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
    void filePathButtonClicked();
};

#endif // RAWWRITESIS3350PLUGIN_H
