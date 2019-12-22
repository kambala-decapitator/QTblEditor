baseDir = $$PWD
include(libtarget.pri)
include(../../common/lib-static.pri)
INCLUDEPATH *= $${PWD}/..
DEFINES += TBL_STATIC
