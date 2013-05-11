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

#include "imagewrapper.h"
#include "imageheader.h"
#include "toolkit.h"

#include <libraw.h>

#include <QApplication>
#include <QDateTime>
#include <QMovie>
#include <QPainter>
#include <QSvgRenderer>

#include<QMutex>


ImageWrapper::ImageWrapper()
    : Cache(), imageFrames(0), formatFlag(REGULAR_FLAG),
      movie(NULL), svgRender(NULL), header(new ImageHeader())
{
}

ImageWrapper::~ImageWrapper()
{
    recycle();
    delete header;
}

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

    static QMutex mutex;
    QMutexLocker locker(&mutex);    //! FIXME: if the thread pool has more than one thread, when they call this function at the same time, may cause problem by the calling order.

    recycle();

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
            svgRender = new QSvgRenderer(imagePath); // this svgRender was created in pre-reading thread.
            if (svgRender->animated()) {
                formatFlag |= SVG_FLAG;
                image = QImage(svgRender->defaultSize(), QImage::Format_ARGB32);
                QPainter painter(&image);
                svgRender->render(&painter);
            }/* else {
                SafeDelete(svgRender);
            }*/
        } else if (imageFrames != 1) {  // "gif" or "mng" animation
            movie = new QMovie(imagePath);  // this movie was created in pre-reading thread.
            movie->setFormat(imageFormat.toLocal8Bit()); /// this is important when the file name didn't end with 'gif'.
            if(movie->isValid()) {
                formatFlag |= MOVIE_FLAG;
                if (movie->jumpToFrame(0))
                    image = movie->currentImage();
            }/*else{    //cannot read image, so delete
                SafeDelete(movie);
            }*/
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

        QImage ret = loadRawImage(filePath);
        if (!ret.isNull())
            image = ret;
        else
        if (!reader.read(&image))
            image = QImage();    // cannot read image

        if (!image.isNull() && ImageHeader::isFormatSupport(imageFormat)) {
            if (header->loadFile(imagePath) && header->hasOrientation()) {
                header->autoRotateImage(image);
            }
        }
    }

//    if (isPreReading) { // will re-create these when animation start.
        SafeDelete(movie);
        SafeDelete(svgRender);
//    }

    if(image.isNull()){
        imageFormat = "";
        imageFrames = 0;
    }

    isReady = true;
}

QImage ImageWrapper::loadRawImage(const QString &filePath)
{
    LibRaw *raw = new LibRaw;
    QImage rawImage;
    if (raw->open_file(filePath.toLocal8Bit()) != LIBRAW_SUCCESS) {
        delete raw;
        return rawImage;
    }

    QSize defaultSize = QSize(raw->imgdata.sizes.width,
                              raw->imgdata.sizes.height);
    if (raw->imgdata.sizes.flip == 5 || raw->imgdata.sizes.flip == 6) {
        defaultSize.transpose();
    }

    const libraw_data_t &imgdata = raw->imgdata;
    libraw_processed_image_t *output;
    if (defaultSize.width() < imgdata.thumbnail.twidth ||
            defaultSize.height() < imgdata.thumbnail.theight) {
        qDebug("Using thumbnail");
        raw->unpack_thumb();
        output = raw->dcraw_make_mem_thumb();
    } else {
        qDebug("Decoding raw data");
        raw->unpack();
        raw->dcraw_process();
        output = raw->dcraw_make_mem_image();
    }

    QImage unscaled;
    uchar *pixels = 0;
    if (output->type == LIBRAW_IMAGE_JPEG) {
        qDebug("libraw_image_jpeg");
        unscaled.loadFromData(output->data, output->data_size, "JPEG");
        if (imgdata.sizes.flip != 0) {
            QTransform rotation;
            int angle = 0;
            if (imgdata.sizes.flip == 3) angle = 180;
            else if (imgdata.sizes.flip == 5) angle = -90;
            else if (imgdata.sizes.flip == 6) angle = 90;
            if (angle != 0) {
                rotation.rotate(angle);
                unscaled = unscaled.transformed(rotation);
            }
        }
    } else {
        qDebug("get raw data, will convert");
        int numPixels = output->width * output->height;
        int colorSize = output->bits / 8;
        int pixelSize = output->colors * colorSize;
        pixels = new uchar[numPixels * 4];
        uchar *data = output->data;
        for (int i = 0; i < numPixels; i++, data += pixelSize) {
            if (output->colors == 3) {
                pixels[i * 4] = data[2 * colorSize];
                pixels[i * 4 + 1] = data[1 * colorSize];
                pixels[i * 4 + 2] = data[0];
            } else {
                pixels[i * 4] = data[0];
                pixels[i * 4 + 1] = data[0];
                pixels[i * 4 + 2] = data[0];
            }
        }
        unscaled = QImage(pixels,
                          output->width, output->height,
                          QImage::Format_RGB32);
    }

    if (unscaled.size() != defaultSize) {
        // TODO: use quality parameter to decide transformation method
        rawImage = unscaled.scaled(defaultSize, Qt::IgnoreAspectRatio,
                                   Qt::SmoothTransformation);
        qDebug("after scale");
    } else {
        rawImage = unscaled;
        if (output->type == LIBRAW_IMAGE_BITMAP) {
            // make sure that the bits are copied
            uchar *b = rawImage.bits();
            Q_UNUSED(b);
        }
    }
    raw->dcraw_clear_mem(output);
    delete pixels;

    delete raw;
    return rawImage;
}

/*! NOTE: QMovie use QTimer for changing frame, and QTimer muse start in the thread that it has been created.
 *  Since QTimer has parent(QMovie) and we couldn't use moveToThread with it (it is a d-pointer member of QMovie),
 *  so muse create the movie in main thread.
 */
void ImageWrapper::startAnimation()
{
    qDebug("isAnimation, will start animation");

    if (formatFlag & MOVIE_FLAG) {
        if (!movie) {
            movie = new QMovie(imagePath); // this movie was created in main thread.
            movie->setFormat(imageFormat.toLocal8Bit());
        }

        if(movie->state() == QMovie::NotRunning)
            movie->start();
        connect(movie, SIGNAL(updated(QRect)), SIGNAL(animationUpdated()));
    } else if (formatFlag & SVG_FLAG) {
        if (!svgRender)
            svgRender = new QSvgRenderer(imagePath); // this svgRender was created in main thread.
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
        qint64 size = fileInfo.size();
        imageAttribute += tr("File name: %1").arg(ToolKit::filename(imagePath));
        imageAttribute += "<br>" + tr("File size: %1 (%2 bytes)")
                .arg(ToolKit::fileSize2Str(size)).arg(size);
        imageAttribute += "<br>" + tr("Created time: %1")
                .arg(fileInfo.created().toString(tr("yyyy-MM-dd, hh:mm:ss")));
    }

    QImage curImage = currentImage();
    if(!curImage.isNull()){
        if(!imageAttribute.isEmpty())
            imageAttribute += QString("<br>");

        if(!imageFormat.isEmpty())
            imageAttribute += tr("Image format: %1").arg(imageFormat);

        int gcd = ToolKit::gcd(curImage.width(), curImage.height());
        QString ratioStr = (gcd == 0) ? "1:1" : QString("%1:%2")
                                        .arg(curImage.width() / gcd)
                                        .arg(curImage.height() / gcd);
        imageAttribute += "<br>" + tr("Image size: %1 x %2 (%3)")
                .arg(curImage.width()).arg(curImage.height()).arg(ratioStr);

//        if(curImage.colorCount() > 0)       // indexed
//            imageAttribute += "<br>" + tr("Color count: %1").arg(curImage.colorCount());
//        else /*if(curImage.depth() >= 16)*/ // non-indexed (do not use color tables)
//            imageAttribute += "<br>" + tr("Color count: true color");
//        imageAttribute += "<br>" + tr("Depth: %1").arg(curImage.depth());

        if(fileInfo.exists() && imageFrames > 1)
            imageAttribute += "<br>" + tr("Frame count: %1").arg(imageFrames);
    }

    if (ImageHeader::isFormatSupport(imageFormat) && header->loadFile(imagePath)) {
        if (header->isJpeg()) {
            if (header->hasQuality()) // even if there is no exif tag, the quality info may also exist.
                imageAttribute += "<br>" + tr("JPEG Quality: %1").arg(header->quality());
        }
        if (header->hasExif()) {
            if (header->hasOrientation() && (header->orientation() != ImageHeader::NORMAL))
                imageAttribute += "<br>" + tr("Orientation: %1").arg(header->orientationString());
            if (header->hasSoftware())
                imageAttribute += "<br>" + tr("Software: %1").arg(header->software());
            if (header->hasMake())
                imageAttribute += "<br>" + tr("Camera make: %1").arg(header->make());
            if (header->hasModel())
                imageAttribute += "<br>" + tr("Camera model: %1").arg(header->model());
            if (header->hasDateTimeOriginal())
                imageAttribute += "<br>" + tr("Original date/time: %1").arg(header->dateTimeOriginal());
            if (header->hasFNumber())
                imageAttribute += "<br>" + tr("F-stop: f/%1").arg(header->fNumber(), 0, 'f', 1);
            if (header->hasExposureTime())
                imageAttribute += "<br>" + tr("Exposure time: 1/%1 s").arg((unsigned) (1.0/header->exposureTime()));
            if (header->hasISOSpeed())
                imageAttribute += "<br>" + tr("ISO speed: ISO-%1").arg(header->ISOSpeed());
            if (header->hasExposureBias())
                imageAttribute += "<br>" + tr("Exposure bias: %1 EV").arg(header->exposureBias());
            if (header->hasFocalLength())
                imageAttribute += "<br>" + tr("Lens focal length: %1 mm").arg(header->focalLength());
            // if (header->focalLengthIn35mm())
            //      imageAttribute += "<br>" + tr("35mm focal length: %1 mm").arg(header->focalLengthIn35mm());
            if (header->hasMeteringMode())
                imageAttribute += "<br>" + tr("Metering mode: %1").arg(header->meteringMode());
            if (header->hasSubjectDistance())
                imageAttribute += "<br>" + tr("Subject distance: %1 m").arg(header->subjectDistance());
            if (header->hasFlash())
                imageAttribute += "<br>" + tr("Flash mode: %1").arg(header->flashMode());
            if (header->hasLightSource())
                imageAttribute += "<br>" + tr("Light source: %1").arg(header->lightSource());
            if (header->hasExposureProgram())
                imageAttribute += "<br>" + tr("Exposure program: %1").arg(header->exposureProgram());
            if (header->hasWhiteBalance())
                imageAttribute += "<br>" + tr("White balance: %1").arg(header->whiteBalance());
            if (header->hasGPSLatitude())
                imageAttribute += "<br>" + tr("GPS Latitude: %1").arg(header->GPSLatitudeString());
            if (header->hasGPSLongitude())
                imageAttribute += "<br>" + tr("GPS Longitude: %1").arg(header->GPSLongitudeString());
            if (header->hasGPSAltitude())
                imageAttribute += "<br>" + tr("GPS Altitude: %1").arg(header->GPSAltitudeString());
        }
    }

    return imageAttribute;
}
