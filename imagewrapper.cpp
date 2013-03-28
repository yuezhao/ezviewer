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

#include "imagewrapper.h"
#include "toolkit.h"

#ifdef USE_EXIF
#include "ExifReader.h"
using namespace PhotoKit;
#endif // USE_EXIF

#include <QDateTime>
#include <QMovie>
#include <QPainter>
#include <QSvgRenderer>


QImage ImageWrapper::currentImage() const
{
    if (!isValid())
        return QImage();

    if ((formatFlag & MOVIE_FLAG) && movie)
        return movie->currentImage();

    return image;
}

void ImageWrapper::recycle()
{
    disconnect(this, SIGNAL(animationUpdated()));
    disconnect(this, SIGNAL(frameUpdated()));
    SafeDelete(movie);
    SafeDelete(svgRender);
}

bool ImageWrapper::isAnimationFromat(const QString &format)
{
    return format == "gif" || format == "mng" || format == "svg";
}

void ImageWrapper::load(const QString &filePath, bool isPreReading)
{
    if (filePath.isEmpty())
        return;

    imagePath = filePath;
    imageAttribute = QString::null;
    formatFlag = REGULAR_FLAG;

    QImageReader reader(imagePath);
    reader.setDecideFormatFromContent(true);
    imageFormat = reader.format();
    imageFrames = reader.imageCount();

    qDebug("format is %s, frame count is %d, file name %s", qPrintable(imageFormat),
           imageFrames, qPrintable(ToolKit::filename(imagePath)));

    if(isAnimationFromat(imageFormat)){
        if (imageFormat == "svg") {
            svgRender = new QSvgRenderer(imagePath);
            if (svgRender->animated()) {
                formatFlag |= SVG_FLAG;
                image = QImage(svgRender->defaultSize(), QImage::Format_ARGB32);
                QPainter painter(&image);
                svgRender->render(&painter);
            } else {
                SafeDelete(svgRender);
            }
        } else if (imageFrames != 1) {  // "gif" or "mng" animation
            movie = new QMovie(imagePath);
            movie->setFormat(imageFormat.toLocal8Bit()); /// this is important when the file name didn't end with 'gif'.
            if(movie->isValid()) {
                formatFlag |= MOVIE_FLAG;
                if (movie->jumpToFrame(0))
                    image = movie->currentImage();
            }else{    //cannot read image, so delete
                SafeDelete(movie);
                qDebug("is animation, but creat QMovie fail");
            }
        }
    }

    if(formatFlag == REGULAR_FLAG){
        if(imageFormat == "ico"){//! is ico image has the same height and width?
            reader.read(&image);
            int maxIndex = 0;
            int maxWidth = image.width();
            for(int i=1; i < imageFrames; ++i){
                if(!reader.jumpToNextImage())
                    break;
                reader.read(&image);
                if(maxWidth < image.width()){
                    maxWidth = image.width();
                    maxIndex = i;
                }
            }
            currentFrame = maxIndex;
            reader.jumpToImage(maxIndex);
        }

        if (!reader.read(&image)) // cannot read image
            image = QImage();
    }


    if (isPreReading) { // will re-create these when animation start.
        SafeDelete(movie);
        SafeDelete(svgRender);
    }

    if(image.isNull()){
        imageFormat = "";
        imageFrames = 0;
    }

    isReady = true;
}


void ImageWrapper::startAnimation()
{
    qDebug("isAnimation, will start animation");

    if (formatFlag & MOVIE_FLAG) {
        if (!movie) {
            movie = new QMovie(imagePath);
            movie->setFormat(imageFormat.toLocal8Bit());
        }

        if(movie->state() == QMovie::NotRunning)
            movie->start();
        connect(movie, SIGNAL(updated(QRect)), SIGNAL(animationUpdated()));
    } else if (formatFlag & SVG_FLAG) {
        if (!svgRender)
            svgRender = new QSvgRenderer(imagePath);
        connect(svgRender, SIGNAL(repaintNeeded()), SLOT(updateSvgImage()));
    }
}

void ImageWrapper::updateSvgImage()
{
//    for (int i = 0; i < image.height(); ++i) {
//        quint8 * line = image.scanLine(i);
//        quint8 * end = line + image.width();
//        while(line != end)
//            *line++ = 0;
//    }
    // TODO: imporve perfermance
    image = QImage(image.size(), QImage::Format_ARGB32);
    QPainter painter(&image);
    svgRender->render(&painter);
    emit animationUpdated();
}

void ImageWrapper::switchAnimationPaused()
{
    if(movie) {
        switch(movie->state()){
        case QMovie::Running:
            movie->setPaused(true);
            break;
        case QMovie::Paused:
            movie->setPaused(false);
            break;
        default:
            break;
        }
    }
}

void ImageWrapper::nextAnimationFrame()
{
    if(movie) {
        switch(movie->state()){
        case QMovie::Running:
            movie->setPaused(true);
            break;
        case QMovie::Paused:
            movie->jumpToNextFrame();
            break;
        default:
            break;
        }
    } else if(imageFrames > 1 && imageFormat == "ico"){
        QImageReader reader(imagePath);
        reader.setDecideFormatFromContent(true);

        reader.jumpToImage(currentFrame++);
        if(!reader.jumpToNextImage()) {
            currentFrame = 0;
            reader.jumpToImage(0);
        }

        if (reader.read(&image)) {
            imageAttribute = QString::null;
            emit frameUpdated();
        }
    }
}

void ImageWrapper::setAnimationPaused(bool paused)
{
    if(movie)
        movie->setPaused(paused);
}


QString ImageWrapper::attribute()
{
    if (!imageAttribute.isEmpty())
        return imageAttribute;

    QFileInfo fileInfo(imagePath);

    if(fileInfo.exists()){
        const QString timeFormat(tr("yyyy-MM-dd, hh:mm:ss"));
        qint64 size = fileInfo.size();

        imageAttribute += tr("File Name: %1").arg(ToolKit::filename(imagePath));
        imageAttribute += "<br>" + tr("File Size: %1 (%2 Bytes)")
                .arg(ToolKit::fileSize2Str(size)).arg(size);
        imageAttribute += "<br>" + tr("Created Time: %1")
                .arg(fileInfo.created().toString(timeFormat));
    }

    QImage curImage = currentImage();
    if(!curImage.isNull()){
        if(!imageAttribute.isEmpty())
            imageAttribute += QString("<br>");

        if(!imageFormat.isEmpty())
            imageAttribute += tr("Image Format: %1").arg(imageFormat);
        if(curImage.colorCount() > 0)       // indexed
            imageAttribute += "<br>" + tr("Color Count: %1").arg(curImage.colorCount());
        else /*if(curImage.depth() >= 16)*/ // non-indexed (do not use color tables)
            imageAttribute += "<br>" + tr("Color Count: True color");
        imageAttribute += "<br>" + tr("Depth: %1").arg(curImage.depth());

        int gcd = ToolKit::gcd(curImage.width(), curImage.height());
        QString ratioStr = (gcd == 0) ? "1:1" : QString("%1:%2")
                                        .arg(curImage.width() / gcd)
                                        .arg(curImage.height() / gcd);


        imageAttribute += "<br>" + tr("Size: %1 x %2 (%3)")
                .arg(curImage.width()).arg(curImage.height()).arg(ratioStr);
        if(fileInfo.exists() && imageFrames > 1)
            imageAttribute += "<br>" + tr("Frame Count: %1").arg(imageFrames);
    }


#ifdef USE_EXIF
    if(fileInfo.exists()){
        ExifReader exif;
        exif.loadFile(imagePath);
        if (exif.hasData()) {
            ExifReader::TagInfo tags = exif.getIFD0Brief();
            if (exif.hasIFD0()) {
                QMap<QString, QString>::ConstIterator it;
                for (it = tags.begin(); it != tags.end(); ++it) {
                    if (!it.value().trimmed().isEmpty())
                        imageAttribute += "<br>" + it.key() + ": " + it.value();
                }
            }
            if (exif.hasIFDExif()) {
                tags = exif.getExifBrief();
                QMap<QString, QString>::ConstIterator it;
                for (it = tags.begin(); it != tags.end(); ++it) {
                    if (!it.value().trimmed().isEmpty())
                        imageAttribute += "<br>" + it.key() + ": " + it.value();
                }
            }

            if (exif.hasIFDGPS()) {
                tags = exif.getGpsBrief();
                QMap<QString, QString>::ConstIterator it;
                for (it = tags.begin(); it != tags.end(); ++it) {
                    if (!it.value().trimmed().isEmpty())
                        imageAttribute += "<br>" + it.key() + ": " + it.value();
                }
            }
        }
    }
#endif // USE_EXIF

    return imageAttribute;
}
