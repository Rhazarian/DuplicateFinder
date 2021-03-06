#-------------------------------------------------
#
# Project created by QtCreator 2018-10-24T02:05:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DuplicateFinder
TEMPLATE = app
CONFIG += c++17
message($$QMAKESPEC)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        main_window.cpp \
    popup_window.cpp \
    error_popup_window.cpp \
    delete_popup_window.cpp

HEADERS += \
        main_window.h \
    popup_window.h \
    error_popup_window.h \
    delete_popup_window.h

FORMS += \
        main_window.ui \
    popup_window.ui \
    error_popup_window.ui \
    delete_popup_window.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
