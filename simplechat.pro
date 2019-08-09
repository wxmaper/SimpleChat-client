QT += core gui widgets websockets

TARGET = simplechat
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += c++11
SOURCES += main.cpp widget.cpp authdialog.cpp
HEADERS += widget.h authdialog.h
FORMS += widget.ui authdialog.ui

RESOURCES += icons.qrc
