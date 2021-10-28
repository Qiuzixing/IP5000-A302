#-------------------------------------------------
#
# Project created by QtCreator 2010-06-23T19:52:34
#
#-------------------------------------------------

QT += core
QT += widgets
QT += gui
QT += network

TARGET = GUI
TEMPLATE = app


#QMAKE_CXXFLAGS += -fdata-sections -ffunction-sections -Wl,--gc-sections

include (common/common.pri)
include (frame/frame.pri)
include (modules/modules.pri)
include (json/json.pri)

SOURCES += \
		main.cpp \
	
FORMS    += dialog.ui
QTPLUGIN += qjpeg
#DEPLOYMENT_PLUGIN += qjpeg
RESOURCES += resource.qrc
CONFIG += release
#CONFIG += separate_debug_info
#LIBS += imageformats/libqjpeg.a
