#ifndef SIS3150USBUI_H
#define SIS3150USBUI_H

#include <QtGui>
#include <QWidget>
#include "sis3150control.h"

class Sis3150usbUI : public QWidget
{
    Q_OBJECT

public:
    Sis3150usbUI(Sis3150control *sis);

public slots:
    void openCloseButtonClicked();
    void outputText(QString&);

private:
    Sis3150control *sis;
    bool deviceOpen;

    void createUI();

    QWidget* createButtons();
    QWidget* createStatusView();

    QPushButton *opencloseButton;
    QPushButton *resetButton;

    QTextEdit *statusViewTextEdit;

signals:
    void deviceOpened();
    void deviceClosed();
};

#endif // SIS3150USBUI_H
