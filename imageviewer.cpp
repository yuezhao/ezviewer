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

#include <QtGui>

#include "global.h"
#include "imageviewer.h"
#include <QtDebug>

//!    paste()
//    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
//    if (mimeData->hasImage())
//        pixmap = qvariant_cast<QPixmap>(mimeData->imageData());
//    update();


const int threshold = 10;

static QPoint deaccelerate(const QPoint &speed, int a = 1, int max = 64)
{
    int x = qBound(-max, speed.x(), max);
    int y = qBound(-max, speed.y(), max);
    x = (x == 0) ? x : (x > 0) ? qMax(0, x - a) : qMin(0, x + a);
    y = (y == 0) ? y : (y > 0) ? qMax(0, y - a) : qMin(0, y + a);
    return QPoint(x, y);
}

ImageViewer::ImageViewer(QWidget *parent)
    : QWidget(parent)
{
    state = NoFileNoPicture;
    movie = NULL;
    currentIndex = -1;
    antialiasMode = 0;
    selfAdaptive = false;

    justPressed = false;
    timeStamp = QTime::currentTime();
    connect(&timer, SIGNAL(timeout()), SLOT(myTimerEvent()));

    setAcceptDrops(true);   //! !!
    setMinimumSize(MIN_SIZE);
}

ImageViewer::~ImageViewer()
{
    SafeDelete(movie);
}

void ImageViewer::changeAntialiasMode(int mode)
{
    if(mode < 0 || mode > 2) return;

    antialiasMode = mode;
    update();
}

void ImageViewer::changeBgColor(const QColor &color)
{
//    if(color.isValid()){
        bgColor = color;
        update();
//    }
}

void ImageViewer::updateShift()
{
    QRectF widgetRect(rect()); //! + SIZE_SHIFT ???
    QRectF pixRect(topLeft + shift, image.size()*scale);
    if(pixRect.width() <= widgetRect.width()
            && pixRect.height() <= widgetRect.height()){
        shift = ORIGIN_POINT;
        setCursor(QCursor(Qt::ArrowCursor));
    }else{
        if(pixRect.width() <= widgetRect.width())
            shift.setX(0);
        else if(pixRect.left() > 0 && pixRect.right() > widgetRect.width())
            shift.setX(shift.x() + (0 - pixRect.left()));
        else if(pixRect.left() < 0 && pixRect.right() < widgetRect.width())
            shift.setX(shift.x() + (widgetRect.width() - pixRect.right()));

        if(pixRect.height() <= widgetRect.height())
            shift.setY(0);
        else if(pixRect.top() > 0 && pixRect.bottom() > widgetRect.height())
            shift.setY(shift.y() + (0 - pixRect.top()));
        else if(pixRect.top() < 0 && pixRect.bottom() < widgetRect.height())
            shift.setY(shift.y() + (widgetRect.height() - pixRect.bottom()));

        if(cursor().shape() != Qt::ClosedHandCursor)//in mouse move event
            setCursor(QCursor(Qt::OpenHandCursor));
    }

    update();
}

void ImageViewer::updatePixmap()
{
    image = movie->currentImage();
    if(rotate != 0)
        image = image.transformed(QMatrix().rotate(rotate),
                                  Qt::SmoothTransformation);
    if(mirrorH || mirrorV)
        image = image.mirrored(mirrorH, mirrorV);
    update(QRect((topLeft + shift).toPoint(), image.size()*scale));
}

void ImageViewer::updateFileIndex(const QString &file)
{
    QFileInfo fileInfo(file);
    QDir dir(fileInfo.absolutePath(), SUPPORT_FORMAT, QDir_SORT_FLAG,
             QDir::Files);
    list = dir.entryInfoList();
    //! verify if currentIndex is valid.
    if(currentIndex < 0 || currentIndex >= list.size() ||
            list.at(currentIndex).absoluteFilePath() != filePath)
        currentIndex = list.indexOf(fileInfo);
}

void ImageViewer::noFileToShow()
{
    image = QImage();
    state = NoFileNoPicture;
    update();
    emit fileNameChange("");    //
    if(cursor().shape() != Qt::ArrowCursor)
        setCursor(QCursor(Qt::ArrowCursor));
}

void ImageViewer::openFile(const QString &file)
{
    updateFileIndex(file);

    loadImage(QFileInfo(file));
}

//! HOWTO improve the loading? multi-threads to read the previous image and the next one?
void ImageViewer::loadImage(const QFileInfo &fileInfo)
{
    //! must test if hasPicture() !
    if(hasPicture() && filePath == fileInfo.absoluteFilePath())//! if the image needs refresh?
        return;

    filePath = fileInfo.absoluteFilePath();

    SafeDelete(movie);  //free memory

    QImageReader reader(filePath);
    reader.setDecideFormatFromContent(true);
    format = reader.format();
    frameCount = reader.imageCount();

    if(format == "gif"){
        movie = new QMovie(filePath);
        if(movie->isValid()){
            if(movie->state() == QMovie::NotRunning)
                movie->start();
            image = movie->currentImage();
            if(frameCount != 1)
                connect(movie, SIGNAL(updated(QRect)), SLOT(updatePixmap()));
            state = FilePicture;
        }else{//cannot read image, so delete
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

        if (reader.read(&image)) {
            state = FilePicture;
        }else{//cannot read image
            state = FileNoPicture;
            image = QImage();   ///
        }
    }else if(format == "gif" && frameCount == 1){
        SafeDelete(movie);
    }

    rotate = 0;
    mirrorH = false;
    mirrorV = false;
    hasUserZoom = false;//! must before call initAdaptiveWidget()
    if(selfAdaptive)
        initAdaptiveWidget();
    else
        initToFitWidget();
    repaint();
    emit fileNameChange(fileInfo.fileName());
}

void ImageViewer::initAdaptiveWidget()
{
    //restore the cursor even if the image cannot load.
    setCursor(QCursor(Qt::ArrowCursor));

    if(!hasPicture()) return;

    QSize sizeSuitable(image.size().expandedTo(MIN_SIZE));
    //minus the topLevel window's frame,like the title bar.
    QSize sizeMaximun(QApplication::desktop()->availableGeometry().size());
    QWidget *topLevelWidget = parentWidget() ? parentWidget() : this;
    sizeMaximun -= (topLevelWidget->frameGeometry().size()
                    - topLevelWidget->geometry().size());
    sizeSuitable = sizeSuitable.boundedTo(sizeMaximun);

    QSize pixSize(image.size());
    //if image large than widget, will scale image to fit widget.
    if(!(sizeSuitable - pixSize).isValid())//! SIZE_ADJUST !!!
        pixSize.scale(sizeSuitable + SIZE_ADJUST, Qt::KeepAspectRatio);
    if(image.width() == 0)
        scale = 1.0;
    else
        scale = qreal(pixSize.width()) / image.width();
    scaleMin = qMin(SCALE_MIN, scale);

    updateTopLeft();
    shift = ORIGIN_POINT;

    if(parentWidget())
        emit sizeChange(pixSize);// to parent widget
    else
        resize(pixSize);    // if this is top level widget
}

void ImageViewer::initToFitWidget()//no change the value of rotate
{
    //restore the cursor even if the image cannot load.
    setCursor(QCursor(Qt::ArrowCursor));

    if(!hasPicture()) return;

    QSize pixSize(image.size());
    //if image large than widget, will scale image to fit widget.
    if(!(rect().size() - pixSize).isValid())//! SIZE_ADJUST !!!
        pixSize.scale(rect().size() + SIZE_ADJUST, Qt::KeepAspectRatio);
    if(image.width() == 0)
        scale = 1.0;
    else
        scale = qreal(pixSize.width()) / image.width();
    scaleMin = qMin(SCALE_MIN, scale);

    updateTopLeft();
    shift = ORIGIN_POINT;
}

void ImageViewer::zoomIn(qreal factor)
{
    if(!hasPicture()) return;

    qreal scale_old = scale;
    scale += factor;
    scale = qMax(scaleMin, qMin(SCALE_MAX, scale));
    if(scale == scale_old)//scale no changed
        return;

    /*! topLeft must determined before shift,
     * otherwise will impact updateShift()
     */
    updateTopLeft();
    shift /= scale_old;
    shift *= scale;
    updateShift();
    hasUserZoom = true;
}

void ImageViewer::nextPic()
{
    if(!hasFile() && filePath.isEmpty()) return;

    updateFileIndex(filePath);

    if(currentIndex + 1 == list.size()) //arrive the end of the file list
        currentIndex = -1;
    if(currentIndex + 1 < list.size())
        loadImage(list.at(++currentIndex));
    else//no image file with support suffix
        noFileToShow();
}

void ImageViewer::prePic()
{
    if(!hasFile() && filePath.isEmpty()) return;

    updateFileIndex(filePath);

    if(currentIndex - 1 < 0) //arrive the head of file list or source file is deleted.
        currentIndex = list.size();
    if(currentIndex > 0)
        loadImage(list.at(--currentIndex));
    else//no image file with support suffix
        noFileToShow();
}

void ImageViewer::rotatePixmap(bool isLeft)
{
    if(!hasPicture()) return;

    rotate += (isLeft ? -90 : 90);
    rotate %= 360;
    QMatrix m = QMatrix().rotate(isLeft ? -90 : 90);
    image = image.transformed(m, Qt::SmoothTransformation);
    initToFitWidget();
    update();
}

void ImageViewer::mirrored(bool horizontal, bool vertical)
{
    if(!hasPicture()) return;

    if(horizontal)
        mirrorH = !mirrorH;
    if(vertical)
        mirrorV = !mirrorV;

    image = image.mirrored(horizontal, vertical);
    initToFitWidget();
    update();
}

void ImageViewer::copyToClipboard()
{
    QApplication::clipboard()->setImage(image);
}

void ImageViewer::switchGifPause()
{
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

void ImageViewer::nextGifFrame()
{
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

void ImageViewer::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    if(bgColor.isValid())
        painter.fillRect(e->rect(), bgColor);

//    QFont font("Arial", 20, QFont::Bold, true);
//    font.setLetterSpacing(QFont::AbsoluteSpacing,5);
//    painter.setFont(font);

    switch(state){
    case FileNoPicture:
         painter.drawText(rect(), Qt::AlignCenter,
                          tr("Cannot load picture:\n'%1'.").arg(filePath));
        break;
    case FilePicture:
//        qDebug() << QString("paintEvent top_left(%1,%2) size(%3,%4)")
//                    .arg(e->rect().left()).arg(e->rect().top())
//                    .arg(e->rect().width()).arg(e->rect().height());
//        qDebug() << QString("topLeft(%1,%2)").arg(topLeft.x()).arg(topLeft.y());
//        qDebug() << QString("scale : %1").arg(scale);
//        qDebug() << QString("pixmap.size(%1,%2)").arg(pixmap.width()).arg(pixmap.height());
//        qDebug() << QString("translate to (%1,%2)")
//                    .arg((topLeft + shift).toPoint().x())
//                    .arg((topLeft + shift).toPoint().y());
//        qDebug() << QString("scale size to (%1,%2)")
//                    .arg(pixmap.width()*scale)
//                    .arg(pixmap.height()*scale);

        switch(antialiasMode){
        case 0:
            if(scale <= 1) break;
        case 1:
//            qDebug() << "use antialiasing";
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            break;
//        case 2:
        default:
            break;
        }

        painter.translate(topLeft + shift);
        painter.scale(scale, scale);
//        painter.setClipRect(e->rect());
        painter.drawImage(ORIGIN_POINT, image);
        break;
    case PictureNoFile:     //! /////paste()
    case NoFileNoPicture:
    default:
        break;
    }
}

void ImageViewer::hideEvent ( QHideEvent * event )
{
    if(movie)
        movie->setPaused(true);
}

void ImageViewer::showEvent ( QShowEvent * event )
{
    if(movie)
        movie->setPaused(false);
}

void ImageViewer::contextMenuEvent( QContextMenuEvent * event )
{
    emit showContextMenu(event->globalPos());
}

void ImageViewer::wheelEvent(QWheelEvent *e)
{
    if(!hasPicture() || !rect().contains(e->pos())) //cursor is not in widget
        return;

    qreal scale_old = scale;

    switch(e->modifiers()){
    case Qt::ShiftModifier:
        zoomIn(e->delta() / qreal(2400)); //e->delta() is +120 or -120
        break;
    case Qt::ControlModifier:
        zoomIn(e->delta() / qreal(600));
        break;
    default:
        zoomIn(e->delta() / qreal(1200));
        break;
    }

    QPointF distance(rect().center() - e->pos());
    QPointF change(distance / scale_old * scale - distance);
    shift += change;    //to keep the cursor position, must after scale.
    updateShift();
}

void ImageViewer::mouseDoubleClickEvent ( QMouseEvent * event )
{
    if(event->button() & Qt::LeftButton)
        emit mouseDoubleClick();
}

void ImageViewer::mousePressEvent ( QMouseEvent * event )
{
    if(event->button() & Qt::LeftButton){
        startPos = event->globalPos();
        if(hasPicture() && cursor().shape() == Qt::OpenHandCursor)
            setCursor(QCursor(Qt::ClosedHandCursor));

        justPressed = true;
        pressPos = event->pos();
        if(timer.isActive()){ // auto scroll
            speed = QPoint(0, 0);
            timer.stop();
        }
    }
}

void ImageViewer::mouseMoveEvent ( QMouseEvent * event )
{
    //! For mouse move events, this is all buttons that are pressed down.
    if(event->buttons() & Qt::LeftButton)
        myMouseMove(event);
}

void ImageViewer::mouseReleaseEvent ( QMouseEvent * event )
{
    if(event->button() & Qt::LeftButton){
        myMouseMove(event);

        if(!justPressed && speed != QPoint(0, 0)) {
            speed /= 4; //! ??
            timer.start(AUTO_SCROLL_INTERVAL);
        }

        if(cursor().shape() == Qt::ClosedHandCursor)
            setCursor(QCursor(Qt::OpenHandCursor));
    }
}

void ImageViewer::myMouseMove(QMouseEvent * event)
{
    QPoint change = event->globalPos() - startPos;

    if(event->modifiers() == Qt::ControlModifier){
        emit siteChange(change);
    }else{
        //if widget smaller than widget, allow to move image.
        //! + SIZE_ADJUST
        if(hasPicture() && !(rect().size() + SIZE_ADJUST - image.size()*scale).isValid()){
            shift += change;
            updateShift();


            QPoint delta = event->pos() - pressPos;
            if(justPressed){
                if (delta.x() > threshold || delta.x() < -threshold ||
                        delta.y() > threshold || delta.y() < -threshold) {
                    timeStamp = QTime::currentTime(); //start calculate
                    justPressed = false; ///
                    this->delta = QPoint(0, 0);
                    pressPos = event->pos();
                }
            }else{
                if (timeStamp.elapsed() > 100) {
                    timeStamp = QTime::currentTime();   //restart calculate
                    speed = delta - this->delta;    ///
                    this->delta = delta;
                }
            }
        }
    }

    startPos = event->globalPos();    //
}

void ImageViewer::myTimerEvent()
{
    speed = deaccelerate(speed);

    QPointF shiftOld = shift;
    shift += speed;
    updateShift();

    if(shift == shiftOld)
        speed = QPoint(0, 0);
    if (speed == QPoint(0, 0))
        timer.stop();
}

void ImageViewer::resizeEvent(QResizeEvent *e)
{
    if(!hasPicture()) return;

    if(hasUserZoom){
        updateTopLeft();
        updateShift();
    }else{
        initToFitWidget();
    }
    QWidget::resizeEvent(e);
}

void ImageViewer::dragEnterEvent(QDragEnterEvent *event)
{
//    if (event->mimeData()->hasFormat("text/uri-list")) {
//        event->acceptProposedAction();
//    }

    const QMimeData *mimeData = event->mimeData();
    if (event->mimeData()->hasUrls()){
        QList<QUrl> urlList(mimeData->urls());
        QFileInfo fileInfo;
        for (int i = 0; i < urlList.size(); ++i) {
            fileInfo.setFile(urlList.at(i).toLocalFile());
            if(fileInfo.isFile()){
//                    && FORMAT_LIST.contains(fileInfo.suffix().toLower())){
                event->acceptProposedAction();
                break;
            }
        }
    }
}

void ImageViewer::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList(mimeData->urls());
        QFileInfo fileInfo;
        QString fileName;
        while(!urlList.empty()){
            fileName = urlList.first().toLocalFile();
            urlList.removeFirst();
            fileInfo.setFile(fileName);
            if(fileInfo.isFile()){
                openFile(fileName);
                break;
            }
        }

        QStringList fileList;
        for(int size = urlList.size(), i=0; i < size; ++i)
            fileList.append(urlList.at(i).toLocalFile());
        if(!fileList.empty())
            emit needOpenFile(fileList);
    }

    event->acceptProposedAction();
}

void ImageViewer::deleteFile(bool messagebox)//! MOVE to mainwindow.cpp...
{
    if(!hasFile()) return;

    QFile file(filePath);
    if(!file.exists()) return;

    if(messagebox){
        int ret = QMessageBox::question(
                    this, tr("Delete File"),
                    tr("Are you sure to delete file '%1'?").arg(file.fileName()),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        if(ret == QMessageBox::No)
            return;
    }

    if(movie) SafeDelete(movie); //! gif image: must free movie before delete file.

    updateFileIndex(filePath);
    int oldIndex = currentIndex;
    if(!file.remove()){
        QMessageBox::warning(this, tr("Delete Failed"),
                             tr("Delete file '%1' failed!").arg(file.fileName()));
    }else{
        updateFileIndex(filePath);
        if(oldIndex != -1)
            currentIndex = oldIndex - 1;    ///so that next time will show the next image of current one.
        if(currentIndex + 1 >= list.size()) //arrive the end of file list
            currentIndex = -1;
        if(currentIndex + 1 < list.size())
            loadImage(list.at(++currentIndex));
        else//no image with suffix supported
            noFileToShow();
    }
}

QString ImageViewer::attributeString()
{
    if(state == NoFileNoPicture) return QString::null;    //

    QString info;
    QFileInfo fileInfo(filePath);

    if(fileInfo.exists()){
        const QString timeFormat(tr("yyyy-MM-dd, hh:mm:ss"));
        QString sizeStr;
        qint64 size = fileInfo.size();
        if(size <= 1024) // 1K = 1024
            sizeStr = tr("%1 B").arg(size);
        else if ( size <= 1048576 ) // 1M = 1048576 = 1024 * 1024
            sizeStr = tr("%1 KB").arg(size / qreal(1024), 0, 'g', 3);
        else if ( size <= 1073741824 ) // 1G = 1073741824 = 1024 * 1024 * 1024
            sizeStr = tr("%1 MB").arg(size / qreal(1048576), 0, 'g', 3);

        info += tr("File Name: %1<br>").arg(fileInfo.fileName());
        info += tr("File Size: %1 (%2 Bytes)<br>").arg(sizeStr).arg(size);
        info += tr("Created Time: %1<br>")
                .arg(fileInfo.created().toString(timeFormat));
        info += tr("Modified Time: %1<br>")
                .arg(fileInfo.lastModified().toString(timeFormat));
        info += tr("Last Read: %1")
                .arg(fileInfo.lastRead().toString(timeFormat));
        if(!format.isEmpty())
            info += tr("<br>Image Format: %1").arg(format);
    }

    if(!image.isNull()){
        if(!info.isEmpty())
            info += tr("<br>");

        if(image.colorCount() > 0)
            info += tr("Color Count: %1<br>").arg(image.colorCount());
        else if(image.depth() >= 16)
            info += tr("Color Count: True color<br>");
        info += tr("Depth: %1<br>").arg(image.depth());
    //    info += tr("BitPlaneCount: %1<br>").arg(image.bitPlaneCount());//the color counts actual used, <= Depth

        qreal ratio = (image.height() != 0) ?
                    qreal(image.width()) / image.height() : 1;
        QString ratioStr;
        if(ratio == 4.0/3)
            ratioStr = "4:3";
        else if(ratio == 16.0/9)
            ratioStr = "16:9";
        else if(ratio == 16.0/10)
            ratioStr = "16:10";
        else if(ratio == 1366.0/768)
            ratioStr = "16:9";
        else if(ratio == 3.0/4)
            ratioStr = "3:4";
        else if(ratio == 9.0/16)
            ratioStr = "9:16";
        else if(ratio > 1)
            ratioStr = tr("%1:1").arg(ratio, 0, 'g', 3);
        else
            ratioStr = tr("1:%1").arg(1 / ratio, 0, 'g', 3);

        info += tr("Size: %1 x %2 (%3)<br>")
                .arg(image.width()).arg(image.height()).arg(ratioStr);
        if(fileInfo.exists() && frameCount != 1)
            info += tr("Frame Count: %1<br>").arg(frameCount);
        info += tr("Current Scale: %1%").arg(scale * 100, 0, 'g', 4);
    }

    return info;
}
