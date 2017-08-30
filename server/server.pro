#-------------------------------------------------
#
# Project created by QtCreator 2017-02-16T20:49:16
#
#-------------------------------------------------

QT       += core gui

TARGET = server
TEMPLATE = app


SOURCES += main.cpp\
        server.cpp \
    mysql.cpp

HEADERS  += server.h \
    connect.h \
    mysql.h

FORMS    += server.ui
QT       += network
QT       += sql
