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
#include "ExifReader.h"
using namespace PhotoKit;

#include <QDateTime>
#include <QMovie>
#include <QPainter>
#include <QSvgRenderer>


void ImageWrapper::recycle()
{
    disconnect(this, SIGNAL(animationUpdate()));

    if(movie){
        movie->stop();
        disconnect(movie);
        SafeDelete(movie);
    } else if (svgRender) {
        disconnect(svgRender);
        SafeDelete(svgRender);
    }
    ////////////////////////////////////////////////
}


bool ImageWrapper::isAnimationFromat(const QString &format)
{
    return format == "gif" || format == "mng" || format == "svg";
}

QImage ImageWrapper::currentImage() const
{
    if (!isValid())
        return QImage();

    if (formatFlag & MOVIE_FLAG)
        return movie->currentImage();

    return image; // REGULAR_FLAG or SVG_FLAG
}

void ImageWrapper::readFile(const QString &filePath)
{
    QImageReader reader(filePath);
    reader.setDecideFormatFromContent(true);
    imageFormat = reader.format();
    imageFrames = reader.imageCount();
    formatFlag = REGULAR_FLAG;
    imagePath = filePath;

    qDebug("format is %s, frame count is %d, file name %s", qPrintable(imageFormat),
           imageFrames, qPrintable(ToolKit::filename(filePath)));

    if(isAnimationFromat(imageFormat)){
        if (imageFormat == "svg") {
            svgRender = new QSvgRenderer(filePath);
            if (svgRender->animated()) {
                formatFlag |= SVG_FLAG;
                image = QImage(svgRender->defaultSize(), QImage::Format_ARGB32);
                QPainter painter(&image);
                svgRender->render(&painter);
            } else {
                SafeDelete(svgRender);
            }
        } else if (/*format == "gif" && */ imageFrames != 1) {
            movie = new QMovie(filePath);
            if(movie->isValid()) {
                formatFlag |= MOVIE_FLAG;
                if (movie->jumpToFrame(0))
                    image = movie->currentImage();
            }else{    //cannot read image, so delete
                SafeDelete(movie);
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
            reader.jumpToImage(maxIndex);
        }

        if (!reader.read(&image)) // cannot read image
            image = QImage();   ///
    }

// check if in prereading thread ? if in main thread, do not delete these.
    SafeDelete(movie);
    SafeDelete(svgRender);

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
        if (!movie)
            movie = new QMovie(imagePath);

        if(movie->state() == QMovie::NotRunning)
            movie->start();
        connect(movie, SIGNAL(updated(QRect)), SIGNAL(animationUpdate()));
    } else if (formatFlag & SVG_FLAG) {
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
    emit animationUpdate();
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
        case QMovie::NotRunning:
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
        case QMovie::NotRunning:
        default:
            break;
        }
    }
}

void ImageWrapper::setAnimationPaused(bool paused)
{
    if(movie)
        movie->setPaused(paused);
}


QString ImageWrapper::attribute() const
{
    QString info;
    QFileInfo fileInfo(imagePath);

    if(fileInfo.exists()){
        const QString timeFormat(tr("yyyy-MM-dd, hh:mm:ss"));
        qint64 size = fileInfo.size();

        info += tr("File Name: %1").arg(ToolKit::filename(imagePath));
        info += "<br>" + tr("File Size: %1 (%2 Bytes)").arg(ToolKit::fileSize2Str(size)).arg(size);
        info += "<br>" + tr("Created Time: %1")
                .arg(fileInfo.created().toString(timeFormat));
    }

    QImage curImage = currentImage();
    if(!curImage.isNull()){
        if(!info.isEmpty())
            info += QString("<br>");

        if(!imageFormat.isEmpty())
            info += tr("Image Format: %1").arg(imageFormat);
        if(curImage.colorCount() > 0)       // indexed
            info += "<br>" + tr("Color Count: %1").arg(curImage.colorCount());
        else /*if(curImage.depth() >= 16)*/ // non-indexed (do not use color tables)
            info += "<br>" + tr("Color Count: True color");
        info += "<br>" + tr("Depth: %1").arg(curImage.depth());

        int gcd = ToolKit::gcd(curImage.width(), curImage.height());
        QString ratioStr = (gcd == 0) ? "1:1" : QString("%1:%2")
                                        .arg(curImage.width() / gcd)
                                        .arg(curImage.height() / gcd);


        info += "<br>" + tr("Size: %1 x %2 (%3)")
                .arg(curImage.width())
                .arg(curImage.height())
                .arg(ratioStr);
        if(fileInfo.exists() && imageFrames != 1)
            info += "<br>" + tr("Frame Count: %1").arg(imageFrames);
    }


    if(fileInfo.exists()){
        ExifReader exif;
        exif.loadFile(imagePath);
        if (exif.hasData()) {
            ExifReader::TagInfo tags = exif.getIFD0Brief();
            if (exif.hasIFD0()) {
                QMap<QString, QString>::ConstIterator it;
                for (it = tags.begin(); it != tags.end(); ++it) {
                    if (!it.value().trimmed().isEmpty())
                        info += "<br>" + it.key() + ": " + it.value();
                }
            }
            if (exif.hasIFDExif()) {
                tags = exif.getExifBrief();
                QMap<QString, QString>::ConstIterator it;
                for (it = tags.begin(); it != tags.end(); ++it) {
                    if (!it.value().trimmed().isEmpty())
                        info += "<br>" + it.key() + ": " + it.value();
                }
            }

            if (exif.hasIFDGPS()) {
                tags = exif.getGpsBrief();
                QMap<QString, QString>::ConstIterator it;
                for (it = tags.begin(); it != tags.end(); ++it) {
                    if (!it.value().trimmed().isEmpty())
                        info += "<br>" + it.key() + ": " + it.value();
                }
            }
        }
    }

    return info;
}
