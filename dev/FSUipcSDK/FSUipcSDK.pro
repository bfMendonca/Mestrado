#-------------------------------------------------
#
# Project created by QtCreator 2017-05-21T21:48:50
#
#-------------------------------------------------

QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FSUipcSDK
TEMPLATE = app

CONFIG( debug, debug | release ) {
    DESTDIR = /release
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
    UI_DIR = release/.ui
}else {
    DESTDIR = debug
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
    UI_DIR = debug/.ui
}


# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        fsuipcwindow.cpp \
    fsinterface.cpp

HEADERS  += fsuipcwindow.h \
    fsinterface.h \
    fsuipc/FSUIPC_User.h \
    fsuipc/resource.h \
    fsuipckeyfile.h

FORMS    += fsuipcwindow.ui

LIBS += -L$$PWD/fsuipc/ -lFSUIPC_User

INCLUDEPATH += $$PWD/fsuipc
DEPENDPATH += $$PWD/fsuipc
