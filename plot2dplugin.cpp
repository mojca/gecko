#include "plot2dplugin.h"
#include "pluginmanager.h"
#include "pluginconnectorqueued.h"

static PluginRegistrar registrar ("plot2d", Plot2DPlugin::create, AbstractPlugin::GroupPlot);

Plot2DPlugin::Plot2DPlugin(int _id, QString _name)
    : BasePlugin(_id, _name),
      msecsToTimeout(500)
{
    createSettings(settingsLayout);

    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"ch 0"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"ch 1"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"ch 2"));
    addConnector(new PluginConnectorQVDouble(this,ScopeCommon::in,"ch 3"));

    std::cout << "Instantiated Plot2DPlugin" << std::endl;

    plot->addChannel(0,tr("ch0"),std::vector<double>(1,0),
                     QColor(Qt::red),Channel::line,1);
    plot->addChannel(1,tr("ch1"),std::vector<double>(1,0),
                     QColor(Qt::green),Channel::line,1);
    plot->addChannel(2,tr("ch2"),std::vector<double>(1,0),
                     QColor(Qt::blue),Channel::line,1);
    plot->addChannel(3,tr("ch3"),std::vector<double>(1,0),
                     QColor(Qt::magenta),Channel::line,1);

    outData.resize(4);

    connect(plot,SIGNAL(histogramCleared(uint,uint)),this,SLOT(resetData(uint,uint)));

    halfSecondTimer = new QTimer();
    halfSecondTimer->start(msecsToTimeout);
    connect(halfSecondTimer,SIGNAL(timeout()),plot,SLOT(update()));
}

Plot2DPlugin::~Plot2DPlugin()
{
    plot->close();
    delete plot;
}

void Plot2DPlugin::createSettings(QGridLayout * l)
{
    // Plugin specific code here

    QWidget* container = new QWidget();
    {
        QGridLayout* cl = new QGridLayout;

        previewButton = new QPushButton(tr("Show..."));
        connect(previewButton,SIGNAL(clicked()),this,SLOT(previewButtonClicked()));
        useExternalBox = new QCheckBox(tr("Use these bounds"));
        useExternalBox->setChecked(false);
        zoomExtendsBox = new QCheckBox(tr("Zoom extends"));
        zoomExtendsBox->setChecked(false);

        plot = new plot2d(0,QSize(320,240),0);
        plot->setWindowTitle(getName());

        QLabel* xminLabel = new QLabel(tr("xmin"));
        QLabel* xmaxLabel = new QLabel(tr("xmax"));
        QLabel* yminLabel = new QLabel(tr("ymin"));
        QLabel* ymaxLabel = new QLabel(tr("ymax"));

        QLabel* updateSpeedLabel = new QLabel(tr("Update Speed"));

        updateSpeedSpinner = new QSpinBox();
        updateSpeedSpinner->setMinimum(100);
        updateSpeedSpinner->setMaximum(5000);
        updateSpeedSpinner->setSingleStep(100);
        updateSpeedSpinner->setValue(msecsToTimeout);

        xminSpinner = new QSpinBox();
        xmaxSpinner = new QSpinBox();
        yminSpinner = new QSpinBox();
        ymaxSpinner = new QSpinBox();

        xminSpinner->setMaximum(1000000);
        xminSpinner->setMinimum(-1000000);
        xmaxSpinner->setMaximum(1000000);
        yminSpinner->setMaximum(1000000);
        yminSpinner->setMinimum(-1000000);
        ymaxSpinner->setMaximum(1000000);

        xminSpinner->setValue(-1000);
        xmaxSpinner->setValue(1000);
        yminSpinner->setValue(-1000);
        ymaxSpinner->setValue(1000);

        connect(updateSpeedSpinner,SIGNAL(valueChanged(int)),this,SLOT(setTimerTimeout(int)));

        connect(xminSpinner,SIGNAL(valueChanged(int)),this,SLOT(boundsChanged()));
        connect(xmaxSpinner,SIGNAL(valueChanged(int)),this,SLOT(boundsChanged()));
        connect(yminSpinner,SIGNAL(valueChanged(int)),this,SLOT(boundsChanged()));
        connect(ymaxSpinner,SIGNAL(valueChanged(int)),this,SLOT(boundsChanged()));
        connect(useExternalBox,SIGNAL(toggled(bool)), this,SLOT(boundsChanged()));
        connect(zoomExtendsBox,SIGNAL(toggled(bool)), this,SLOT(boundsChanged()));

        cl->addWidget(previewButton,0,0,1,2);
        cl->addWidget(useExternalBox,0,2,1,1);
        cl->addWidget(zoomExtendsBox,0,3,1,1);

        cl->addWidget(xminLabel,1,0,1,1);
        cl->addWidget(xminSpinner,1,1,1,1);
        cl->addWidget(xmaxLabel,1,2,1,1);
        cl->addWidget(xmaxSpinner,1,3,1,1);

        cl->addWidget(yminLabel,2,0,1,1);
        cl->addWidget(yminSpinner,2,1,1,1);
        cl->addWidget(ymaxLabel,2,2,1,1);
        cl->addWidget(ymaxSpinner,2,3,1,1);

        cl->addWidget(updateSpeedLabel,3,0,1,2);
        cl->addWidget(updateSpeedSpinner,3,2,1,2);

        boundsChanged();

        container->setLayout(cl);
    }

    // End

    l->addWidget(container,0,0,1,1);
}

void Plot2DPlugin::userProcess()
{
    //std::cout << "Plot2DPlugin Processing" << std::endl;

    int i = 0;
    foreach(PluginConnector* input, (*inputs))
    {
        if(input->getData().canConvert< QVector<double> > ())
        {
            QVector<double> data = input->getData().value< QVector<double> > ();
            outData.at(i).assign(data.begin(),data.end());

            if(!outData.at(i).empty ()) plot->getChannelById(i)->setData(outData.at(i));
        }
        i++;
    }

    //plot->redraw();
}

void Plot2DPlugin::applySettings(QSettings* settings)
{
    QString set;
    settings->beginGroup(getName());
        set = "plotGeometry"; if(settings->contains(set)) plot->restoreGeometry(settings->value(set).toByteArray());
        set = "plotVisible";  if(settings->contains(set)) if(settings->value(set).toBool()) plot->show();
    settings->endGroup();

    // Apply UI settings
}

void Plot2DPlugin::saveSettings(QSettings* settings)
{
    if(settings == NULL)
    {
        std::cout << getName().toStdString() << ": no settings file" << std::endl;
        return;
    }
    else
    {
        std::cout << getName().toStdString() << " saving settings...";
        settings->beginGroup(getName());
            settings->setValue("plotGeometry",plot->saveGeometry());
            plot->isVisible() ? settings->setValue("plotVisible",true) : settings->setValue("plotVisible",false);
        settings->endGroup();
        std::cout << " done" << std::endl;
    }
}

void Plot2DPlugin::previewButtonClicked()
{
    if(plot->isHidden())
    {
        plot->show();
    }
    else
    {
        plot->hide();
    }
}

void Plot2DPlugin::resetData(uint ch, uint plotid)
{
    Q_UNUSED(ch);
    Q_UNUSED(plotid);
}

void Plot2DPlugin::boundsChanged()
{
    plot->setMaximumExtends(xminSpinner->value(),
                            xmaxSpinner->value(),
                            yminSpinner->value(),
                            ymaxSpinner->value());
    plot->toggleExternalBoundaries(useExternalBox->isChecked());
    plot->zoomExtends(zoomExtendsBox->isChecked());
}

void Plot2DPlugin::setTimerTimeout(int msecs)
{
    msecsToTimeout = msecs;
    halfSecondTimer->setInterval(msecsToTimeout);
}
