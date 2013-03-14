/****************************************************************************
 * EZ Viewer
 * Copyright (C) 2012 huangezhao. CHINA.
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

#include "picmanager.h"

#include "config.h"
#include "osrelated.h"
#include "toolkit.h"

#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QFile>


PicManager::PicManager(QWidget *parent)
    : ImageViewer(parent), curCache(ImageCache::getNullCache()),
      listMode(FileNameListMode), currentIndex(-1), fsWatcher(this)
{
//    state = NoFileNoPicture;
    connect(&fsWatcher, SIGNAL(directoryChanged(QString)),
            SLOT(directoryChanged()));
    connect(&fsWatcher, SIGNAL(fileChanged(QString)),
            SLOT(fileChanged(QString)));

    preReadingTimer.setSingleShot(true);
    connect(&preReadingTimer, SIGNAL(timeout()), SLOT(preReadingNextPic()));
}

PicManager::~PicManager()
{
    curCache->freeCache();
}

void PicManager::updateFileNameList(const QString &curfile)
{
    QFileInfo fileInfo(curfile);
    curDir = fileInfo.absolutePath();
    if(!curDir.endsWith('/'))
        curDir.append('/');

    list = QDir(curDir, Config::supportFormats(), Config::DirSortFlag, QDir::Files)
            .entryList();
    currentIndex = list.indexOf(fileInfo.fileName());
}

void PicManager::updateFullPathList(const QString &file)
{
    int index = list.indexOf(file);
    //! 如果文件名已经过滤并且简化，则可以不用验证index != -1 ??
    if(!QFile::exists(file) && index != -1)
        list.removeAt(index);
    currentIndex = list.indexOf(file);
}

void PicManager::updateFileIndex(int oldIndex)
{
    if(currentIndex != -1) //! verify if currentIndex is valid.
        return;

    if(list.isEmpty()){
        noFileToShow();
        return;
    }

    if(oldIndex >= list.size())
        currentIndex = list.size() - 1;
    else if(oldIndex != -1)
        currentIndex = oldIndex;
    else
        currentIndex = 0;

    readFile(currentIndex);
//    QTimer::singleShot(0, this, SLOT(preReadingNextPic()));
    preReadingNextPic();    ///
}

void PicManager::directoryChanged()
{
    int oldIndex = currentIndex;
    updateFileNameList(curPath);
    updateFileIndex(oldIndex);
}

void PicManager::fileChanged(const QString &filePath)
{
    int oldIndex = currentIndex;
    updateFullPathList(filePath);
    updateFileIndex(oldIndex);
}

QString PicManager::getPathAtIndex(int index) const
{
    QString file = list.at(index);
    bool isAbsolute = file.contains('/') || file.contains('\\'); ///
    return isAbsolute ? file : curDir + file;
}

void PicManager::readFile(const QString &fullPath)
{
    preReadingTimer.stop(); // 如果预读还未开始则取消

    QFileInfo fileInfo(fullPath);
    if(curPath == fileInfo.absoluteFilePath() )//! if the image needs refresh?
        return;

    curPath = fileInfo.absoluteFilePath();
    curName = fileInfo.fileName();

    if(curCache->movie){
        curCache->movie->stop();
        disconnect(curCache->movie, SIGNAL(updated(QRect)));
    }

    curCache = ImageCache::getCache(curPath);

    if(curCache->movie){
        if(curCache->movie->state() == QMovie::NotRunning)
            curCache->movie->start();
        connect(curCache->movie, SIGNAL(updated(QRect)), SLOT(updateGifImage()));
    }

    QString msg = curCache->image.isNull() ? tr("Cannot load picture:\n'%1'.")
                                      .arg(curPath) : QString::null;
    loadImage(curCache->image, msg);
//     state = image.isNull() ? FileNoPicture : FilePicture;
    emit fileNameChange(curName);
}

void PicManager::noFileToShow()
{
    if(curCache->movie){
        curCache->movie->stop();
        disconnect(curCache->movie, SIGNAL(updated(QRect)));
    }
    curCache = ImageCache::getNullCache();
    curPath = curName = QString::null;
//    state = NoFileNoPicture;
    loadImage(curCache->image);
    emit fileNameChange("");    //
}

void PicManager::openFile(const QString &file)
{
//    if(!QFileInfo(file).isFile()) return;

    QStringList oldWatchList(fsWatcher.files() + fsWatcher.directories());
    if(!oldWatchList.empty())
        fsWatcher.removePaths(oldWatchList);

    listMode = FileNameListMode;
    updateFileNameList(file);
    // make sure if file is no a picture, this will show error message.
    readFile(file); //readFile(list.at(currentIndex));
//    QTimer::singleShot(1000, this, SLOT(preReadingNextPic()));
//    preReadingNextPic(); ///
    preReadingTimer.start(2000);

    fsWatcher.addPath(curDir);
    if(!QFileInfo(curDir).isRoot())// watch the parent dir, will get notice when rename current dir.
        fsWatcher.addPath(QFileInfo(curDir).absolutePath());
}

void PicManager::openFiles(const QStringList &fileList)
{
    //! ?? check if is file and if is exist, remove from list if no file.
    if(fileList.empty()) return;
    if(fileList.size() == 1){
        openFile(fileList.first());
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QStringList oldWatchList(fsWatcher.files() + fsWatcher.directories());
    if(!oldWatchList.empty())
        fsWatcher.removePaths(oldWatchList);

    listMode = FullPathListMode;
    curDir = "";
    list = fileList;
    currentIndex = 0;
    readFile(currentIndex);
//    QTimer::singleShot(0, this, SLOT(preReadingNextPic()));
//    preReadingNextPic(); ///
    preReadingTimer.start(2000);

    fsWatcher.addPaths(list);       //放到另一个线程中？？？

    QApplication::restoreOverrideCursor();
}

bool PicManager::prePic()
{
    // maybe current file is not a picture, and current dir has no any picture also, so we need check if(list.size() < 1).
    if(!hasFile() || list.size() < 1) return false;

    currentIndex = getPreIndex(currentIndex);
    readFile(currentIndex);

    preReadingPrePic();
    return true;
}

bool PicManager::nextPic()
{
    if(!hasFile() || list.size() < 1) return false; //

    currentIndex = getNextIndex(currentIndex);
    readFile(currentIndex);

    preReadingNextPic();
    return true;
}

void PicManager::switchGifPause()
{
    QMovie * &movie = curCache->movie;
    if(movie)
        switch(movie->state()){
        case QMovie::Running:
            movie->setPaused(true);
            break;
        case QMovie::Paused:
            movie->setPaused(false);
            break;
        case QMovie::NotRunning:
            break;
        }
}

void PicManager::nextGifFrame()
{
    QMovie * &movie = curCache->movie;
    if(movie)
        switch(movie->state()){
        case QMovie::Running:
            movie->setPaused(true);
            break;
        case QMovie::Paused:
            movie->jumpToNextFrame();
            break;
        case QMovie::NotRunning:
            break;
        }
}

void PicManager::setGifPaused(bool paused)
{
    if(curCache->movie)
        curCache->movie->setPaused(paused);
}

void PicManager::updateGifImage()
{
    updatePixmap(curCache->movie->currentImage());
}

void PicManager::hideEvent ( QHideEvent * event )
{
    setGifPaused(true);
}

void PicManager::showEvent ( QShowEvent * event )
{
    setGifPaused(false);
}

void PicManager::deleteFile(bool needAsk)
{
    if(!hasFile() || !QFile::exists(curPath)) return;

    if(needAsk){
        int ret = QMessageBox::question(
                    0, tr("Delete File"),
                    tr("Are you sure to delete file '%1'?").arg(curName),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        if(ret == QMessageBox::No)
            return;
    }

    if(curCache->movie) SafeDelete(curCache->movie); //! gif image: must free movie before delete file.
    curCache->cacheHasChanged();

    OSRelated::moveFile2Trash(curPath); ///
}

QString PicManager::attribute() const
{
    QString info;
    QFileInfo fileInfo(curPath);

    if(fileInfo.exists()){
        const QString timeFormat(tr("yyyy-MM-dd, hh:mm:ss"));
        qint64 size = fileInfo.size();

        info += tr("File Name: %1").arg(curName);
        info += "<br>" + tr("File Size: %1 (%2 Bytes)").arg(ToolKit::fileSize2Str(size)).arg(size);
        info += "<br>" + tr("Created Time: %1")
                .arg(fileInfo.created().toString(timeFormat));
    }

    if(!currentImage().isNull()){
        if(!info.isEmpty())
            info += QString("<br>");

        if(!curCache->format.isEmpty())
            info += tr("Image Format: %1").arg(curCache->format);
        if(currentImage().colorCount() > 0)
            info += "<br>" + tr("Color Count: %1").arg(currentImage().colorCount());
        else if(currentImage().depth() >= 16)
            info += "<br>" + tr("Color Count: True color");
        info += "<br>" + tr("Depth: %1").arg(currentImage().depth());
    //    info += "<br>" + tr("BitPlaneCount: %1").arg(image.bitPlaneCount());//the color counts actual used, <= Depth

        int gcd = ToolKit::gcd(currentImage().width(), currentImage().height());
        QString ratioStr = (gcd == 0) ? "1:1" : QString("%1:%2")
                                        .arg(currentImage().width() / gcd)
                                        .arg(currentImage().height() / gcd);


        info += "<br>" + tr("Size: %1 x %2 (%3)")
                .arg(currentImage().width())
                .arg(currentImage().height())
                .arg(ratioStr);
        if(fileInfo.exists() && curCache->frameCount != 1)
            info += "<br>" + tr("Frame Count: %1").arg(curCache->frameCount);
//        info += "<br>" + tr("Current Scale: %1%").arg(currentScale() * 100, 0, 'g', 4);
    }

    return info;
}
