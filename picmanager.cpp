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
#include "global.h"
#include "osrelated.h"
#include "tooltip.h"

#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QFile>


PicManager::PicManager(QWidget *parent)
    : ImageViewer(parent), curImage(ImageFactory::getImageWrapper(QString::null)),
      listMode(FileNameListMode), currentIndex(-1), fsWatcher(this)
{
//    state = NoFileNoPicture;
    connect(&fsWatcher, SIGNAL(directoryChanged(QString)),
            SLOT(directoryChanged()));
    connect(&fsWatcher, SIGNAL(fileChanged(QString)),
            SLOT(fileChanged(QString)));
}

PicManager::~PicManager()
{
    ImageFactory::freeAllCache();
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
    //! If file name has been filtered and simplified, does index != -1 needed??
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
    QFileInfo fileInfo(fullPath);
    if(curPath == fileInfo.absoluteFilePath() )//! if the image needs refresh?
        return;

    curPath = fileInfo.absoluteFilePath();
    curName = fileInfo.fileName();

    curImage->recycle();
    curImage = ImageFactory::getImageWrapper(curPath);

    while(!curImage->getReady()){
        qApp->processEvents(QEventLoop::AllEvents);
        // wait for pre-reading in another thread
    }

    if(curImage->isAnimation()) {
        connect(curImage, SIGNAL(animationUpdated()), SLOT(updateAnimation()));
        curImage->startAnimation();
    } else if (curImage->frameCount() > 1){ // like ico format
        connect(curImage, SIGNAL(frameUpdated()), SLOT(updateImage()));
    }

    QImage image = curImage->currentImage();
    QString errorMsg = image.isNull() ? Global::LoadFileErrorInfo().arg(curPath)
                                 : QString::null;
    loadImage(image, errorMsg);
//     state = image.isNull() ? FileNoPicture : FilePicture;
    emit imageChanged(curName);
}

void PicManager::noFileToShow()
{
    curImage->recycle();
    curImage = ImageFactory::getImageWrapper(QString::null);
    curPath = curName = QString::null;
//    state = NoFileNoPicture;
    loadImage(QImage());
    emit imageChanged(QString::null);    //
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
    preReadingNextPic();

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
    preReadingNextPic();

    fsWatcher.addPaths(list);       // run in background thread ?

    QApplication::restoreOverrideCursor();
}

bool PicManager::prePic()
{
    // maybe current file is not a picture, and current dir has no any picture also, so we need check if(list.size() < 1).
    if(!hasFile() || list.size() < 1) return false;
    if (!curImage->getReady()) return false;

    currentIndex = getPreIndex(currentIndex);
    readFile(currentIndex);

    preReadingPrePic();
    return true;
}

bool PicManager::nextPic()
{
    if(!hasFile() || list.size() < 1) return false; //
    if (!curImage->getReady()) return false;

    currentIndex = getNextIndex(currentIndex);
    readFile(currentIndex);

    preReadingNextPic();
    return true;
}

void PicManager::updateAnimation()
{
    updatePixmap(curImage->currentImage());
}

void PicManager::updateImage()
{
    loadImage(curImage->currentImage());
}

void PicManager::hideEvent ( QHideEvent * event )
{
    setAnimationPaused(true);
}

void PicManager::showEvent ( QShowEvent * event )
{
    setAnimationPaused(false);
}

void PicManager::deleteFile(bool needAsk)
{
    if(!hasFile() || !QFile::exists(curPath)) return;
    if (!curImage->getReady()) return;

    if(needAsk){
        int ret = QMessageBox::question(
                    0, tr("Delete File"),
                    tr("Are you sure to delete file '%1'?").arg(curName),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        if(ret == QMessageBox::No)
            return;
    }

    curImage->recycle();
    OSRelated::moveFile2Trash(curPath); ///
//    if (!QFile::exists(curPath)) {
//        QPoint site = mapToGlobal(pos()) + QPoint(10, height() - 40);
//        ToolTip::showText(site, tr("Delete file success!"));
//    }
}

