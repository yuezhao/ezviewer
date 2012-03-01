TEMPLATE = app
TARGET = EzViewer
DEPENDPATH += .
INCLUDEPATH += .

#win32 {
#    INCLUDEPATH  += $$quote(C:/MinGW/msys/1.0/local/include)
#    LIBS         += $$quote(C:/MinGW/msys/1.0/local/lib/libexiv2.dll.a)
#}

#unix {
#    INCLUDEPATH  += /usr/local/include
#    LIBS         += -L/usr/local/lib -lexiv2
#}

HEADERS       = imageviewer.h \
    mainwindow.h \
    settingwidget.h \
    global.h \
    floatmanager.h
SOURCES       = imageviewer.cpp \
                main.cpp \
    mainwindow.cpp \
    settingwidget.cpp \
    floatmanager.cpp

RESOURCES += \
    res.qrc

win32:RC_FILE += \
    icon.rc

TRANSLATIONS += \
    res/ImageViewer_zh_CN.ts

FORMS += \
    settingwidget.ui
