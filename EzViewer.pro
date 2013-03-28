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
    tools/tooltip.h

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
    tools/tooltip.cpp

TRANSLATIONS += res/EzViewer_zh_CN.ts

RESOURCES += res/res.qrc

FORMS += contralbar.ui \
    settingwidget.ui

win32:RC_FILE += win.rc

OTHER_FILES += $$RC_FILE \
    log.txt \
    TODO.txt



# if you don't need reading exif infornation, append '#' at next line to cancel it.
DEFINES += USE_EXIF
LIBEXIF_OUT = $$PWD/libexif-port/out/bin

contains(DEFINES, USE_EXIF) {
    HEADERS += tools/ExifReader.h
    SOURCES += tools/ExifReader.cpp

    win32 {
        INCLUDEPATH += libexif-port
        LIBS += -L$$LIBEXIF_OUT -lexif
    } else {
        LIBS += -lexif
    }
}
