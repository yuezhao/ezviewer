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

    // use repaint()
    void loadImage(const QImage &im,
                   const QString &msg_if_no_image = QString::null);
    // use update(), no change member like scale, shift
    void updatePixmap(const QImage &image);

    bool hasPicture() const                 { return !image.isNull(); }
    bool noPicture()  const                 { return image.isNull(); }
    qreal currentScale() const              { return scale; }

public slots:
    void changeScaleMode(int mode);
    void changeAlignMode(int mode);
    void changeAntialiasMode(int mode);
    // if color is invalid, means disabled custom background color.
    void changeBgColor(const QColor &color);

    void moveContent(int deltaX, int deltaY);
    void zoomIn(double factor); // pivot is center of this widget.
    void zoomIn(double factor, const QPoint &pivot);
    void rotateLeft()       { rotatePixmap(-90); }
    void rotateRight()      { rotatePixmap(90); }
    void mirrorHorizontal() { mirrored(MirrorHorizontal); }
    void mirrorVertical()   { mirrored(MirrorVertical); }
    void copyToClipboard();

protected slots:
    void paintEvent(QPaintEvent *e);
    void resizeEvent ( QResizeEvent * event );

    void mouseMoveEvent ( QMouseEvent * event );
    void mousePressEvent ( QMouseEvent * event );
    void mouseReleaseEvent ( QMouseEvent * event );

private slots:
    void myTimerEvent();            // for auto scroll

private:
    bool scaleLargeThanWidget();

    void updateImageArea()
    { update(QRect((topLeft + shift).toPoint(), image.size()*scale)); }

    /*! init the value of topLeft and scale, according to the size of image
     * no use update(), no impact the value of rotate or mirrorH/mirrorV.
     */
    void layoutImage(); // determine the visible rect of the image.

    void calcScaleRatio();
    /*! updateShift() needs the value of topLeft,
     * so the order of these two functions below is important.
     */
    void calcTopLeft();    // no use update()
    void calcShift();      // no use update()
    void updateShift();    // use update()

    void rotatePixmap(int degree); // use update()
    enum MirrorMode {
        MirrorHorizontal = 0,
        MirrorVertical
    };
    void mirrored(MirrorMode mode);

    void myMouseMove(QMouseEvent * event);

private:
    QImage image;
    QString errStr; // msg to show if image is null.

    int scaleMode;
    int alignMode;
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


inline void ImageViewer::calcTopLeft()
{
    topLeft.setX((rect().width() - image.width() * scale) / qreal(2));
    topLeft.setY((rect().height() - image.height() * scale ) / qreal(2));
}


#endif
