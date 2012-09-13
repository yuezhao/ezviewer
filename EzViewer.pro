TEMPLATE = app
TARGET = EzViewer
DEPENDPATH += .
INCLUDEPATH += .

HEADERS       = imageviewer.h \
    mainwindow.h \
    settingwidget.h \
    global.h \
    floatframe.h \
    contralbar.h
SOURCES       = imageviewer.cpp \
                main.cpp \
    mainwindow.cpp \
    settingwidget.cpp \
    floatframe.cpp \
    contralbar.cpp

RESOURCES += \
    res.qrc

win32:RC_FILE += \
    icon.rc

win32 {
VERSION = 2.0
QMAKE_TARGET_COMPANY = huangezhao
QMAKE_TARGET_DESCRIPTION = "A Simple Picture View"
QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2012 huangezhao"
QMAKE_TARGET_PRODUCT = $${TARGET}
}

TRANSLATIONS += \
    res/ImageViewer_zh_CN.ts

FORMS += \
    settingwidget.ui \
    contralbar.ui
