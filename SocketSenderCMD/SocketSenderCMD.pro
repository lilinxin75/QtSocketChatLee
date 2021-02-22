#-------------------------------------------------
#
# Project created by QtCreator 2019-01-15T14:30:41
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = SocketSenderCMD
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
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


SOURCES += main.cpp \
    client.cpp

HEADERS += \
    client.h
