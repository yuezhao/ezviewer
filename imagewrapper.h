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

#ifndef IMAGEWRAPPER_H
#define IMAGEWRAPPER_H

#include <QString>
#include <QImage>


class Cache
{
public:
    bool isValid() const { return isReady && hashCode > HASH_INVALID; }

protected:
    const static uint HASH_INVALID = 0;

    Cache() : hashCode(0), isReady(false) {}

    uint hashCode;
    bool isReady;

private: // do not copy object
    Cache(const Cache &r);
    const Cache & operator=(const Cache &r);
};


class QMovie;
class QSvgRenderer;
class ImageWrapper : public QObject, private Cache
{
    Q_OBJECT
public:
    QImage  currentImage() const;
    QString format() const { return isValid() ? imageFormat : QString::null; }
    int     frameCount() const { return isValid() ? imageFrames : 0; }
    QString attribute() const;

    bool isAnimation() const { return isValid() ? formatFlag & ANIMATION_MASK : false; }
    void startAnimation();
    void nextAnimationFrame();
    void setAnimationPaused(bool paused);
    void switchAnimationPaused();

    void recycle();

signals:
    void animationUpdate();
//   void cacheNeedReading(ImageCache *ic, const QString &filePath);

private slots:
    void updateSvgImage();

private:
    friend class ImageFactory;

    ImageWrapper() : Cache(), formatFlag(REGULAR_FLAG),
        movie(NULL), svgRender(NULL), imageFrames(0) {}
    ~ImageWrapper() { recycle(); }

    enum ImageFormatFlag {
        REGULAR_FLAG = 0x0,
        MOVIE_FLAG = 0x1,
        SVG_FLAG = 0x2,

        ANIMATION_MASK = 0xff,
    };

    uint formatFlag;

    QMovie *movie;
    QSvgRenderer *svgRender;
    QImage  image;
    QString imageFormat;
    int imageFrames;
    //QIcon *icon;
    QString imagePath;

    static bool isAnimationFromat(const QString &format);

    void readFile(const QString &filePath);
//    void callPreReadingThread(const QString &filePath)
//    {  emit cacheNeedReading(this, filePath); }
};


#endif // IMAGEWRAPPER_H
