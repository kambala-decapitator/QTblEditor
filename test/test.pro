TEMPLATE = app

CONFIG -= qt
CONFIG += cmdline c++17

SOURCES += \
    test.cpp

include(../3rdparty/catch/catch.pri)
include(../3rdparty/filesystem/filesystem.pri)
include(../tbl/tbl.pri)

