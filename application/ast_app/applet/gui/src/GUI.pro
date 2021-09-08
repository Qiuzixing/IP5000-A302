#-------------------------------------------------
#
# Project created by QtCreator 2010-06-23T19:52:34
#
#-------------------------------------------------

QT       += network

TARGET = GUI
TEMPLATE = app


#QMAKE_CXXFLAGS += -fdata-sections -ffunction-sections -Wl,--gc-sections
SOURCES += main.cpp\
        dialog.cpp

HEADERS  += dialog.h
FORMS    += dialog.ui
QTPLUGIN += qjpeg
#DEPLOYMENT_PLUGIN += qjpeg
RESOURCES += resource.qrc
CONFIG += release
#CONFIG += separate_debug_info
#LIBS += imageformats/libqjpeg.a
