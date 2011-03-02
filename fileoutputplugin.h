#ifndef FILEOUTPUTPLUGIN_H
#define FILEOUTPUTPLUGIN_H


#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QFile>
#include <QFileDialog>
#include <QDateTime>
#include <iostream>
#include <vector>

#include "baseplugin.h"

class BasePlugin;

class FileOutputPlugin : public virtual BasePlugin
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
    FileOutputPlugin(int _id, QString _name);
    static AbstractPlugin *create (int _id, const QString &_name, const Attributes &_attrs) {
        Q_UNUSED (_attrs);
        return new FileOutputPlugin (_id, _name);
    }

    QString getFilePath() { return filePath; }

    void setFilePath(QString _filePath);

    virtual void userProcess();

    virtual void applySettings(QSettings*) {}
    virtual void saveSettings(QSettings*) {}

public slots:
    void filePathButtonClicked();
};

#endif // FILEOUTPUTPLUGIN_H
