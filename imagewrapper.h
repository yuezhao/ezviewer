/****************************************************************************
 * EZ Viewer
 * Copyright (C) 2013 huangezhao. CHINA.
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
    enum { HASH_INVALID = 0 };

    Cache() : hashCode(HASH_INVALID), isReady(false) {}

    bool isValid()     const { return hashCode > HASH_INVALID && isReady; }
    uint getHashCode() const { return hashCode; }
    bool getReady()    const { return isReady; }

    void setHashCode(uint hash) { hashCode = hash; }
    void setReady(bool ready)   { isReady = ready; }

protected:
    uint hashCode;
    volatile bool isReady;  /// important! or it may go into dead loop.

private: // do not copy object
    Cache(const Cache &r);
    const Cache & operator=(const Cache &r);
};


class ImageHeader;
class QMovie;
class QSvgRenderer;
class ImageWrapper : public QObject, public Cache
{
    Q_OBJECT
public:
    ImageWrapper();
    ~ImageWrapper();

    QImage  currentImage() const;
    QString format() const { return isValid() ? imageFormat : QString::null; }
    int     frameCount() const { return isValid() ? imageFrames : 0; }
    QString attribute();

    bool isAnimation() const { return isValid() && (formatFlag & ANIMATION_MASK); }
    void startAnimation();
    void nextAnimationFrame();
    void setAnimationPaused(bool paused);
    void switchAnimationPaused();

    void load(const QString &filePath, bool isPreReading = false);
    void recycle();

signals:
    void animationUpdated();    // image size will not changed.
    void frameUpdated();        // image size may changed.

private slots:
    void updateSvgImage();

private:
    static bool isAnimationFromat(const QString &format);
#ifdef TESTING_RAW
    static QImage loadRawImage(const QString &filePath);
#endif // TESTING_RAW

    enum ImageFormatFlag {
        REGULAR_FLAG = 0x0,
        MOVIE_FLAG = 0x1,
        SVG_FLAG = 0x2,

        ANIMATION_MASK = 0xf
    };

    QImage  image;
    QString imagePath;
    QString imageFormat;
    int     imageFrames;
    int     currentFrame; // for ico format
    QString imageAttribute;
    uint    formatFlag;
    QMovie *movie;
    QSvgRenderer *svgRender;
    ImageHeader *header;
};


#endif // IMAGEWRAPPER_H
