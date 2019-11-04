TEMPLATE = lib

include(libtarget.pri)
TARGET = $$libTarget

CONFIG -= qt
CONFIG += staticlib c++17

SOURCES += \
    tbl.cpp

HEADERS += \
    exceptions.h \
    tbl.h

include(../3rdparty/filesystem/filesystem.pri)
