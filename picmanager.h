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

#ifndef PICMANAGER_H
#define PICMANAGER_H

#include <QString>
#include <QStringList>
#include <QFileSystemWatcher>

#include "imageviewer.h"
#include "imagecache.h"

class PicManager : public ImageViewer
{
    Q_OBJECT
public:
    PicManager(QWidget *parent = 0);
    ~PicManager();

    const QImage  & currentImage() const { return curCache->image; }
    const QString & fileName() const { return curName; }
    const QString & filePath() const { return curPath; }
    bool  hasPicture() const { return !currentImage().isNull(); } /// need virtual??
    bool  hasFile() const            { return !curPath.isEmpty(); }
    bool  isAnimation() const        { return curCache->movie; }
    QString attribute() const;

signals:
    void fileNameChange(const QString &fileName);

public slots:
    void openFile(const QString &file);
    void openFiles(const QStringList &fileList);
    bool prePic();
    bool nextPic();

    void deleteFileAsk() { deleteFile(true); }
    void deleteFileNoAsk() { deleteFile(false); }

    void switchGifPause();
    void nextGifFrame();
    void setGifPaused(bool paused);

protected slots:
    void updateGifImage();
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
    void readFile(const QString &file);
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

    ImageCache *curCache;
    QString curPath;
    QString curName;

    enum LIST_MODE {
        FileNameListMode,
        FullPathListMode,
    } listMode;
    QString curDir; // only use for FileNameListMode
    QStringList list;
    int currentIndex;
    QFileSystemWatcher fsWatcher;
};


#endif // PICMANAGER_H
