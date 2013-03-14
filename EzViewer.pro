TEMPLATE = app
TARGET = EzViewer
DEPENDPATH += .
INCLUDEPATH += . libexif-port

HEADERS = global.h \
    config.h \
    imageviewer.h \
    mainwindow.h \
    settingwidget.h \
    floatframe.h \
    contralbar.h \
    fileassoc.h \
    picmanager.h \
    toolkit.h \
    imagecache.h \
    osrelated.h \
    tooltip.h \
    tools/ExifReader.h

SOURCES = main.cpp \
    imageviewer.cpp \
    mainwindow.cpp \
    settingwidget.cpp \
    floatframe.cpp \
    contralbar.cpp \
    fileassoc.cpp \
    picmanager.cpp \
    toolkit.cpp \
    imagecache.cpp \
    osrelated.cpp \
    tooltip.cpp \
    config.cpp \
    tools/ExifReader.cpp

TRANSLATIONS += \
    res/EzViewer_zh_CN.ts

RESOURCES += \
    res/res.qrc

FORMS += \
    settingwidget.ui \
    contralbar.ui

win32:RC_FILE += \
    win.rc

win32 {
    LIBS += -L$$PWD/libexif-port/out/bin -lexif
} else {
    LIBS += -lexif
}

OTHER_FILES += $$RC_FILE \
    log.txt \
    TODO.txt
