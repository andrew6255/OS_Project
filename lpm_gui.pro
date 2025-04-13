
QT += core gui widgets
QT += widgets charts
QT += charts


CONFIG += c++17
TARGET = lpm_gui
TEMPLATE = app
SOURCES += main.cpp \
           processmanager.cpp \
           statswindow.cpp

HEADERS += processmanager.h \
           statswindow.h
