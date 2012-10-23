#include "picmanager.h"

#include "global.h"
#include "toolkit.h"

#include <QMessageBox>
#include <QImageReader>
#include <QDir>
#include <QFile>

#ifdef Q_WS_WIN
#include <windows.h>
#include <ShellAPI.h>
#   ifdef UNICODE
#   define QStringToCHAR(x) (wchar_t*)x.utf16()
#   else
#   define QStringToCHAR(x) x.local8Bit().constData()
#   endif // UNICODE
#endif // Q_WS_WIN


PicManager::PicManager(QWidget *parent)
    : ImageViewer(parent), curCache(new ImageCache()), currentIndex(-1), fsWatcher(this)
{
//    state = NoFileNoPicture;
    connect(&fsWatcher, SIGNAL(directoryChanged(QString)),
            SLOT(directoryChanged(QString)));
}

PicManager::~PicManager()
{
    SafeDelete(curCache);
}

void PicManager::openFile(const QString &file)
{
    fsWatcher.removePaths(fsWatcher.files() + fsWatcher.directories());

    //    updateFileIndex(file);
    QFileInfo fileInfo(file);
    QDir dir(fileInfo.absolutePath(), SUPPORT_FORMAT, QDir_SORT_FLAG,
             QDir::Files);
    list = dir.entryInfoList();
    currentIndex = list.indexOf(fileInfo);

    readFile(fileInfo);

    QString dirPath(fileInfo.absolutePath());
    fsWatcher.addPath(dirPath);
    fsWatcher.addPath(QFileInfo(dirPath).absolutePath());
}

void PicManager::updateFileIndex(const QString &file)
{
    QFileInfo fileInfo(file);
    QDir dir(fileInfo.absolutePath(), SUPPORT_FORMAT, QDir_SORT_FLAG,
             QDir::Files);
    list = dir.entryInfoList();
    //! verify if currentIndex is valid.
    if(currentIndex < 0 || currentIndex >= list.size() ||
            list.at(currentIndex).absoluteFilePath() != curPath)
        currentIndex = list.indexOf(fileInfo);
}

void PicManager::directoryChanged(const QString &dirPath)
{
    qDebug("dirChanged: %s", qPrintable(dirPath));
    updateFileIndex(curPath);
}

bool PicManager::prePic()
{
    if(!hasFile()) return false;

    if(currentIndex - 1 < 0) //arrive the head of file list or source file is deleted.
        currentIndex = list.size();
    readFile(list.at(--currentIndex));
    return true;
}

bool PicManager::nextPic()
{
    if(!hasFile()) return false;

    if(currentIndex + 1 == list.size()) //arrive the end of the file list
        currentIndex = -1;
    readFile(list.at(++currentIndex));
    return true;
}

void PicManager::readFile(const QFileInfo &fileInfo)
{
    //! must test if hasPicture() !
    if(/*hasPicture() && */ curPath == fileInfo.absoluteFilePath() )//! if the image needs refresh?
        return;

    curPath = fileInfo.absoluteFilePath();
    curName = fileInfo.fileName();

    SafeDelete(curCache);
    curCache = getCache(fileInfo);

    if(curCache->movie){
        if(curCache->movie->state() == QMovie::NotRunning)
            curCache->movie->start();
        connect(curCache->movie, SIGNAL(updated(QRect)), SLOT(updateGifImage()));
    }

    QString msg = curCache->image.isNull() ? tr("Cannot load picture:\n'%1'.")
                                      .arg(filePath()) : QString::null;
    loadImage(curCache->image, msg);
//     state = image.isNull() ? FileNoPicture : FilePicture;
    emit fileNameChange(curName);
}

PicManager::ImageCache * PicManager::getCache(const QFileInfo &fileInfo)
{
    ImageCache *result = new ImageCache();
    QImage   &image = result->image;
    QMovie * &movie = result->movie;
    QString &format = result->format;
    int &frameCount = result->frameCount;

    QString path(fileInfo.absoluteFilePath());

    QImageReader reader(path);
    reader.setDecideFormatFromContent(true);
    format = reader.format();
    frameCount = reader.imageCount();

    if(format == "gif"){
        movie = new QMovie(path);
        if(movie->isValid()){
            if(movie->state() == QMovie::NotRunning)
                movie->start(); ///
            image = movie->currentImage();
            movie->stop();  ///
        }else{    //cannot read image, so delete
            SafeDelete(movie);
        }
    }

    if(!movie){
        if(format == "ico"){//! is ico image has the same height and width?
            reader.read(&image);
            int maxIndex = 0;
            int maxWidth = image.width();
            for(int i=1; i < frameCount; ++i){
                if(!reader.jumpToNextImage())
                    break;
                reader.read(&image);
                if(maxWidth < image.width()){
                    maxWidth = image.width();
                    maxIndex = i;
                }
            }
            reader.jumpToImage(maxIndex);
        }

        if (!reader.read(&image)) // cannot read image
            image = QImage();   ///

    }else if(/*format == "gif" && */ frameCount == 1){
        SafeDelete(movie);
    }

    return result;
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
    curCache->image = curCache->movie->currentImage();
    updatePixmap(curCache->image);
}

void PicManager::hideEvent ( QHideEvent * event )
{
    setGifPaused(true);
}

void PicManager::showEvent ( QShowEvent * event )
{
    setGifPaused(false);
}


bool deleteFile(const QString &filePath)
{
#ifdef Q_WS_WIN
    qDebug(qPrintable(QDir::toNativeSeparators(filePath)));

    SHFILEOPSTRUCT FileOp;//定义SHFILEOPSTRUCT结构对象;
    FileOp.hwnd = 0;
    FileOp.wFunc = FO_DELETE; //执行文件删除操作;
    FileOp.pFrom = QStringToCHAR(QDir::toNativeSeparators(filePath));  //源文件路径
    FileOp.pTo = NULL; //目标文件路径;
    FileOp.fFlags = FOF_ALLOWUNDO;//此标志使删除文件备份到Windows回收站
    FileOp.fFlags |= FOF_NOCONFIRMATION;    //! 直接删除，不进行确认

    if(SHFileOperation(&FileOp)) //这里开始删除文件
        return false;
    else
        return true;
#else
    return QFile::remove(filePath);
#endif // Q_WS_WIN
}

void PicManager::deleteFile(bool messagebox)
{
    if(!hasFile()) return;
    if(!QFile::exists(curPath)) return; ////

    if(messagebox){
        int ret = QMessageBox::question(
                    0, tr("Delete File"),
                    tr("Are you sure to delete file '%1'?").arg(curName),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        if(ret == QMessageBox::No)
            return;
    }

    if(curCache->movie) SafeDelete(curCache->movie); //! gif image: must free movie before delete file.

    updateFileIndex(curPath);
    int oldIndex = currentIndex;

    if(!::deleteFile(curPath)){// if(!file.remove()){
        QMessageBox::warning(0, tr("Delete Failed"),
                             tr("Delete file '%1' failed!").arg(curName));
    }else{
        updateFileIndex(curPath);
        if(oldIndex != -1)
            currentIndex = oldIndex - 1;    ///so that next time will show the next image of current one.
        if(currentIndex + 1 >= list.size()) //arrive the end of file list
            currentIndex = -1;
        if(currentIndex + 1 < list.size())
            readFile(list.at(++currentIndex));
        else//no image with suffix supported
            noFileToShow();
    }
}

void PicManager::noFileToShow()
{
    curCache->image = QImage();
    curPath = curName = QString::null;
//    state = NoFileNoPicture;
    loadImage(curCache->image);
    emit fileNameChange("");    //
}

QString PicManager::attribute() const
{
    //    if(state == NoFileNoPicture) return QString::null;    //

    QString info;
    QFileInfo fileInfo(curPath);

    if(fileInfo.exists()){
        const QString timeFormat(tr("yyyy-MM-dd, hh:mm:ss"));
        qint64 size = fileInfo.size();
        QString sizeStr = ToolKit::fileSize2Str(size);

        info += tr("File Name: %1<br>").arg(curName);
        info += tr("File Size: %1 (%2 Bytes)<br>").arg(sizeStr).arg(size);
        info += tr("Created Time: %1<br>")
                .arg(fileInfo.created().toString(timeFormat));
        info += tr("Modified Time: %1<br>")
                .arg(fileInfo.lastModified().toString(timeFormat));
        info += tr("Last Read: %1")
                .arg(fileInfo.lastRead().toString(timeFormat));
        if(!curCache->format.isEmpty())
            info += tr("<br>Image Format: %1").arg(curCache->format);
    }

    if(!currentImage().isNull()){
        if(!info.isEmpty())
            info += tr("<br>");

        if(currentImage().colorCount() > 0)
            info += tr("Color Count: %1<br>").arg(currentImage().colorCount());
        else if(currentImage().depth() >= 16)
            info += tr("Color Count: True color<br>");
        info += tr("Depth: %1<br>").arg(currentImage().depth());
    //    info += tr("BitPlaneCount: %1<br>").arg(image.bitPlaneCount());//the color counts actual used, <= Depth

        int gcd = ToolKit::gcd(currentImage().width(), currentImage().height());
        QString ratioStr = (gcd == 0) ? "1:1" : QString("%1:%2")
                                        .arg(currentImage().width() / gcd)
                                        .arg(currentImage().height() / gcd);


        info += tr("Size: %1 x %2 (%3)<br>")
                .arg(currentImage().width())
                .arg(currentImage().height())
                .arg(ratioStr);
        if(fileInfo.exists() && curCache->frameCount != 1)
            info += tr("Frame Count: %1<br>").arg(curCache->frameCount);
        info += tr("Current Scale: %1%").arg(currentScale() * 100, 0, 'g', 4);
    }

    return info;
}
