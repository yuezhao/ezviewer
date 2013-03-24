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

#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QTime>
#include <QTimer>


class ImageViewer : public QWidget
{
    Q_OBJECT

public:
    ImageViewer(QWidget *parent = 0);

    //use repaint()
    void loadImage(const QImage &im,
                   const QString &msg_if_no_image = QString::null);
    //use update(), no change member like scale, shift
    void updatePixmap(const QImage &image);

    bool hasPicture() const                 { return !image.isNull(); }
    bool noPicture()  const                 { return image.isNull(); }
    qreal currentScale() const              { return scale; }
    const QColor & backgroundColor() const  { return bgColor; }
    int AntialiasMode() const               { return antialiasMode; }

signals:
    void siteChange(const QPoint &change);

public slots:
    void changeAntialiasMode(int mode);
    //! if color is invalid, means disabled custom background color.
    void changeBgColor(const QColor &color);

    void zoomIn(qreal factor);
    void rotateLeft()       { rotatePixmap(true); }
    void rotateRight()      { rotatePixmap(false); }
    void mirrorHorizontal() { mirrored(true, false); }
    void mirrorVertical()   { mirrored(false, true); }
    void copyToClipboard();

protected slots:
    void wheelEvent(QWheelEvent *e);
    void paintEvent(QPaintEvent *e);
    void mouseMoveEvent ( QMouseEvent * event );
    void mousePressEvent ( QMouseEvent * event );
    void mouseReleaseEvent ( QMouseEvent * event );
    void resizeEvent ( QResizeEvent * event );

private slots:
    void myTimerEvent();            // for auto scroll

private:
    /*! init the value of topLeft and scale, according to the size of image
     * no use update(), no impact the value of rotate or mirrorH/mirrorV.
     */
    void initToFitWidget();

    void updateImageArea()
    { update(QRect((topLeft + shift).toPoint(), image.size()*scale)); }

    /*! updateShift() needs the value of topLeft,
     * so the order of these two functions below is important.
     */
    void updateTopLeft();    // no use update()
    void updateShift();      // use update()
    void rotatePixmap(bool isLeft);//ture left or right 90 degrees£¬use update()
    void mirrored(bool horizontal = false, bool vertical = true);

    void myMouseMove(QMouseEvent * event);

private:
    QImage image;
    QString errStr; // msg to show if image is null.

    int antialiasMode;
    QColor bgColor;

    QPointF topLeft;   //
    qreal scale;
    qreal scaleMin; //
    int rotate;
    bool mirrorH;
    bool mirrorV;
    QPointF shift;    //
    QPoint startPos;
    bool hasUserZoom;

    bool justPressed;
    QPoint pressPos;
    QPoint delta;
    QPoint speed;
    QTime  timeStamp;
    QTimer timer;
};

inline void ImageViewer::updateTopLeft()
{
    topLeft = QPointF(rect().width() - image.width()*scale,
                      rect().height() - image.height()*scale ) / qreal(2);
}

#endif
