TEMPLATE = app
TARGET = EzViewer

QT += svg
DEPENDPATH += . tools ui
INCLUDEPATH += . tools ui

HEADERS = global.h \
    config.h \
    imageviewer.h \
    mainwindow.h \
    imagewrapper.h \
    imagefactory.h \
    picmanager.h \
    settingdialog.h \
    ui/contralbar.h \
    ui/commonsetting.h \
    tools/floatframe.h \
    tools/fileassoc.h \
    tools/osrelated.h \
    tools/toolkit.h \
    tools/tooltip.h \
    tools/action.h \
    tools/actionmanager.h \
    ui/shortcutsetting.h

SOURCES = main.cpp \
    config.cpp \
    imageviewer.cpp \
    mainwindow.cpp \
    imagewrapper.cpp \
    imagefactory.cpp \
    picmanager.cpp \
    settingdialog.cpp \
    ui/contralbar.cpp \
    ui/commonsetting.cpp \
    tools/floatframe.cpp \
    tools/fileassoc.cpp \
    tools/osrelated.cpp \
    tools/toolkit.cpp \
    tools/tooltip.cpp \
    tools/actionmanager.cpp \
    ui/shortcutsetting.cpp

TRANSLATIONS += res/EzViewer_zh_CN.ts

RESOURCES += res/res.qrc

FORMS += ui/contralbar.ui \
    ui/commonsetting.ui \
    ui/shortcutsetting.ui

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
