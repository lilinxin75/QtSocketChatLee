#-------------------------------------------------
#
# Project created by QtCreator 2019-01-16T10:59:21
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = SocketReceverCMD
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
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


SOURCES += main.cpp \
    server.cpp

HEADERS += \
    server.h
