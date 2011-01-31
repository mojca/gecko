#ifndef SIS3100UI_H
#define SIS3100UI_H

#include <QGroupBox>
#include <QLabel>
#include <QListView>
#include <QGridLayout>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QLineEdit>

#include "sis3100module.h"
#include "baseui.h"

class Sis3100Module;

class Sis3100UI : public virtual BaseUI
{
    Q_OBJECT

    QWidget* createButtons();
    QWidget* createStatusView();
    QWidget* createVmeControl();

    bool deviceOpen;
    QPushButton *opencloseButton;
    QPushButton *resetButton;

    QPushButton *readButton;
    QPushButton *writeButton;

    QComboBox *addrModeSpinner;
    QComboBox *dataModeSpinner;

    QLabel *addrLabel;
    QLabel *dataLabel;

    QLineEdit *addrEdit;
    QLineEdit *dataEdit;

    QTextEdit *statusViewTextEdit;

    QWidget* vmeControl;

public:
    Sis3100UI(Sis3100Module* _module);
    ~Sis3100UI() {}
    virtual void createUI();
    virtual void applySettings() {}

private:
    void moduleOpened ();
    void moduleClosed ();

    friend class Sis3100Module;

public slots:
    void openCloseButtonClicked();
    void readButtonClicked();
    void writeButtonClicked();
    void outputText(QString);
    void modeChanged(QString);

signals:
    void deviceOpened();
    void deviceClosed();
};

#endif // SIS3100UI_H
