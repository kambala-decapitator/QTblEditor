TEMPLATE = lib
TARGET = $$libTarget
DESTDIR = $${OUT_PWD}/../../$$libTarget

CONFIG -= qt
CONFIG += c++17 precompile_header

INCLUDEPATH *= $${PWD}
PRECOMPILED_HEADER = $${PWD}/../common/common.pch

SOURCES += \
    $${PWD}/d2color.cpp \
    $${PWD}/tbl.cpp

HEADERS += \
    $${PWD}/d2color.h \
    $${PWD}/exceptions.h \
    $${PWD}/tbl.h \
    $${PWD}/common/tbl.h

include(../3rdparty/filesystem/filesystem.pri)

CONFIG(release, debug|release): DEFINES += NDEBUG
