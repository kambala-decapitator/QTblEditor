TEMPLATE = app

CONFIG -= qt
CONFIG += cmdline c++17 precompile_header

PRECOMPILED_HEADER = console-app.pch
PRECOMPILED_DIR = pch

SOURCES += \
  main.cpp

include(../3rdparty/args/args.pri)
include(../3rdparty/filesystem/filesystem.pri)

CONFIG(debug, debug|release) {
    include(../tbl/shared/tbl.pri)
}
else {
    include(../tbl/static/tbl.pri)
}
