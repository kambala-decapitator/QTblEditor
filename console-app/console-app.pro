TEMPLATE = app

CONFIG -= qt
CONFIG += cmdline c++11

SOURCES += \
        main.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../tbl/release/ -ltbl
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../tbl/debug/ -ltbl
else:unix: LIBS += -L$$OUT_PWD/../tbl/ -ltbl

INCLUDEPATH += $$PWD/../tbl
DEPENDPATH += $$PWD/../tbl

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../tbl/release/libtbl.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../tbl/debug/libtbl.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../tbl/release/tbl.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../tbl/debug/tbl.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../tbl/libtbl.a
