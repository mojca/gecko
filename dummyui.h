#ifndef DUMMYUI_H
#define DUMMYUI_H

#include <QGroupBox>
#include <QLabel>
#include <QListView>
#include <QGridLayout>
#include <QPushButton>

#include "dummymodule.h"
#include "baseui.h"

class DummyModule;

class DummyUI : public virtual BaseUI
{
    QPushButton *trigger;
    QPushButton *pollTrigger;
public:
    DummyUI(DummyModule* _module);
    ~DummyUI() {}
    virtual void createUI();
    virtual void applySettings() {};
};

#endif // DUMMYUI_H
