#-------------------------------------------------
#
# Project created by QtCreator 2021-05-19T16:20:55
#
#-------------------------------------------------

QT       += core gui
QT      +=network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-unused-variable
QMAKE_CXXFLAGS += -Wconversion-null
QMAKE_CXXFLAGS += -std=c++0x
TARGET = P3Ktest
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    cp3kcmd.cpp \
    ctcpsocket.cpp

HEADERS  += widget.h \
    cp3kcmd.h \
    ctcpsocket.h

FORMS    += widget.ui
