TEMPLATE = app

CONFIG -= qt
CONFIG += cmdline c++17 precompile_header

PRECOMPILED_HEADER = test.pch
PRECOMPILED_DIR = pch

SOURCES += \
    test.cpp

include(../3rdparty/catch/catch.pri)
include(../3rdparty/filesystem/filesystem.pri)
include(../tbl/shared/tbl.pri)


testDataDirName = data
testDataInDir = $$shell_quote($$shell_path($${PWD}/$${testDataDirName}))
testDataOutDir = $$shell_quote($$shell_path($${OUT_PWD}/$${testDataDirName}))

win32:isEmpty(MINGW_IN_SHELL) {
    QMAKE_POST_LINK += mklink /D $$testDataOutDir $$testDataInDir
    QMAKE_POST_LINK += & if %errorlevel%==9009 echo "mklink doesn't exist"
}
else {
    QMAKE_POST_LINK += [ -L $$testDataOutDir ] || ln -s $$testDataInDir $$testDataOutDir
}

win32-msvc*: DEFINES += _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
