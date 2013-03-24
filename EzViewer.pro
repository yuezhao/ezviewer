TEMPLATE = app
TARGET = EzViewer

QT += svg
DEPENDPATH += . tools
INCLUDEPATH += . tools

HEADERS = global.h \
    config.h \
    imageviewer.h \
    mainwindow.h \
    imagewrapper.h \
    imagefactory.h \
    picmanager.h \
    settingwidget.h \
    floatframe.h \
    contralbar.h \
    tools/fileassoc.h \
    tools/osrelated.h \
    tools/toolkit.h \
    tools/tooltip.h \
    tools/ExifReader.h

SOURCES = main.cpp \
    config.cpp \
    imageviewer.cpp \
    mainwindow.cpp \
    imagewrapper.cpp \
    imagefactory.cpp \
    picmanager.cpp \
    settingwidget.cpp \
    floatframe.cpp \
    contralbar.cpp \
    tools/fileassoc.cpp \
    tools/osrelated.cpp \
    tools/toolkit.cpp \
    tools/tooltip.cpp \
    tools/ExifReader.cpp

TRANSLATIONS += \
    res/EzViewer_zh_CN.ts

RESOURCES += \
    res/res.qrc

FORMS += \
    settingwidget.ui \
    contralbar.ui

win32 {
    INCLUDEPATH += libexif-port
    LIBEXIF_OUT = $$PWD/libexif-port/out/bin
    LIBS += -L$$LIBEXIF_OUT -lexif

    RC_FILE += win.rc
} else {
    LIBS += -lexif
}

OTHER_FILES += $$RC_FILE \
    log.txt \
    TODO.txt
