#ifndef CACHEHISTOGRAMPLUGIN_H
#define CACHEHISTOGRAMPLUGIN_H

#include <QWidget>
#include <vector>

#include "basecacheplugin.h"

class QComboBox;
class BasePlugin;

struct CacheHistogramPluginConfig
{
    double inputWeight;
    bool normalize;
    bool autoreset;
    bool autosave;
    double xmin, xmax;
    int nofBins, ymax;
    int autosaveInt;
    int autoresetInt;

    CacheHistogramPluginConfig()
        : inputWeight(1.), normalize(false), autoreset(false), autosave(true),
        xmin(0.),xmax(4095.),nofBins(4096),ymax(99),autosaveInt(60),autoresetInt(60)
    {}
};

class CacheHistogramPlugin : public virtual BaseCachePlugin
{
    Q_OBJECT

protected:
    std::vector<double> cache;

    double binWidth;

    CacheHistogramPluginConfig conf;
    void recalculateBinWidth();

    QDoubleSpinBox* xminSpinner;
    QDoubleSpinBox* xmaxSpinner;
    QSpinBox* ymaxSpinner;
    QComboBox* nofBinsBox;

    QCheckBox* autosaveCheck;
    QCheckBox* autoresetCheck;

    QSpinBox* autosaveSpinner;
    QSpinBox* autoresetSpinner;

    QTimer* writeToFileTimer;
    QTimer* resetTimer;

    bool writeToFile;
    int fileCount;

    virtual void createSettings(QGridLayout*);

public:
    CacheHistogramPlugin(int _id, QString _name);
    static AbstractPlugin *create (int _id, const QString &_name, const Attributes &_attrs) {
        Q_UNUSED (_attrs);
        BaseCachePlugin* bcp = new CacheHistogramPlugin (_id, _name);
        return dynamic_cast<BasePlugin*>(bcp);
    }

    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);
    virtual void userProcess();

public slots:
    void xminChanged(double);
    void xmaxChanged(double);
    void ymaxChanged(int);
    void nofBinsChanged(int);
    virtual void normalizeChanged(bool);
    virtual void inputWeightChanged(double);
    void autoresetChanged(bool);
    void autosaveChanged(bool);
    void autoresetIntChanged(int);
    void autosaveIntChanged(int);

    void scheduleWriteToFile();
    void scheduleResetHistogram();
};

#endif // CACHEHISTOGRAMPLUGIN_H
