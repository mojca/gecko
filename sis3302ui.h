#ifndef SIS3302UI_H
#define SIS3302UI_H

#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QMap>
#include <QMessageBox>
#include <QMutex>
#include <QPushButton>
#include <QSignalMapper>
#include <QSpinBox>
#include <QStringList>
#include <QTabWidget>
#include <QTimer>
#include "viewport.h"
#include "hexspinbox.h"

#include "sis3302module.h"
#include "baseui.h"

class Sis3302Module;

class Sis3302UI : public virtual BaseUI
{
    Q_OBJECT

public:
    Sis3302UI(Sis3302Module* _module);
    ~Sis3302UI();

    void applySettings();

protected:
    Sis3302Module* module;

    QTabWidget tabs;

    QMap<QString,QWidget*> tabsMap;
    QMap<QString,QWidget*> groups;
    QMap<QString,QWidget*> widgets;
    QSignalMapper sm;

    QStringList tn; // Tab names
    QStringList gn; // Group names
    QStringList wn; // WidgetNames

    void createUI(); // Has to be implemented

    // Generic methods for ui creation
    void addTab(QString _name);
    void addGroupToTab(QString _tname, QString _name);
    void addUnnamedGroupToTab(QString _tname, QString _name);
    void addGroupToGroup(QString _tname, QString _gname, QString _name);
    void addUnnamedGroupToGroup(QString _tname, QString _gname, QString _name);

    void addButtonToGroup (QString _tname, QString _gname, QString _name, QString _cname);
    void addSpinnerToGroup (QString _tname, QString _gname, QString _name, QString _cname, int min, int max);
    void addDoubleSpinnerToGroup (QString _tname, QString _gname, QString _name, QString _cname, double min, double max);
    void addHexSpinnerToGroup (QString _tname, QString _gname, QString _name, QString _cname, int min, int max);
    void addCheckBoxToGroup (QString _tname, QString _gname, QString _name, QString _cname);
    void addPopupToGroup (QString _tname, QString _gname, QString _name, QString _cname, QStringList _itNames);

    QWidget* attachLabel(QWidget* w,QString _label);

public slots:
    void uiInput(QString _name);
    void clicked_start_button();
    void clicked_stop_button();
    void clicked_arm_button();
    void clicked_disarm_button();
    void clicked_reset_button();
    void clicked_clear_button();
};

#endif // SIS3302UI_H
