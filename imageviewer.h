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

#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QDir>
#include <QFileInfoList>
#include <QTime>
#include <QTimer>


class ImageViewer : public QWidget
{
    Q_OBJECT

public:
    ImageViewer(QWidget *parent = 0);
    ~ImageViewer();

    inline bool hasPicture();
    inline bool hasFile();
    inline QString currentFile();
    QString attributeString();
//    inline bool isAnimation () { return movie; }

signals:
    void fileNameChange(const QString &fileName);
    void mouseDoubleClick();
    void showContextMenu(const QPoint &pos);
    void siteChange(const QPoint &change);

public slots:
    void changeAntialiasMode(int mode);
    void changeBgColor(const QColor &color);

    void openFile(const QString &file);
    void zoomIn(qreal factor);

    void nextPic();
    void prePic();
    inline void rotateLeft();
    inline void rotateRight();
    inline void mirrorHorizontal();
    inline void mirrorVertical();
    void copyToClipboard();
    inline void deleteFileAsk();
    inline void deleteFileNoAsk();
    void switchGifPause();
    void nextGifFrame();

protected slots:
    void wheelEvent(QWheelEvent *e);
    void paintEvent(QPaintEvent *e);
    void mouseDoubleClickEvent ( QMouseEvent * event );
    void mouseMoveEvent ( QMouseEvent * event );
    void mousePressEvent ( QMouseEvent * event );
    void mouseReleaseEvent ( QMouseEvent * event );
    void resizeEvent ( QResizeEvent * event );
    void hideEvent ( QHideEvent * event );
    void showEvent ( QShowEvent * event );
    void contextMenuEvent ( QContextMenuEvent * event );
    //    irtual void	closeEvent ( QCloseEvent * event )

private slots:
    void updatePixmap();            //use update()
    void myTimerEvent();            // for auto scroll

private:
    enum ImageState{
        NoFileNoPicture,
        PictureNoFile,
        FileNoPicture,
        FilePicture
    };

    void loadImage(const QFileInfo &fileInfo);//use repaint()

    /*! init the value of topLeft and scale, according to the size of image
     * no use update(), no impact the value of rotate or mirrorH/mirrorV.
     */
    void initToFitWidget();

    //! resize the window , according the picture's size
//    void initAdaptiveWidget();

    /*! updateShift() needs the value of topLeft,
     * so the order of these two functions below is important.
     */
    inline void updateTopLeft();    //no use update()
    void updateShift();             //use update()
    void rotatePixmap(bool isLeft);//ture left or right 90 degrees£¬use update()
    void mirrored(bool horizontal = false, bool vertical = true);

    void updateFileIndex(const QString &file);
    void noFileToShow();
    void myMouseMove(QMouseEvent * event);
    void deleteFile(bool messagebox);

private:
    ImageState state;
    int antialiasMode;
    QColor bgColor;

    QImage image;
    QMovie *movie;
    QString format;
    int frameCount;

    QPointF topLeft;   //
    qreal scale;
    qreal scaleMin; //
    int rotate;
    bool mirrorH;
    bool mirrorV;
    QPointF shift;    //
    QPoint startPos;
    bool hasUserZoom;

    QString filePath;
    QFileInfoList list;
    int currentIndex;

    bool justPressed;
    QPoint pressPos;
    QPoint delta;
    QPoint speed;
    QTime  timeStamp;
    QTimer timer;
};


inline bool ImageViewer::hasPicture()
{
    return state == FilePicture || state == PictureNoFile;
}
inline bool ImageViewer::hasFile()
{
    return state == FilePicture || state == FileNoPicture;
}
inline QString ImageViewer::currentFile()
{
    return hasFile() ? filePath : QString::null;
}
inline void ImageViewer::rotateLeft()
{
    rotatePixmap(true);
}
inline void ImageViewer::rotateRight()
{
    rotatePixmap(false);
}
inline void ImageViewer::mirrorHorizontal()
{
    mirrored(true, false);
}
inline void ImageViewer::mirrorVertical()
{
    mirrored(false, true);
}
inline void ImageViewer::deleteFileAsk()
{
    deleteFile(true);
}
inline void ImageViewer::deleteFileNoAsk()
{
    deleteFile(false);
}
inline void ImageViewer::updateTopLeft()
{
    topLeft = QPointF(rect().width() - image.width()*scale,
                      rect().height() - image.height()*scale) / qreal(2);
}

#endif
