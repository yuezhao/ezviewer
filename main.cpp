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

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QSettings>

#include "mainwindow.h"
#include "global.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString lang_country(QLocale::system().name());
    QTranslator qt_ts;
    if(qt_ts.load(QObject::tr(":/res/qt_%1").arg(lang_country)))
        app.installTranslator( &qt_ts );
    else if(qt_ts.load(QObject::tr("lang/qt_%1").arg(lang_country)))
        app.installTranslator( &qt_ts );
    QTranslator app_ts;
    if(app_ts.load(QObject::tr(":/res/ImageViewer_%1").arg(lang_country)))
        app.installTranslator( &app_ts );
    else if(app_ts.load(QObject::tr("lang/ImageViewer_%1").arg(lang_country)))
        app.installTranslator( &app_ts );

    MainWindow m;
    m.show();

    QStringList args(app.arguments());
    args.removeFirst(); // remove name of executable
    if(!args.empty()){
        m.openFiles(args);
    }else{
        QSettings settings(INI_FILE_PATH, QSettings::IniFormat);
        if(settings.value(DialogKey, true).toBool()) // show dialog while launch.
            m.openFile();
    }

    return app.exec();
}
