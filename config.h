/****************************************************************************
 * EZ Viewer
 * Copyright (C) 2013 huangezhao. CHINA.
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

#ifndef CONFIG_H
#define CONFIG_H

#include <QCoreApplication>
#include <QDir>
#include <QPointF>
#include <QSize>
#include <QSettings>
#include <QVariant>


namespace Config {

const QString ConfigFileName = "EzViewer.ini";

inline QString ConfigFilePath()
{
    return qApp->applicationDirPath() + "/" + ConfigFileName;
           //QDir::homePath()+"/Application Data/"+qApp->applicationName()+".ini"
}

const QString GeometryKey = "geometry";

const QString StartupGroup = "Startup";
const QString SizeModeKey = StartupGroup + "/SizeMode";
const QString DialogKey = StartupGroup + "/ShowDialog";

const QString EffectGroup = "Effect";
const QString AntialiasModeKey = EffectGroup + "/Antialiasing";
const QString EnableBgColorKey = EffectGroup + "/EnableBgColor";
const QString BgColorKey = EffectGroup + "/BgColor";

const QString AutoPlayGroup = "AutoPlay";
const QString TimerIntervalKey = AutoPlayGroup + "/TimerInterval";

const QString AdvancedGroup = "Advanced";
const QString EnablePreReadingKey = AdvancedGroup + "/PreReading";
const QString CacheValueKey = AdvancedGroup + "/CacheValue";

const QString FormGroup = "Form";
const QString UseTitleBarKey = FormGroup + "/UseTitleBar";


const qreal ScaleMax = 20.0;
const qreal ScaleMin = 0.1;
const QPointF OriginPoint(0.0, 0.0);
const QSize SizeAdjusted(0, 1);

const QSize WindowMinSize(280, 200);
const QSize WindowFitSize(500, 400);
const QString BgGreen = "#C7EDCC";

const int AutoScrollInterval = 25;    //20
const QDir::SortFlags DirSortFlag = QDir::Name | QDir::IgnoreCase;
const int FileSizePrecision = 2;


inline void makesureConfigFileExist()
{
    if(!QFile::exists(ConfigFilePath()))
        QFile(ConfigFilePath()).open(QIODevice::WriteOnly); // create config file
}

inline void setValue(const QString &key, const QVariant &value)
{
    QSettings settings(ConfigFilePath(), QSettings::IniFormat);
    settings.setValue(key, value);
}

inline void clearConfig()
{
    QSettings(ConfigFilePath(), QSettings::IniFormat).clear();
}

inline QVariant	value(const QString &key, const QVariant &defaultValue = QVariant())
{
    QSettings settings(ConfigFilePath(), QSettings::IniFormat);
    return settings.value(key, defaultValue);
}

}

#endif // CONFIG_H
