/****************************************************************************
 * EZ Viewer
 * Copyright (C) 2012 huangezhao. CHINA.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/

#ifndef GLOBAL_H
#define GLOBAL_H

#define INI_FILE_NAME   "EzViewer.ini"
#define VERSON_NO       "2.0"
#define BUILD_TIME      "2012/2-2012/3"
#define WEIBO           "www.weibo.com/huangezhao"
#define PROJECT_SITE    "code.google.com/p/ezviewer/"

#ifdef Q_OS_WIN32
#define INI_FILE_PATH   qApp->applicationDirPath() + "/" + INI_FILE_NAME
//QDir::homePath()+"/Application Data/"+qApp->applicationName()+".ini"
#else
#define INI_FILE_PATH   INI_FILE_NAME
#endif

class GlobalStr {
public:
    static inline QString PROJECT_NAME()
    { return QObject::tr("EzViewer"); }
    static inline QString PROJECT_AUTHOR()
    { return QObject::tr("huangezhao"); }
    static inline QString ABOUT_TEXT()
    {
        return QObject::tr("<p align='center'>%1 - A Simple Image Viewer</p>" \
                           "<p>Version: &nbsp;&nbsp; v%2"
                           "<br>Build Time: &nbsp;&nbsp; %3"  \
                           "<br>CopyRight &#169; 2012 by %4</p>" \
                           "<p>Contact Author: &nbsp;&nbsp; <a href='http://%5'>%5</a>" \
                           "<br>Project Home: &nbsp;&nbsp; <a href='http://%6'>%6</a></p>")
                .arg(PROJECT_NAME()).arg(VERSON_NO).arg(BUILD_TIME)
                .arg(PROJECT_AUTHOR()).arg(WEIBO).arg(PROJECT_SITE);
    }
};

#define QDir_SORT_FLAG QDir::Name | QDir::IgnoreCase

const qreal SCALE_MAX = 20.0;
const qreal SCALE_MIN = 0.1;
const QPointF ORIGIN_POINT(0.0, 0.0);
const QSize SIZE_ADJUST(0, 1);
const QSize MIN_SIZE(280, 160);
const QSize FIT_SIZE(500, 400);
const QString BG_GREEN("#C7EDCC");
const QString SUPPORT_FORMAT("*.jpg *.bmp *.gif *.png *.jpeg *.ico *.svg *.pbm *.pgm *.ppm *.tif *.tiff *.xbm *.xpm");
//const QStringList FORMAT_LIST(QString(SUPPORT_FORMAT).remove("*.").split(' '));

const int AUTO_SCROLL_INTERVAL = 25;//20
const int TOP_LEVEL_COUNT = 2;

const QString StartupGroup("Startup");
const QString SizeModeKey(StartupGroup + "/SizeMode");
const QString DialogKey(StartupGroup + "/ShowDialog");
const QString EffectGroup("Effect");
const QString AntialiasModeKey(EffectGroup + "/Antialiasing");
const QString EnableBgColorKey(EffectGroup + "/EnableBgColor");
const QString BgColorKey(EffectGroup + "/BgColor");
const QString AutoPlayGroup("AutoPlay");
const QString TimerIntervalKey(AutoPlayGroup + "/TimerInterval");
const QString FormGroup("Form");
const QString UseTitleBarKey(FormGroup + "/UseTitleBar");


#define SafeDelete(arg) \
    if(arg){ \
        delete arg; \
        arg = NULL; \
    }


#endif // GLOBAL_H
