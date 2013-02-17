TEMPLATE = app
TARGET = EzViewer
DEPENDPATH += .
INCLUDEPATH += .

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
    tooltip.h

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
    tooltip.cpp

TRANSLATIONS += \
    res/EzViewer_zh_CN.ts

FORMS += \
    settingwidget.ui \
    contralbar.ui

RESOURCES += \
    res/res.qrc

win32:RC_FILE += \
    win.rc

OTHER_FILES += $$RC_FILE \
    log.txt \
    TODO.txt
