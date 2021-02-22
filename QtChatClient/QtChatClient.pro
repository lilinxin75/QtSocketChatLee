#-------------------------------------------------
#
# Project created by QtCreator 2021-01-26T17:16:21
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtChatClient
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG(debug, debug|release) {
    DESTDIR  = ../SocketSender/Debug$$arch
    LIBS += -L../SocketSender/Debug$$arch
    OBJECTS_DIR 	= ./Debug/obj
    MOC_DIR      	= ./Debug/moc
    RCC_DIR 		= ./Debug/rcc
} else {
    DESTDIR  = ../SocketSender/Release$$arch
    LIBS += -L../SocketSender/Release$$arch
    OBJECTS_DIR 	= ./Release/obj
    MOC_DIR       	= ./Release/moc
    RCC_DIR 		= ./Release/rcc
}


SOURCES += \
        main.cpp \
        chatclientwindow.cpp

HEADERS += \
        chatclientwindow.h

FORMS += \
        chatclientwindow.ui
