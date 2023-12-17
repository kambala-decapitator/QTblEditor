######################################################################
# Automatically generated by qmake (2.01a) ?? ????. 6 10:32:02 2010
######################################################################

TEMPLATE = app
TARGET = QTblEditor

QT += network

greaterThan(QT_MAJOR_VERSION, 4): {
    DEFINES += IS_QT5
    IS_QT5 = 1

    QT += widgets
    *-clang*: cache()
}
greaterThan(QT_MAJOR_VERSION, 5) {
    DEFINES += IS_QT6
    QT += core5compat
}

CONFIG(release, debug|release): {
    IS_RELEASE_BUILD = 1
    DEFINES += QT_NO_DEBUG_OUTPUT \
               QT_NO_WARNING_OUTPUT
}

# Input
HEADERS += editstringcell.h \
           qtbleditor.h \
           qtcompat.h \
           tblstructure.h \
           editstringcelldialog.h \
           findreplacedialog.h \
           gotorowdialog.h \
           tablepanelwidget.h \
           d2stringtablewidget.h \
           tablesdifferenceswidget.h \
           editcolorsdialog.h \
           editorssplitterhandle.h

FORMS += editstringcell.ui \
         qtbleditor.ui \
         editstringcelldialog.ui \
         findreplacedialog.ui \
         gotorowdialog.ui \
         tablepanelwidget.ui \
         tablesdifferenceswidget.ui \
         editcolorsdialog.ui

SOURCES += editstringcell.cpp \
           main.cpp \
           qtbleditor.cpp \
           qtcompat.cpp \
           tblstructure.cpp \
           editstringcelldialog.cpp \
           findreplacedialog.cpp \
           gotorowdialog.cpp \
           tablepanelwidget.cpp \
           d2stringtablewidget.cpp \
           tablesdifferenceswidget.cpp \
           editcolorsdialog.cpp \
           colors.cpp \
           editorssplitterhandle.cpp

RESOURCES += qtbleditor.qrc

TRANSLATIONS += qtbleditor_ru.ts \
                qtbleditor_zh.ts

OTHER_FILES += TODO.txt

macx {
    ICON = File_icons/Baal.icns

    !isEmpty(IS_QT5) {
        QMAKE_MAC_SDK = macosx
    }
    else {
        *-clang* {
            QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9 # for libc++
        }
        else {
            !isEmpty(IS_RELEASE_BUILD) {
                # oldfags using apple-gcc must include PPC support
                MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
                if (!exists($$MAC_SDK)): MAC_SDK = /Developer/Xcode3.2.6/SDKs/MacOSX10.5.sdk
                QMAKE_MAC_SDK = $$MAC_SDK

                CONFIG += x86 ppc
                QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5
            }
        }
    }
}
win32 {
    RC_FILE = qtbleditor.rc
}
