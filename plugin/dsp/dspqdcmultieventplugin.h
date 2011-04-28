#ifndef DSPQDCMULTIEVENTPLUGIN_H
#define DSPQDCMULTIEVENTPLUGIN_H

#include <QTabWidget>

#include "baseplugin.h"
#include "geckoui.h"

class BasePlugin;

struct DspQdcMultiEventPluginConfig
{
    int width;
    int pointsForBaseline;
    int min;
    int max;
    int nofBins;
    int nofEvents;

    DspQdcMultiEventPluginConfig() : width(20),
        pointsForBaseline(10),
        min(0),
        max(100),
        nofBins(4096),
        nofEvents(1)
    {}
};

class DspQdcMultiEventPlugin : public virtual BasePlugin
{
    Q_OBJECT
public:
    DspQdcMultiEventPlugin(int _id, QString _name);
    static AbstractPlugin *create (int id, const QString &name, const Attributes &attrs) {
        Q_UNUSED (attrs);
        return new DspQdcMultiEventPlugin (id, name);
    }
    virtual void userProcess();

    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

protected:
    virtual void createSettings(QGridLayout*);

    DspQdcMultiEventPluginConfig conf;

    QVector<double> outData;

    // UI
    QTabWidget tabs;    // Tabs widget
    GeckoUiFactory uif; // UI generator factory
    QStringList tn; // Tab names
    QStringList gn; // Group names
    QStringList wn; // WidgetNames

    bool scheduleResize;

signals:

public slots:
    void uiInput(QString _name);
    void clicked_reset_button();

};

#endif // DSPQDCMULTIEVENTPLUGIN_H
