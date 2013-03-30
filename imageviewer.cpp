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

#include <QtGui>

#include "config.h"
#include "imageviewer.h"
#include "tooltip.h"

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
    antialiasMode = 0;

    justPressed = false;
    timeStamp = QTime::currentTime();
    connect(&timer, SIGNAL(timeout()), SLOT(myTimerEvent()));

    setMinimumSize(Config::WindowMinSize);
}

void ImageViewer::changeAntialiasMode(int mode)
{
    if(mode < 0 || mode > 2 || antialiasMode == mode) return;

    antialiasMode = mode;
    updateImageArea();
}

// If color is invalid, means no background color enabled.
void ImageViewer::changeBgColor(const QColor &color)
{
    if(bgColor != color){
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

    updateImageArea();
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
    repaint();      ///
}

void ImageViewer::updateShift()
{
    QRectF widgetRect(rect()); //! + SIZE_SHIFT ???
    QRectF pixRect(topLeft + shift, image.size()*scale);
    if(pixRect.width() <= widgetRect.width()
            && pixRect.height() <= widgetRect.height()){
        shift = Config::OriginPoint;
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
        pixSize.scale(rect().size() + Config::SizeAdjusted, Qt::KeepAspectRatio);
    if(image.width() == 0)
        scale = 1.0;
    else
        scale = qreal(pixSize.width()) / image.width();
    scaleMin = qMin(Config::ScaleMin, scale);

    updateTopLeft();
    shift = Config::OriginPoint;
}

void ImageViewer::zoomIn(double factor)
{
    if(noPicture()) return;

    qreal scale_old = scale;
    scale += factor;
    scale = qMax(scaleMin, qMin(Config::ScaleMax, scale));
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

    ToolTip::showText(mapToGlobal(rect().center()),
                      QString("<font size='7'><b>%1%</b></font>").arg(scale * 100, 0, 'g', 4),
                      true, 0.7, 800);
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

    painter.save();

    painter.translate(topLeft + shift);
    painter.scale(scale, scale);
//    painter.setClipRect(e->rect());
    painter.drawImage(Config::OriginPoint, image);

    painter.restore();  ///


    // draw scroll bar
    if(/*!justPressed && */ speed != QPoint(0, 0)){
        const QColor LINE_COLOR(0, 0, 0, 80);
        painter.setBrush(LINE_COLOR);
        painter.setPen(Qt::NoPen);

        const int LINE_WIDTH = 10;
        const int MARGE = 3;
        const qreal RADIUS = 6.0;
        int rectW = rect().width(), rectH = rect().height();
        qreal scaleW = image.width() * scale; //! qreal
        qreal scaleH = image.height() * scale;
        if(scaleW > rectW){     // draw horizontal scroll bar
            QRectF rectangle(-(topLeft + shift).x() / scaleW * rectW,
                            rectH - LINE_WIDTH - MARGE,
                            rectW / scaleW * rectW,
                            LINE_WIDTH);
            painter.drawRoundedRect(rectangle, RADIUS, RADIUS);
        }
        if(scaleH > rectH){     // draw vertical scroll bar
            QRectF rectangle(rectW - LINE_WIDTH - MARGE,
                            -(topLeft + shift).y() / scaleH * rectH,
                            LINE_WIDTH,
                            rectH / scaleH * rectH);
            painter.drawRoundedRect(rectangle, RADIUS, RADIUS);
        }
    }
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
            timer.start(Config::AutoScrollInterval);
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
        if(hasPicture() && !(rect().size() + Config::SizeAdjusted - image.size()*scale).isValid()){
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
