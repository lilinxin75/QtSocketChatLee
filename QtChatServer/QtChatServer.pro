#-------------------------------------------------
#
# Project created by QtCreator 2021-01-26T15:27:39
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtChatServer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG(debug, debug|release) {
    DESTDIR  = ../SocketRecever/Debug$$arch
    LIBS += -L../SocketRecever/Debug$$arch
    OBJECTS_DIR 	= ./Debug/obj
    MOC_DIR      	= ./Debug/moc
    RCC_DIR 		= ./Debug/rcc
} else {
    DESTDIR  = ../SocketRecever/Release$$arch
    LIBS += -L../SocketRecever/Release$$arch
    OBJECTS_DIR 	= ./Release/obj
    MOC_DIR       	= ./Release/moc
    RCC_DIR 		= ./Release/rcc
}


SOURCES += \
        main.cpp \
        chatserverwindow.cpp

HEADERS += \
        chatserverwindow.h

FORMS += \
        chatserverwindow.ui
