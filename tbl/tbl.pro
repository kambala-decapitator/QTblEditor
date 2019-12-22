TEMPLATE = lib

include(libtarget.pri)
TARGET = $$libTarget

CONFIG -= qt
CONFIG += staticlib c++17 precompile_header

PRECOMPILED_HEADER = ../common/common.pch

SOURCES += \
    d2color.cpp \
    tbl.cpp

HEADERS += \
    d2color.h \
    exceptions.h \
    tbl.h

include(../3rdparty/filesystem/filesystem.pri)

CONFIG(release, debug|release): DEFINES += NDEBUG
