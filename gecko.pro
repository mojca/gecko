# -------------------------------------------------
# Project created by QtCreator 2009-07-14T17:06:02
# -------------------------------------------------
TARGET = gecko
TEMPLATE = app
CONFIG += qt
QT += network
CONFIG += thread
QMAKE_CXXFLAGS_RELEASE += -march=native \
    -O3
LIBS += -g \
    -L$$PWD/lib/sis3150_calls \
    -L$$PWD/lib/sis3100_calls \
    -l_sis3150 \
    -l_sis3100 \
    -lgsl \
    -lgslcblas \
    -lusb
INCLUDEPATH += include \
    lib/sis3150_calls \
    lib/sis3100_calls
SOURCES += core/baseplugin.cpp \
    core/eventbuffer.cpp \
    core/geckoremote.cpp \
    core/interfacemanager.cpp \
    core/main.cpp \
    core/modulemanager.cpp \
    core/outputplugin.cpp \
    core/plot2d.cpp \
    core/pluginconnector.cpp \
    core/pluginconnectorthreadbuffered.cpp \
    core/pluginmanager.cpp \
    core/pluginthread.cpp \
    core/remotecontrolpanel.cpp \
    core/runmanager.cpp \
    core/runthread.cpp \
    core/scopechannel.cpp \
    core/scopemainwindow.cpp \
    core/threadbuffer.cpp \
    core/viewport.cpp \
    interface/sis3100module.cpp \
    interface/sis3100ui.cpp \
    interface/sis3150module.cpp \
    interface/sis3150ui.cpp \
    module/caen1290dmx.cpp \
    module/caen1290module.cpp \
    module/caen1290ui.cpp \
    module/caen785module.cpp \
    module/caen785ui.cpp \
    module/caen792module.cpp \
    module/caen792ui.cpp \
    module/caen820module.cpp \
    module/caen820dmx.cpp \
    module/caen820ui.cpp \
    module/caenadcdmx.cpp \
    module/dummymodule.cpp \
    module/dummyui.cpp \
    module/sis3302dmx.cpp \
    module/sis3302module.cpp \
    module/sis3302ui.cpp \
    module/sis3350dmx.cpp \
    module/sis3350module.cpp \
    module/sis3350ui.cpp \
    plugin/aux/dummyplugin.cpp \
    plugin/aux/fanoutplugin.cpp \
    plugin/aux/inttodoubleplugin.cpp \
    plugin/cache/basecacheplugin.cpp \
    plugin/cache/cachehistogramplugin.cpp \
    plugin/cache/cachesignalplugin.cpp \
    plugin/dsp/dspadcplugin.cpp \
    plugin/dsp/dspampspecplugin.cpp \
    plugin/dsp/dspcalfilterplugin.cpp \
    plugin/dsp/dspcfdplugin.cpp \
    plugin/dsp/dspclippingdetectorplugin.cpp \
    plugin/dsp/dspextractsignalplugin.cpp \
    plugin/dsp/dspkalmanbaselineplugin.cpp \
    plugin/dsp/dsppileupcorrectionplugin.cpp \
    plugin/dsp/dsppileupseparatorplugin.cpp \
    plugin/dsp/dspqdcspecplugin.cpp \
    plugin/dsp/dsptimefilterplugin.cpp \
    plugin/dsp/dsptriggerlmaxplugin.cpp \
    plugin/output/fileoutputplugin.cpp \
    plugin/output/rawwritesis3350plugin.cpp \
    plugin/output/rawwritesis3350pluginV2.cpp \
    plugin/output/vectoroutputplugin.cpp \
    plugin/pack/eventbuilderplugin.cpp \
    plugin/pack/packsis3350plugin.cpp \
    plugin/plot/plot2dplugin.cpp

HEADERS += core/addeditdlgs.h \
    core/geckoremote.h \
    core/pluginthread.h \
    core/remotecontrolpanel.h \
    core/runthread.h \
    core/scopemainwindow.h \
    core/systeminfo.h \
    include/abstractinterface.h \
    include/abstractmanager.h \
    include/abstractmodule.h \
    include/abstractplugin.h \
    include/baseinterface.h \
    include/basemodule.h \
    include/baseplugin.h \
    include/baseui.h \
    include/confmap.h \
    include/eventbuffer.h \
    include/hexspinbox.h \
    include/interfacemanager.h \
    include/modulemanager.h \
    include/outputplugin.h \
    include/plot2d.h \
    include/pluginconnector.h \
    include/pluginconnectorplain.h \
    include/pluginconnectorqueued.h \
    include/pluginconnectorthreadbuffered.h \
    include/pluginmanager.h \
    include/runmanager.h \
    include/samqvector.h \
    include/scopechannel.h \
    include/threadbuffer.h \
    include/viewport.h \
    interface/sis3100module.h \
    interface/sis3100ui.h \
    interface/sis3150module.h \
    interface/sis3150ui.h \
    module/caen1290dmx.h \
    module/caen1290module.h \
    module/caen1290ui.h \
    module/caen785module.h \
    module/caen785ui.h \
    module/caen792module.h \
    module/caen792ui.h \
    module/caen820dmx.h \
    module/caen820module.h \
    module/caen820ui.h \
    module/caenadcdmx.h \
    module/caen_v1290.h \
    module/caen_v785.h \
    module/caen_v792.h \
    module/caen_v820.h \
    module/dummymodule.h \
    module/dummyui.h \
    module/sis3302dmx.h \
    module/sis3302.h \
    module/sis3302module.h \
    module/sis3302ui.h \
    module/sis3350dmx.h \
    module/sis3350.h \
    module/sis3350module.h \
    module/sis3350ui.h \
    plugin/aux/dummyplugin.h \
    plugin/aux/fanoutplugin.h \
    plugin/aux/inttodoubleplugin.h \
    plugin/cache/basecacheplugin.h \
    plugin/cache/cachehistogramplugin.h \
    plugin/cache/cachesignalplugin.h \
    plugin/dsp/dspadcplugin.h \
    plugin/dsp/dspampspecplugin.h \
    plugin/dsp/dspcalfilterplugin.h \
    plugin/dsp/dspcfdplugin.h \
    plugin/dsp/dspclippingdetectorplugin.h \
    plugin/dsp/dspextractsignalplugin.h \
    plugin/dsp/dspkalmanbaselineplugin.h \
    plugin/dsp/dsppileupcorrectionplugin.h \
    plugin/dsp/dsppileupseparatorplugin.h \
    plugin/dsp/dspqdcspecplugin.h \
    plugin/dsp/dsptimefilterplugin.h \
    plugin/dsp/dsptriggerlmaxplugin.h \
    plugin/output/fileoutputplugin.h \
    plugin/output/rawwritesis3350plugin.h \
    plugin/output/rawwritesis3350pluginV2.h \
    plugin/output/vectoroutputplugin.h \
    plugin/pack/eventbuilderplugin.h \
    plugin/pack/packsis3350plugin.h \
    plugin/plot/plot2dplugin.h


OTHER_FILES +=

