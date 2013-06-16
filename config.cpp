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
#include "actionmanager.h"
#include "qxmlputget.h"

#include <QFileSystemWatcher>
#include <QImageReader>

#define SPLIT_FUNCTION(x) &x, #x
#define SPLIT_PARAM(x) #x, x


const qreal Config::ZoomInRatioNomal = 0.1;
const qreal Config::ZoomInRatioSlow = 0.05;
const qreal Config::ZoomInRatioFast = 0.2;
const qreal Config::ZoomOutRatioNomal = -0.1;
const qreal Config::ZoomOutRatioSlow = -0.05;
const qreal Config::ZoomOutRatioFast = -0.2;
const int   Config::DefaultScrollContentSpeed = 15;

const qreal Config::ScaleMaxLimit = 20.0;
const qreal Config::ScaleMinLimit = 0.1;
const QPointF Config::OriginPoint(0.0, 0.0);
const QSize Config::SizeAdjusted(0, 1);
const QSize Config::WindowMinSize(280, 200);
const QSize Config::WindowFitSize(800, 500);
const QString Config::DefaultBgColor = "#C7EDCC";
const QDir::SortFlags Config::DefaultDirSortFlag = QDir::LocaleAware;//QDir::Name | QDir::IgnoreCase;
const int Config::ClickInterval = 1500;
const int Config::ClickThreshold = 30;
const int Config::AutoScrollInterval = 20;
const int Config::FileSizePrecision = 2;


const bool DefaultShowDialog = true;
const bool DefaultEnableBgColor = true;
const bool DefaultAutoRotateImage = true;
const int  DefaultTimerInterval = 4;
const int  TimerIntervalMinLimit = 1;
const int  TimerIntervalMaxLimit = 1000;
const int  InvalidCacheNum = -1;
const int  CacheNumMinLimit = 0;
const int  CacheNumMaxLimit = 5;


Config *Config::sInstance = NULL;


const QString ConfigFileName = "EzViewer.ini";
const QString ShortcutFileName = "shortcut.xml";

const QString GeometryKey = "geometry";
const QString StartupGroup = "Startup";
const QString SizeModeKey = StartupGroup + "/SizeMode";
const QString DialogKey = StartupGroup + "/ShowDialog";
const QString EffectGroup = "Effect";
const QString ScaleModeKey = EffectGroup + "/ScaleMode";
const QString AlignModeKey = EffectGroup + "/AlignMode";
const QString AntialiasModeKey = EffectGroup + "/Antialiasing";
const QString EnableBgColorKey = EffectGroup + "/EnableBgColor";
const QString BgColorKey = EffectGroup + "/BgColor";
const QString AutoPlayGroup = "AutoPlay";
const QString TimerIntervalKey = AutoPlayGroup + "/TimerInterval";
const QString AdvancedGroup = "Advanced";
const QString AutoRotateKey = AdvancedGroup + "/AutoRotate";
const QString EnablePreReadingKey = AdvancedGroup + "/PreReading";
const QString CacheNumKey = AdvancedGroup + "/CacheValue";
const QString FormGroup = "Form";
const QString UseTitleBarKey = FormGroup + "/UseTitleBar";

const QString RootKey = "ezviewer";
const QString ShortcutKey = "shortcut";
const QString KeySequenceKey = "keys";
const QString ActionKey = "action";


Config::Config()
    : QObject(qApp), cfgWatcher(new QFileSystemWatcher(this))
{
    initConfigValue();
    loadAllShortcut();

    ActionManager::registerFunction(tr("Setting: Enable/Disable Custom Background Color"),
                   this, SPLIT_FUNCTION(Config::changeBgColorMode));
    ActionManager::registerFunction(tr("Setting: Enable/Disable Pre-reading"),
                   this, SPLIT_FUNCTION(Config::changePreReadingMode));

    ActionManager::registerFunction(tr("Setting Scale Mode: ") + tr("Scale Large Image to Fit Window"),
                   this, &Config::changeScaleMode, SPLIT_PARAM(ScaleLargeImageToFitWidget));
    ActionManager::registerFunction(tr("Setting Scale Mode: ") + tr("Keep Image Size"),
                   this, &Config::changeScaleMode, SPLIT_PARAM(KeepImageSize));
    ActionManager::registerFunction(tr("Setting Scale Mode: ") + tr("Fit Window Width"),
                   this, &Config::changeScaleMode, SPLIT_PARAM(FitWidgetWidth));
    ActionManager::registerFunction(tr("Setting Scale Mode: ") + tr("Fit Window Height"),
                   this, &Config::changeScaleMode, SPLIT_PARAM(FitWidgetHeight));
    ActionManager::registerFunction(tr("Setting Scale Mode: ") + tr("Scale to Fit Window"),
                   this, &Config::changeScaleMode, SPLIT_PARAM(ScaleToFitWidget));
    ActionManager::registerFunction(tr("Setting Scale Mode: ") + tr("Scale to Expand Window"),
                   this, &Config::changeScaleMode, SPLIT_PARAM(ScaleToExpandWidget));

    ActionManager::registerFunction(tr("Setting Align Mode: ") + tr("Align Left Top"),
                   this, &Config::changeAlignMode, SPLIT_PARAM(AlignLeftTop));
    ActionManager::registerFunction(tr("Setting Align Mode: ") + tr("Align Left Center"),
                   this, &Config::changeAlignMode, SPLIT_PARAM(AlignLeftCenter));
    ActionManager::registerFunction(tr("Setting Align Mode: ") + tr("Align Left Bottom"),
                   this, &Config::changeAlignMode, SPLIT_PARAM(AlignLeftBottom));
    ActionManager::registerFunction(tr("Setting Align Mode: ") + tr("Align Center Top"),
                   this, &Config::changeAlignMode, SPLIT_PARAM(AlignCenterTop));
    ActionManager::registerFunction(tr("Setting Align Mode: ") + tr("Align Center"),
                   this, &Config::changeAlignMode, SPLIT_PARAM(AlignCenterCenter));
    ActionManager::registerFunction(tr("Setting Align Mode: ") + tr("Align Center Bottom"),
                   this, &Config::changeAlignMode, SPLIT_PARAM(AlignCenterBottom));
    ActionManager::registerFunction(tr("Setting Align Mode: ") + tr("Align Right Top"),
                   this, &Config::changeAlignMode, SPLIT_PARAM(AlignRightTop));
    ActionManager::registerFunction(tr("Setting Align Mode: ") + tr("Align Right Center"),
                   this, &Config::changeAlignMode, SPLIT_PARAM(AlignRightCenter));
    ActionManager::registerFunction(tr("Setting Align Mode: ") + tr("Align Right Bottom"),
                   this, &Config::changeAlignMode, SPLIT_PARAM(AlignRightBottom));

    ActionManager::registerFunction(tr("Setting Antialias Mode: ") + tr("Using When Pictures Zoom In"),
                   this, &Config::changeAntialiasMode, SPLIT_PARAM(AntialiasWhenZoomIn));
    ActionManager::registerFunction(tr("Setting Antialias Mode: ") + tr("Always Using"),
                   this, &Config::changeAntialiasMode, SPLIT_PARAM(AlwaysAntialias));
    ActionManager::registerFunction(tr("Setting Antialias Mode: ") + tr("Never Using"),
                   this, &Config::changeAntialiasMode, SPLIT_PARAM(NoAntialias));

    QList<QByteArray> list = QImageReader::supportedImageFormats();
    for(int i=0; i < list.size(); ++i)
        mFormatsList.append(list.at(i));

    mFormats = mFormatsList.join(" *.");
    mFormats.prepend("*.");

    watchConfigFile(ConfigFilePath());
    watchConfigFile(ShortcutFilePath());
}

QString Config::ConfigFilePath()
{
    return qApp->applicationDirPath() + "/" + ConfigFileName;
}

QString Config::ShortcutFilePath()
{
    return qApp->applicationDirPath() + "/" + ShortcutFileName;
}

void Config::watchConfigFile(const QString &filePath)
{
    if(!QFile::exists(filePath))
        QFile(filePath).open(QIODevice::WriteOnly); // create config file

    cfgWatcher->addPath(filePath);
    connect(cfgWatcher, SIGNAL(fileChanged(QString)), SLOT(fileChanged(QString)));
}

void Config::fileChanged(const QString &filePath)
{
    if (filePath == ConfigFilePath())
        initConfigValue();
    else if (filePath == ShortcutFilePath())
        loadAllShortcut();
}

void Config::initConfigValue()
{
    qDebug("initConfigValue");
    QSettings settings(ConfigFilePath(), QSettings::IniFormat);
    mShowDialog = settings.value(DialogKey, DefaultShowDialog).toBool();
    mScaleMode = (ScaleMode)settings.value(ScaleModeKey, DefaultScaleMode).toInt();
    mAlignMode = (AlignMode)settings.value(AlignModeKey, DefaultAlignMode).toInt();
    mAntialiasMode = (AntialiasMode)settings.value(AntialiasModeKey, DefaultAntialiasMode).toInt();
    mEnableBgColor = settings.value(EnableBgColorKey, DefaultEnableBgColor).toBool();
    QString colorStr = settings.value(BgColorKey, DefaultBgColor).toString();
    mTimerInterval = settings.value(TimerIntervalKey, DefaultTimerInterval).toInt();
    mAutoRotateImage = settings.value(AutoRotateKey, DefaultAutoRotateImage).toBool();
    mEnablePreReading = settings.value(
                EnablePreReadingKey, OSRelated::preReadingSuggested()).toBool();
    mCacheNum = settings.value(CacheNumKey, InvalidCacheNum).toInt();
    mLastGeometry = settings.value(GeometryKey).toByteArray();

    if (mScaleMode < ScaleModeBegin || mScaleMode > ScaleModeEnd)
        mScaleMode = DefaultScaleMode;
    if (mAlignMode < AlignModeBegin || mAlignMode > AlignModeEnd)
        mAlignMode = DefaultAlignMode;
    if(mAntialiasMode < AntialiasModeBegin || mAntialiasMode > AntialiasModeEnd)
        mAntialiasMode = DefaultAntialiasMode;
    mBgColor.setNamedColor(colorStr);
    if(!mBgColor.isValid())
        mBgColor.setNamedColor(DefaultBgColor);
    if(mTimerInterval < TimerIntervalMinLimit || mTimerInterval > TimerIntervalMaxLimit)
        mTimerInterval = DefaultTimerInterval;
    if(mCacheNum < CacheNumMinLimit || mCacheNum > CacheNumMaxLimit)
        mCacheNum = OSRelated::cacheSizeSuggested();

    emit configChanged();
}

void Config::restoreDefaultsConfig()
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

void Config::setScaleMode(ScaleMode mode)
{
    setValue(ScaleModeKey, mode);
}

void Config::setAlignMode(AlignMode mode)
{
    setValue(AlignModeKey, mode);
}

void Config::setAntialiasMode(AntialiasMode mode)
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

void Config::setAutoRotateImage(bool enabled)
{
    setValue(AutoRotateKey, enabled);
}

void Config::setEnablePreReading(bool enabled)
{
    setValue(EnablePreReadingKey, enabled);
}

void Config::setCacheValue(int value)
{
    setValue(CacheNumKey, value);
}

void Config::setLastGeometry(const QByteArray &geometry)
{
    setValue(GeometryKey, geometry);
}

void Config::changeScaleMode(ScaleMode mode)
{
    if (scaleMode() == mode)
        setValue(ScaleModeKey, DefaultScaleMode);
    else
        setValue(ScaleModeKey, mode);
}

void Config::changeAlignMode(AlignMode mode)
{
    if (alignMode() == mode)
        setValue(AlignModeKey, DefaultAlignMode);
    else
        setValue(AlignModeKey, mode);
}

void Config::changeAntialiasMode(AntialiasMode mode)
{
    if (antialiasMode() == mode)
        setValue(AntialiasModeKey, DefaultAntialiasMode);
    else
        setValue(AntialiasModeKey, mode);
}

void Config::addShortcut(const QString &keySequence, const QString &actionScript)
{
    if (ActionManager::bindShortcut(keySequence, actionScript))
        saveAllShortcut();
}

void Config::addShortcut(const QStringList &keySequences, const QString &actionScript)
{
    ActionManager::bindShortcut(keySequences, actionScript);
    saveAllShortcut();
}

void Config::removeShortcut(const QString &keySequence)
{
    if (ActionManager::unbindShortcut(keySequence))
        saveAllShortcut();
}

void Config::removeShortcut(const QStringList &keySequences)
{
    ActionManager::unbindShortcut(keySequences);
    saveAllShortcut();
}

void Config::saveAllShortcut()
{
    QXmlPut xmlPut(RootKey);

    QMap<QString, QString> shortcuts = ActionManager::getAllShortcut();
    QMap<QString, QString>::const_iterator it = shortcuts.constBegin();
    while (it != shortcuts.constEnd()) {
        xmlPut.descend(ShortcutKey);
        xmlPut.putString(KeySequenceKey, it.key());
        xmlPut.putString(ActionKey, it.value());
        xmlPut.rise();
        ++it;
    }

    xmlPut.save(ShortcutFilePath());
}

void Config::loadAllShortcut()
{
    ActionManager::unbindAllShortcut(); ///

    QXmlGet xmlGet;
    xmlGet.load(ShortcutFilePath());

    QString key, action;
    while (xmlGet.findNext(ShortcutKey)) {
        xmlGet.descend();
        if (xmlGet.find(KeySequenceKey))
            key = xmlGet.getString();
        if (xmlGet.find(ActionKey))
            action = xmlGet.getString();
        xmlGet.rise();

        if (!key.isEmpty() && !action.isEmpty())
            ActionManager::bindShortcut(key, action);

        key.clear();
        action.clear();
    }
}

