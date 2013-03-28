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

#ifndef CONFIG_H
#define CONFIG_H

#include <QCoreApplication>
#include <QColor>
#include <QDir>
#include <QPointF>
#include <QSize>
#include <QSettings>
#include <QVariant>


class QFileSystemWatcher;

class Config : public QObject
{
    Q_OBJECT
public:
signals:
    void configChanged();

public:
    const static qreal ScaleMax;
    const static qreal ScaleMin;
    const static QPointF OriginPoint;
    const static QSize SizeAdjusted;

    const static QSize WindowMinSize;
    const static QSize WindowFitSize;
    const static QString BgGreen;

    const static int AutoScrollInterval = 25;    //20
    const static int FileSizePrecision = 2;
    const static QDir::SortFlags DirSortFlag;


    /**
     * if config has been changed, the @method of @receiver will be invoked.
     */
    static void insertConfigWatcher(const QObject *receiver, const char *method);
    static void cancelConfigWatcher(const QObject *receiver);

    static void clearConfig();

    static QString supportFormats() { return instance()->mFormats; }
    static QStringList formatsList() { return instance()->mFormatsList; }


    static bool showDialog()    { return instance()->mShowDialog; }
    static int  antialiasMode() { return instance()->mAntialiasMode; }
    static bool enableBgColor() { return instance()->mEnableBgColor; }
    static QColor bgColor()     { return instance()->mBgColor; }
    static int  timerInterval() { return instance()->mTimerInterval; }
    static bool enablePreReading() { return instance()->mEnablePreReading; }
    static int  cacheValue()    { return instance()->mCacheValue; }
    static QByteArray lastGeometry(){ return instance()->mLastGeometry; }

    static void setShowDialog(bool enabled);
    static void setAntialiasMode(int mode);
    static void setEnableBgColor(bool enabled);
    static void setBgColor(const QColor &color);
    static void setTimerInterval(int interval);
    static void setEnablePreReading(bool enabled);
    static void setCacheValue(int value);
    static void setLastGeometry(const QByteArray &geometry);

private slots:
    void initConfigValue();

private:
    static QString ConfigFilePath();
    static void setValue(const QString &key, const QVariant &value);

    static Config *instance();


    Config();
    void watchConfigFile();

    static Config *sInstance;

    QFileSystemWatcher *cfgWatcher;

    bool mShowDialog;
    int mAntialiasMode;
    bool mEnableBgColor;
    QColor mBgColor;
    int mTimerInterval;
    bool mEnablePreReading;
    int mCacheValue;
    QByteArray mLastGeometry;

    QStringList mFormatsList;
    QString mFormats;
};



inline Config *Config::instance()
{
    if(!sInstance)
        sInstance = new Config();
    return sInstance;
}

inline void Config::setValue(const QString &key, const QVariant &value)
{
    QSettings settings(ConfigFilePath(), QSettings::IniFormat);
    settings.setValue(key, value);
}

#endif // CONFIG_H
