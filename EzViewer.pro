TEMPLATE = app
TARGET = EzViewer

QT += svg xml
DEPENDPATH  += . tools ui 3rdparty
INCLUDEPATH += . tools ui 3rdparty

# DEFINES += TESTING_RAW
contains(DEFINES, TESTING_RAW) {
    INCLUDEPATH += ../qtimageformats/LibRaw-0.15.0-Beta4/libraw/
    LIBS += -L"../qtimageformats/LibRaw-build-desktop-Release/buildfiles/release-x86" -lraw
}

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
    ui/shortcutsetting.h \
    tools/floatframe.h \
    tools/fileassoc.h \
    tools/osrelated.h \
    tools/toolkit.h \
    tools/tooltip.h \
    tools/action.h \
    tools/actionmanager.h \
    tools/velocitytracker.h \
    3rdparty/qxmlputget.h \
    3rdparty/exif.h \
    tools/imageheader.h

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
    ui/shortcutsetting.cpp \
    tools/floatframe.cpp \
    tools/fileassoc.cpp \
    tools/osrelated.cpp \
    tools/toolkit.cpp \
    tools/tooltip.cpp \
    tools/actionmanager.cpp \
    tools/velocitytracker.cpp \
    3rdparty/qxmlputget.cpp \
    3rdparty/exif.cpp \
    tools/imageheader.cpp \
    3rdparty/jpgqguess.cpp

TRANSLATIONS += res/EzViewer_zh_CN.ts

RESOURCES += res/res.qrc

FORMS += ui/contralbar.ui \
    ui/commonsetting.ui \
    ui/shortcutsetting.ui

win32:RC_FILE += win.rc

OTHER_FILES += $$RC_FILE \
    log.txt \
    TODO.txt \
    ReadMe.txt \
    COPYING \
    README.md
