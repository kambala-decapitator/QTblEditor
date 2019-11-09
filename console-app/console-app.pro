TEMPLATE = app

CONFIG -= qt
CONFIG += cmdline c++17

SOURCES += \
  main.cpp

include(../3rdparty/args/args.pri)
include(../3rdparty/filesystem/filesystem.pri)
include(../tbl/tbl.pri)
