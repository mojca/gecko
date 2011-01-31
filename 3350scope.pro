# -------------------------------------------------
# Project created by QtCreator 2009-07-14T17:06:02
# -------------------------------------------------
TARGET = gecko
TEMPLATE = app
CONFIG += qt
QT += network
CONFIG += thread
QMAKE_CXXFLAGS += -march=native \
    -O3
LIBS += -g \
    -L../sis3150_calls \
    -L../sis3100_calls \
    -L../../samdsp \
    -l_sis3150 \
    -l_sis3100 \
    -lgsl \
    -lgslcblas \
    -lusb \
    -lsamdsp
INCLUDEPATH += ../sis3150_calls \
    ../sis3100_calls
SOURCES += main.cpp \
    plot2d.cpp \
    sis3350ui.cpp \
    scopemainwindow.cpp \
    modulemanager.cpp \
    dummymodule.cpp \
    scopechannel.cpp \
    baseui.cpp \
    dummyui.cpp \
    basemodule.cpp \
    sis3150ui.cpp \
    sis3150module.cpp \
    sis3350module.cpp \
    pluginmanager.cpp \
    baseplugin.cpp \
    pluginconnector.cpp \
    fileoutputplugin.cpp \
    dummyplugin.cpp \
    runthread.cpp \
    pluginthread.cpp \
    threadbuffer.cpp \
    baseinterfacemodule.cpp \
    sis3100module.cpp \
    sis3100ui.cpp \
    demuxsis3350plugin.cpp \
    dsptimefilterplugin.cpp \
    caen785module.cpp \
    pluginconnectorthreadbuffered.cpp \
    vectoroutputplugin.cpp \
    plot2dplugin.cpp \
    caen785ui.cpp \
    demuxcaenadcplugin.cpp \
    dspampspecplugin.cpp \
    caen792module.cpp \
    caen792ui.cpp \
    basedaqmodule.cpp \
    dspqdcspecplugin.cpp \
    fanoutplugin.cpp \
    dspcalfilterplugin.cpp \
    dspclippingdetectorplugin.cpp \
    dsptriggerlmaxplugin.cpp \
    dspextractsignalplugin.cpp \
    basecacheplugin.cpp \
    cachesignalplugin.cpp \
    dsppileupcorrectionplugin.cpp \
    caen1290module.cpp \
    cachehistogramplugin.cpp \
    dspkalmanbaselineplugin.cpp \
    dsppileupseparatorplugin.cpp \
    dspadcplugin.cpp \
    runmanager.cpp \
    caen1290ui.cpp \
    rawwritesis3350plugin.cpp \
    rawwritesis3350pluginV2.cpp \
    demuxcaen1290plugin.cpp \
    inttodoubleplugin.cpp \
    viewport.cpp \
    caen820module.cpp \
    caen820ui.cpp \
    demuxcaen820plugin.cpp \
    eventbuilderplugin.cpp \
    packsis3350plugin.cpp
HEADERS += sis3350.h \
    plot2d.h \
    sis3350ui.h \
    scopemainwindow.h \
    abstractmanager.h \
    modulemanager.h \
    abstractmodule.h \
    dummymodule.h \
    basemodule.h \
    scopechannel.h \
    baseui.h \
    dummyui.h \
    sis3150module.h \
    sis3150ui.h \
    sis3350module.h \
    pluginmanager.h \
    abstractplugin.h \
    baseplugin.h \
    pluginconnector.h \
    fileoutputplugin.h \
    dummyplugin.h \
    runthread.h \
    pluginthread.h \
    threadbuffer.h \
    baseinterfacemodule.h \
    sis3100module.h \
    sis3100ui.h \
    demuxsis3350plugin.h \
    dsptimefilterplugin.h \
    caen785module.h \
    caen_v785.h \
    pluginconnectorthreadbuffered.h \
    vectoroutputplugin.h \
    plot2dplugin.h \
    caen785ui.h \
    demuxcaenadcplugin.h \
    dspampspecplugin.h \
    caen_v792.h \
    caen792module.h \
    caen792ui.h \
    basedaqmodule.h \
    dspqdcspecplugin.h \
    fanoutplugin.h \
    dspcalfilterplugin.h \
    dspclippingdetectorplugin.h \
    dsptriggerlmaxplugin.h \
    dspextractsignalplugin.h \
    basecacheplugin.h \
    cachesignalplugin.h \
    dsppileupcorrectionplugin.h \
    addeditdlgs.h \
    caen1290module.h \
    caen_v1290.h \
    cachehistogramplugin.h \
    dspkalmanbaselineplugin.h \
    dsppileupseparatorplugin.h \
    dspadcplugin.h \
    runmanager.h \
    caen1290ui.h \
    rawwritesis3350plugin.h \
    rawwritesis3350pluginV2.h \
    demuxcaen1290plugin.h \
    inttodoubleplugin.h \
    pluginconnectorqueued.h \
    confmap.h \
    viewport.h \
    caen820module.h \
    caen_v820.h \
    caen820ui.h \
    demuxcaen820plugin.h \
    eventbuilderplugin.h \
    packsis3350plugin.h
