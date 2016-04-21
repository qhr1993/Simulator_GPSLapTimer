#-------------------------------------------------
#
# Project created by QtCreator 2016-02-18T21:18:52
#
#-------------------------------------------------

QT       += core gui
QT      += xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fyp_timer_test
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    laptimerthread.cpp \
    kmldata.cpp \
    kmlhandler.cpp \
    kmlmaploader.cpp

HEADERS  += mainwindow.h \
    laptimerthread.h \
    kmldata.h \
    kmlhandler.h \
    kmlmaploader.h

FORMS    += mainwindow.ui
