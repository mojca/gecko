#ifndef RAWWRITESIS3350PLUGINV2_H
#define RAWWRITESIS3350PLUGINV2_H

#include <QCheckBox>
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

class RawWriteSis3350PluginV2 : public BasePlugin
{
    Q_OBJECT

protected:
    QString filePath;
    QString fileName;
    QString prefix;

    QLineEdit* filePathLineEdit;
    QPushButton* filePathButton;

    QCheckBox* fileSaveCheck[4];

    virtual void createSettings(QGridLayout*);

public:
    RawWriteSis3350PluginV2(int _id, QString _name);

    static BasePlugin *create (int id, const QString &name, const Attributes &attrs) {
        Q_UNUSED (attrs);
        return new RawWriteSis3350PluginV2 (id, name);
    }

    QString getFilePath() { return filePath; }

    void setFilePath(QString _filePath);

    virtual void userProcess();
    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
    void filePathButtonClicked();
    void fileNameEditChanged();
    void fileSaveCheckToggled(int);
    void reset();
    void updateChMask();

private:
    bool saveEnabled[4];
    uint64_t bytesWritten;
    uint32_t cycles;
    uint32_t cycleSkip;
    uint32_t chMask;
    uint8_t nofEnabledChannels;
    uint32_t fileNo;
};

#endif // RAWWRITESIS3350PLUGINV2_H
