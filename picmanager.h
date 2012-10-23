#ifndef PICMANAGER_H
#define PICMANAGER_H

#include <QString>
#include <QFileInfoList>
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
    void noFileToShow();
    bool prePic();
    bool nextPic();

    void deleteFileAsk() { deleteFile(true); }
    void deleteFileNoAsk() { deleteFile(false); }

    void switchGifPause();
    void nextGifFrame();
    void setGifPaused(bool paused);

protected slots:
    void updateGifImage();
    void directoryChanged(const QString &dirPath);

    void hideEvent ( QHideEvent * event );
    void showEvent ( QShowEvent * event );

//    enum ImageState{
//        NoFileNoPicture,
//        PictureNoFile,
//        FileNoPicture,
//        FilePicture
//    };

private:
    void updateFileInfoList(const QString &file);
    void updateFileIndex(const QString &file);
    void readFile(const QFileInfo &fileInfo);
    void deleteFile(bool needAsk);

    ImageCache *curCache;
    QString curPath;
    QString curName;
    QFileInfoList list;
    int currentIndex;
    QFileSystemWatcher fsWatcher;
};


#endif // PICMANAGER_H
