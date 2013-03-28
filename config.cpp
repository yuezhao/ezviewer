/****************************************************************************
 * EZ Viewer
 * Copyright (C) 2013 huangezhao. CHINA.
 * Contact: huangezhao (huangezhao@gmail.com)
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

#include "config.h"
#include "osrelated.h"

#include <QFileSystemWatcher>
#include <QImageReader>

const qreal Config::ScaleMax = 20.0;
const qreal Config::ScaleMin = 0.1;
const QPointF Config::OriginPoint(0.0, 0.0);
const QSize Config::SizeAdjusted(0, 1);
const QSize Config::WindowMinSize(280, 200);
const QSize Config::WindowFitSize(500, 400);
const QString Config::BgGreen = "#C7EDCC";
const QDir::SortFlags Config::DirSortFlag = QDir::Name | QDir::IgnoreCase;

Config *Config::sInstance = NULL;


const QString ConfigFileName = "EzViewer.ini";

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


Config::Config()
    : cfgWatcher(new QFileSystemWatcher(this))
{
    initConfigValue();

    QList<QByteArray> list = QImageReader::supportedImageFormats();
    for(int i=0; i < list.size(); ++i)
        mFormatsList.append(list.at(i));

    mFormats = mFormatsList.join(" *.");
    mFormats.prepend("*.");

    watchConfigFile();
}

void Config::initConfigValue()
{
    QSettings settings(ConfigFilePath(), QSettings::IniFormat);
    mShowDialog = settings.value(DialogKey, true).toBool();
    mAntialiasMode = settings.value(AntialiasModeKey, 0).toInt();
    mEnableBgColor = settings.value(EnableBgColorKey, true).toBool();
    QString colorStr = settings.value(BgColorKey, BgGreen).toString();
    mTimerInterval = settings.value(TimerIntervalKey, 4).toInt();
    mEnablePreReading = settings.value(
                EnablePreReadingKey, OSRelated::preReadingSuggested()).toBool();
    mCacheValue = settings.value(CacheValueKey, -1).toInt();
    mLastGeometry = settings.value(GeometryKey).toByteArray();

    if(mAntialiasMode < 0 || mAntialiasMode > 2) // 3 modes
        mAntialiasMode = 0;
    mBgColor.setNamedColor(colorStr);
    if(!mBgColor.isValid())
        mBgColor.setNamedColor(BgGreen);
    if(mTimerInterval < 1 || mTimerInterval > 1000)
        mTimerInterval = 4;
    if(mCacheValue < 0 || mCacheValue > 5)
        mCacheValue = OSRelated::cacheSizeSuggested();

    emit configChanged();
}

void Config::watchConfigFile()
{
    if(!QFile::exists(ConfigFilePath()))
        QFile(ConfigFilePath()).open(QIODevice::WriteOnly); // create config file

    cfgWatcher->addPath(ConfigFilePath());
    connect(cfgWatcher, SIGNAL(fileChanged(QString)), SLOT(initConfigValue()));
}

QString Config::ConfigFilePath()
{
    return qApp->applicationDirPath() + "/" + ConfigFileName;
           //QDir::homePath()+"/Application Data/"+qApp->applicationName()+".ini"
}

void Config::clearConfig()
{
    QSettings(ConfigFilePath(), QSettings::IniFormat).clear();
}

void Config::insertConfigWatcher(const QObject *receiver, const char *method)
{
    QObject::connect(instance(), SIGNAL(configChanged()), receiver, method);
}

void Config::cancelConfigWatcher(const QObject *receiver)
{
    QObject::disconnect(instance(), 0, receiver, 0);
}

void Config::setShowDialog(bool enabled)
{
    setValue(DialogKey, enabled);
}

void Config::setAntialiasMode(int mode)
{
    setValue(AntialiasModeKey, mode);
}

void Config::setEnableBgColor(bool enabled)
{
    setValue(EnableBgColorKey, enabled);
}

void Config::setBgColor(const QColor &color)
{
    setValue(BgColorKey, color.name());
}

void Config::setTimerInterval(int interval)
{
    setValue(TimerIntervalKey, interval);
}

void Config::setEnablePreReading(bool enabled)
{
    setValue(EnablePreReadingKey, enabled);
}

void Config::setCacheValue(int value)
{
    setValue(CacheValueKey, value);
}

void Config::setLastGeometry(const QByteArray &geometry)
{
    setValue(GeometryKey, geometry);
}
