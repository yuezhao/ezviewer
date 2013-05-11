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
#include "velocitytracker.h"


ImageViewer::ImageViewer(QWidget *parent)
    : QWidget(parent),
      scaleMode(Config::DefaultScaleMode),
      alignMode(Config::DefaultAlignMode),
      antialiasMode(Config::DefaultAntialiasMode),
      bgColor(Config::DefaultBgColor),
      leftMousePressed(false),
      velocityTracker(new VelocityTracker(this))
{
    setMinimumSize(Config::WindowMinSize);

    velocityTracker->setTickInterval(Config::AutoScrollInterval);
    connect(velocityTracker, SIGNAL(changedDelta(QPoint)), SLOT(scrollContent(QPoint)));
}

void ImageViewer::changeScaleMode(int mode)
{
    if (mode < Config::ScaleModeBegin || mode > Config::ScaleModeEnd
            || scaleMode == mode)
        return;

    scaleMode = mode;
    layoutImage();
    update();
}

void ImageViewer::changeAlignMode(int mode)
{
    if (mode < Config::AlignModeBegin || mode > Config::AlignModeEnd
            || alignMode == mode)
        return;

    alignMode = mode;
    layoutImage();
    update();
}

void ImageViewer::changeAntialiasMode(int mode)
{
    if(mode < Config::AntialiasModeBegin || mode > Config::AntialiasModeEnd
            || antialiasMode == mode)
        return;

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
    if (velocityTracker->isScrolling())
        velocityTracker->stopAutoScrolling();

    image = im;
    errStr = msg_if_no_image;
    rotate = 0;
    mirrorH = false;
    mirrorV = false;
    hasUserZoom = false;
    layoutImage();
    repaint();      ///
}


void ImageViewer::layoutImage()
{
    if(hasPicture()) {
        calcScaleRatio();
        calcTopLeft();
        calcShift();
    }

    updateCursor();
}

void ImageViewer::updateCursor()
{
    if(noPicture() || !scaleLargeThanWidget()) {
        changeCursor(Qt::ArrowCursor);
    } else if(leftMousePressed) {
        changeCursor(Qt::ClosedHandCursor);
    } else {
        changeCursor(Qt::OpenHandCursor);
    }
}

bool ImageViewer::scaleLargeThanWidget()
{
    return image.width() * scale > rect().width() + Config::SizeAdjusted.width()
            || image.height() * scale > rect().height() + Config::SizeAdjusted.height();
}

void ImageViewer::calcScaleRatio()
{
    if(image.width() == 0 || image.height() == 0) {
        scale = 1.0;
        scaleMin = qMin(Config::ScaleMinLimit, scale);
        return;
    }

    QSize pixSize(image.size());

    switch (scaleMode) {
    case Config::KeepImageSize:
        scale = 1.0;
        break;
    case Config::FitWidgetWidth:
        scale = width() / qreal(image.width());
        break;
    case Config::FitWidgetHeight:
        scale = height() / qreal(image.height());
        break;
    case Config::ScaleToFitWidget:
    {
        qreal widthRatio = width() / qreal(image.width());
        qreal heightRatio = height() / qreal(image.height());
        scale = qMin(widthRatio, heightRatio);
        break;
    }
    case Config::ScaleToExpandWidget:
    {
        qreal widthRatio = width() / qreal(image.width());
        qreal heightRatio = height() / qreal(image.height());
        scale = qMax(widthRatio, heightRatio);
        break;
    }
    case Config::ScaleLargeImageToFitWidget:
    default:
        //if image large than widget, will scale image to fit widget.
        if(!(rect().size() - pixSize).isValid())
            pixSize.scale(rect().size() + Config::SizeAdjusted, Qt::KeepAspectRatio);
        scale = qreal(pixSize.width()) / image.width();
        break;
    }

    scaleMin = qMin(Config::ScaleMinLimit, scale);
}

void ImageViewer::calcShift()
{
    if(!scaleLargeThanWidget()) {
        shift.setX(0);
        shift.setY(0);
        return;
    }

    switch (alignMode) {
    case Config::AlignLeftTop:
        shift.setX(topLeft.x() < 0 ? -topLeft.x() : 0);
        shift.setY(topLeft.y() < 0 ? -topLeft.y() : 0);
        break;
    case Config::AlignCenterTop:
        shift.setX(0);
        shift.setY(topLeft.y() < 0 ? -topLeft.y() : 0);
        break;
    case Config::AlignRightTop:
        shift.setX(topLeft.x() < 0 ? topLeft.x() : 0);
        shift.setY(topLeft.y() < 0 ? -topLeft.y() : 0);
        break;
    case Config::AlignLeftCenter:
        shift.setX(topLeft.x() < 0 ? -topLeft.x() : 0);
        shift.setY(0);
        break;
    case Config::AlignRightCenter:
        shift.setX(topLeft.x() < 0 ? topLeft.x() : 0);
        shift.setY(0);
        break;
    case Config::AlignLeftBottom:
        shift.setX(topLeft.x() < 0 ? -topLeft.x() : 0);
        shift.setY(topLeft.y() < 0 ? topLeft.y() : 0);
        break;
    case Config::AlignCenterBottom:
        shift.setX(0);
        shift.setY(topLeft.y() < 0 ? topLeft.y() : 0);
        break;
    case Config::AlignRightBottom:
        shift.setX(topLeft.x() < 0 ? topLeft.x() : 0);
        shift.setY(topLeft.y() < 0 ? topLeft.y() : 0);
        break;
    case Config::AlignCenterCenter:
    default:
        shift.setX(0);
        shift.setY(0);
        break;
    }
}

void ImageViewer::updateShift()
{
    QRectF widgetRect(rect()); //! + SIZE_SHIFT ???
    QRectF pixRect(topLeft + shift, image.size()*scale);
    if(pixRect.width() <= widgetRect.width()
            && pixRect.height() <= widgetRect.height()){
        shift = QPointF(0, 0);
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
    }

    update();
}


void ImageViewer::scrollContent(int deltaX, int deltaY)
{
    // if widget smaller than widget, allow to move image.
    if(hasPicture() && scaleLargeThanWidget()){
        shift.setX(shift.x() + deltaX);
        shift.setY(shift.y() + deltaY);
        updateShift();
    }
}

void ImageViewer::zoomIn(double factor)
{
    if(noPicture()) return;

    qreal scale_old = scale;
    scale += factor;
    scale = qMax(scaleMin, qMin(Config::ScaleMaxLimit, scale));
    if(scale == scale_old) // scale no changed
        return;

    /*! topLeft must determined before shift,
     * otherwise will impact updateShift()
     */
    calcTopLeft();
    shift /= scale_old;
    shift *= scale;
    updateShift();
    updateCursor();
    hasUserZoom = true;

    ToolTip::showText(mapToGlobal(rect().center()),
                      QString("<font size='7'><b>%1%</b></font>").arg(scale * 100, 0, 'g', 4),
                      true, 0.7, 800);
}

void ImageViewer::zoomIn(double factor, const QPoint &pivot)
{
    if(noPicture() || !rect().contains(pivot)) // pivot must inner widget
        return;

    qreal scale_old = scale;
    zoomIn(factor);

    QPointF distance(rect().center() - pivot);
    QPointF change(distance / scale_old * scale - distance);
    shift += change;    // to keep the pivot position, must after scale.
    updateShift();
}

void ImageViewer::rotatePixmap(int degree)
{
    if(noPicture()) return;

    rotate += degree;
    rotate %= 360;
    QMatrix matrix = QMatrix().rotate(degree);
    image = image.transformed(matrix, Qt::SmoothTransformation);
    layoutImage();
    update();
}

void ImageViewer::mirrored(MirrorMode mode)
{
    if(noPicture()) return;

    switch (mode) {
    case MirrorHorizontal:
        mirrorH = !mirrorH;
        break;
    case MirrorVertical:
        mirrorV = !mirrorV;
        break;
    default:
        return;
    }

    image = image.mirrored(mode == MirrorHorizontal, mode == MirrorVertical);
    layoutImage();
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

    if(noPicture() && !errStr.isEmpty()){
        painter.drawText(rect(), Qt::AlignCenter, errStr);
        return;
    }

    switch(antialiasMode){
    case Config::AntialiasWhenZoomIn:
        if(scale <= 1) break;
    case Config::AlwaysAntialias:
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        break;
    case Config::NoAntialias:
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
    if(velocityTracker->isScrolling()){
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
        calcTopLeft();
        updateShift();
        updateCursor();
    }else{
        layoutImage(); //! FIXME: if user drag and move image content (when AlignCenter is hasUserZoom and shift != (0,0)) , this will reset the shift if align mode isn't AlignCenterCenter.
    }
    QWidget::resizeEvent(e);
}

void ImageViewer::mousePressEvent ( QMouseEvent * event )
{
    velocityTracker->handleMousePress(event);

    if(event->button() & Qt::LeftButton){
        leftMousePressed = true;
        updateCursor();
    }
}

void ImageViewer::mouseMoveEvent ( QMouseEvent * event )
{
    velocityTracker->handleMouseMove(event);
}

void ImageViewer::mouseReleaseEvent ( QMouseEvent * event )
{
    velocityTracker->handleMouseRelease(event);

    if(event->button() & Qt::LeftButton){
        leftMousePressed = false;
        updateCursor();
    }
}
