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

//! HOWTO improve the loading? multi-threads to read the previous image and the next one?
//void ImageViewer::loadImage(const QImage &im, const QString &msg_if_no_image)


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
    antialiasMode = 0;

    justPressed = false;
    timeStamp = QTime::currentTime();
    connect(&timer, SIGNAL(timeout()), SLOT(myTimerEvent()));

    setMinimumSize(MIN_SIZE);
}

void ImageViewer::changeAntialiasMode(int mode)
{
    if(mode < 0 || mode > 2) return;

    antialiasMode = mode;
    update();
}

void ImageViewer::changeBgColor(const QColor &color)
{
    if(color.isValid()){
        bgColor = color;
        update();
    }
}

void ImageViewer::updatePixmap(const QImage &im)
{
    image = im;
    if(rotate != 0)
        image = image.transformed(QMatrix().rotate(rotate),
                                  Qt::SmoothTransformation);
    if(mirrorH || mirrorV)
        image = image.mirrored(mirrorH, mirrorV);

    update(QRect((topLeft + shift).toPoint(), image.size()*scale));
}

void ImageViewer::loadImage(const QImage &im, const QString &msg_if_no_image)
{
    image = im;
    errStr = msg_if_no_image;
    rotate = 0;
    mirrorH = false;
    mirrorV = false;
    hasUserZoom = false;
    initToFitWidget();
    repaint();
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

void ImageViewer::initToFitWidget()//no change the value of rotate
{
    //restore the cursor even if the image cannot load.
    setCursor(QCursor(Qt::ArrowCursor));

    if(noPicture()) return;

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
    if(noPicture()) return;

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

void ImageViewer::rotatePixmap(bool isLeft)
{
    if(noPicture()) return;

    rotate += (isLeft ? -90 : 90);
    rotate %= 360;
    QMatrix m = QMatrix().rotate(isLeft ? -90 : 90);
    image = image.transformed(m, Qt::SmoothTransformation);
    initToFitWidget();
    update();
}

void ImageViewer::mirrored(bool horizontal, bool vertical)
{
    if(noPicture()) return;

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

void ImageViewer::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    if(bgColor.isValid())
        painter.fillRect(e->rect(), bgColor);

    if(noPicture() && !errStr.isEmpty()){ // case FileNoPicture:
        painter.drawText(rect(), Qt::AlignCenter, errStr);
        return;
    }

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
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        break;
//       case 2:
    default:
        break;
    }

    painter.translate(topLeft + shift);
    painter.scale(scale, scale);
//    painter.setClipRect(e->rect());
    painter.drawImage(ORIGIN_POINT, image);
}

void ImageViewer::contextMenuEvent( QContextMenuEvent * event )
{
    emit showContextMenu(event->globalPos());
}

void ImageViewer::resizeEvent(QResizeEvent *e)
{
    if(noPicture()) return;

    if(hasUserZoom){
        updateTopLeft();
        updateShift();
    }else{
        initToFitWidget();
    }
    QWidget::resizeEvent(e);
}

void ImageViewer::wheelEvent(QWheelEvent *e)
{
    if(noPicture() || !rect().contains(e->pos())) //cursor is not in widget
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
