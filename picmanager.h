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

#ifndef PICMANAGER_H
#define PICMANAGER_H

#include <QString>
#include <QStringList>
#include <QFileSystemWatcher>

#include "imageviewer.h"
#include "imagewrapper.h"
#include "imagefactory.h"


class PicManager : public ImageViewer
{
    Q_OBJECT
public:
    PicManager(QWidget *parent = 0);
    ~PicManager();

    bool  isAnimation() const { return curImage->isAnimation(); }
    const QString & fileName() const { return curName; }
    const QString & filePath() const { return curPath; }
    bool  hasPicture() const { return !curImage->currentImage().isNull(); } /// need virtual??
    bool  hasFile() const            { return !curPath.isEmpty(); }
    QString attribute() const { return curImage->attribute(); }

    void setCacheNumber(int num) { ImageFactory::setCacheNumber(num); }
    void setPreReadingEnabled(bool enabled)
    { ImageFactory::setPreReadingEnabled(enabled); }

signals:
    void imageChanged(const QString &fileName);

public slots:
    void openFile(const QString &file);
    void openFiles(const QStringList &fileList);
    bool prePic();
    bool nextPic();

    void deleteFileAsk()    { deleteFile(true); }
    void deleteFileNoAsk()  { deleteFile(false); }

    void switchAnimationState() { curImage->switchAnimationPaused(); }
    void nextAnimationFrame()   { curImage->nextAnimationFrame(); }
    void setAnimationPaused(bool paused) { curImage->setAnimationPaused(paused); }

protected slots:
    void updateAnimation();
    void updateImage();
    void directoryChanged();
    void fileChanged(const QString &filePath);

    void hideEvent ( QHideEvent * event );
    void showEvent ( QShowEvent * event );

//    enum ImageState{
//        NoFileNoPicture,
//        PictureNoFile,
//        FileNoPicture,
//        FilePicture
//    };

private:
    void readFile(const QString &fullPath);
    void readFile(int index) { readFile(getPathAtIndex(index)); }
    void deleteFile(bool needAsk);

    // will change curDir, list, currentIndex.
    void updateFileNameList(const QString &curfile);
    // remove fileChanged from list.
    void updateFullPathList(const QString &fileChanged);
    /* compare oldIndex and currentIndex, if current file changed,
     * read new file and show; if list is empty, call noFileToShow();
     */
    void updateFileIndex(int oldIndex);
    void noFileToShow();

    ImageWrapper *curImage;
    QString curPath;
    QString curName;

    int getPreIndex(int curIndex) const;
    int getNextIndex(int curIndex) const;
    QString getPathAtIndex(int index) const;

    void preReadingPic(int index) const;
    void preReadingPrePic() const;
    void preReadingNextPic() const;


    enum LIST_MODE {
        FileNameListMode,
        FullPathListMode
    } listMode;
    QString curDir; // only use for FileNameListMode
    QStringList list;
    int currentIndex;
    QFileSystemWatcher fsWatcher;
};



inline int PicManager::getPreIndex(int curIndex) const
{
    //arrive the head of file list or source file is deleted.
    return (curIndex - 1 < 0) ? list.size() - 1 : curIndex - 1;
}

inline int PicManager::getNextIndex(int curIndex) const
{
    //arrive the end of the file list
    return (curIndex + 1 == list.size()) ? 0 : curIndex + 1;
}

inline void PicManager::preReadingPic(int index) const
{
    ImageFactory::preReading(getPathAtIndex(index));
}

inline void PicManager::preReadingPrePic() const
{
    if(list.size() > 1){
        preReadingPic(getPreIndex(currentIndex));
    }
}

inline void PicManager::preReadingNextPic() const
{
    if(list.size() > 1){
        preReadingPic(getNextIndex(currentIndex));
    }
}

#endif // PICMANAGER_H
