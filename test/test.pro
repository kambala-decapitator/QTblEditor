TEMPLATE = app

CONFIG -= qt
CONFIG += cmdline c++17

DEPENDPATH += $$PWD/../tbl
INCLUDEPATH += \
  $$PWD/../tbl \
  $$PWD/../3rdparty/filesystem/filesystem/include \
  $$PWD/../3rdparty/catch/Catch2/include


win32:!g++ {
  libName = tbl.lib
}
else: libName = libtbl.a

win32 {
  CONFIG(debug, debug|release) {
    outLibDir = tbl/debug
  }
  else {
    outLibDir = tbl/release
  }
}
else {
  outLibDir = tbl
}

LIBS += -L$$OUT_PWD/../$$outLibDir/ -ltbl
PRE_TARGETDEPS += $$OUT_PWD/../$$outLibDir/$$libName

SOURCES += \
    test.cpp

HEADERS += \
    catch.hpp

