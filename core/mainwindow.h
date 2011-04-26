#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QSignalMapper>
#include <QSpinBox>
#include <QList>
#include <QCheckBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QTabWidget>
#include <QTimer>
#include "sis3150control.h"
#include "sis3350control.h"
#include "sis3150usbui.h"
#include "sis3350ui.h"
#include "viewport.h"
#include "postprocesswindow.h"
#include "configmanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Sis3150control *sis3150;
    Sis3350control *sis;

public slots:
    void connectDevicesTo3150();

protected:
    void createUI();
    void setupPostProcessWindow();
    void closeEvent(QCloseEvent *ev);
    ConfigManager *c;


    QTabWidget *tabs;
    QTimer *freeRunnerTimer;

    Sis3150usbUI *sis3150usbUI;
    Sis3350UI    *sis3350UI;

    PostProcessWindow *ppw;


};

#endif // MAINWINDOW_H
