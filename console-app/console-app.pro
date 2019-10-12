TEMPLATE = app

CONFIG -= qt
CONFIG += cmdline c++17

SOURCES += \
  main.cpp

DEPENDPATH += $$PWD/../tbl
INCLUDEPATH += \
  $$PWD/../tbl \
  $$PWD/../filesystem/include


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

