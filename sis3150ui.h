#ifndef SIS3150UI_H
#define SIS3150UI_H

#include <QGroupBox>
#include <QLabel>
#include <QListView>
#include <QGridLayout>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QLineEdit>

#include "sis3150module.h"
#include "baseui.h"

class Sis3150Module;

class Sis3150UI : public virtual BaseUI
{
    Q_OBJECT

public:
    enum AddrMode {A24, A32};
    enum DataMode {D16, D32};

public:
    Sis3150UI(Sis3150Module* _module);
    ~Sis3150UI() {}
    virtual void createUI();
    virtual void applySettings() {}


private:
    void moduleOpened ();
    void moduleClosed ();

    friend class Sis3150Module;

public slots:
    void openCloseButtonClicked();
    void readButtonClicked();
    void writeButtonClicked();
    void outputText(QString);
    void addrModeChanged(int);
    void dataModeChanged(int);

signals:
    void deviceOpened();
    void deviceClosed();

private:
    QWidget* createButtons();
    QWidget* createStatusView();
    QWidget* createVmeControl();

private:
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

    AddrMode addrmode;
    DataMode datamode;

    Sis3150Module *module;
    QString name;

};

#endif // SIS3150UI_H
