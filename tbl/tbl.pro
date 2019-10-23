TEMPLATE = lib

CONFIG -= qt
CONFIG += staticlib c++17

SOURCES += \
    tbl.cpp

HEADERS += \
    exceptions.h \
    filesystem.h \
    tbl.h

INCLUDEPATH += $$PWD/../filesystem/include
