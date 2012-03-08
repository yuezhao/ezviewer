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

TRANSLATIONS += \
    res/ImageViewer_zh_CN.ts

FORMS += \
    settingwidget.ui \
    contralbar.ui
