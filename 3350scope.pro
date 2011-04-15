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
    -L../sis3150_calls \
    -L../sis3100_calls \
    -l_sis3150 \
    -l_sis3100 \
    -lgsl \
    -lgslcblas \
    -lusb
INCLUDEPATH += ../sis3150_calls \
    ../sis3100_calls
SOURCES += main.cpp \
    plot2d.cpp \
    sis3350ui.cpp \
    scopemainwindow.cpp \
    modulemanager.cpp \
    dummymodule.cpp \
    scopechannel.cpp \
    dummyui.cpp \
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
    sis3100module.cpp \
    sis3100ui.cpp \
    sis3350dmx.cpp \
    dsptimefilterplugin.cpp \
    caen785module.cpp \
    pluginconnectorthreadbuffered.cpp \
    vectoroutputplugin.cpp \
    plot2dplugin.cpp \
    caen785ui.cpp \
    caenadcdmx.cpp \
    dspampspecplugin.cpp \
    caen792module.cpp \
    caen792ui.cpp \
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
    caen1290dmx.cpp \
    inttodoubleplugin.cpp \
    viewport.cpp \
    caen820module.cpp \
    caen820ui.cpp \
    caen820dmx.cpp \
    eventbuilderplugin.cpp \
    packsis3350plugin.cpp \
    interfacemanager.cpp \
    geckoremote.cpp \
    remotecontrolpanel.cpp \
    outputplugin.cpp \
    eventbuffer.cpp \
    sis3302module.cpp \
    sis3302ui.cpp \
    demuxsis3302plugin.cpp
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
    sis3100module.h \
    sis3100ui.h \
    sis3350dmx.h \
    dsptimefilterplugin.h \
    caen785module.h \
    caen_v785.h \
    pluginconnectorthreadbuffered.h \
    vectoroutputplugin.h \
    plot2dplugin.h \
    caen785ui.h \
    caenadcdmx.h \
    dspampspecplugin.h \
    caen_v792.h \
    caen792module.h \
    caen792ui.h \
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
    caen1290dmx.h \
    inttodoubleplugin.h \
    pluginconnectorqueued.h \
    confmap.h \
    viewport.h \
    caen820module.h \
    caen_v820.h \
    caen820ui.h \
    caen820dmx.h \
    eventbuilderplugin.h \
    packsis3350plugin.h \
    abstractinterface.h \
    interfacemanager.h \
    baseinterface.h \
    systeminfo.h \
    geckoremote.h \
    remotecontrolpanel.h \
    eventbuffer.h \
    outputplugin.h \
    pluginconnectorplain.h \
    samqvector.h \
    sis3302module.h \
    sis3302.h \
    sis3302ui.h \
    hexspinbox.h \
    demuxsis3302plugin.h

OTHER_FILES +=

