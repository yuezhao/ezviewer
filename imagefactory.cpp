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

#include "imagefactory.h"

#include "config.h"
#include "imagewrapper.h"
#include "toolkit.h"

#include <QRunnable>
#include <QThreadPool>


/* list:
 * first one hold curCache for PicManager,
 * second one is for pre-reading (if enabled),
 * others are caches for file that had been viewed.
 */
QList<ImageWrapper *> ImageFactory::list;
int ImageFactory::CacheNumber = 0;


class Runnable : public QRunnable
{
public:
    Runnable(ImageWrapper * iw, const QString &filePath, bool preReading = true)
        : image(iw), path(filePath), isPreReading(preReading) {}

    void run() {
        if (image)
            image->load(path, isPreReading);
    }

private:
    ImageWrapper *image;
    QString path;
    bool isPreReading;
};


ImageWrapper * ImageFactory::newOrReuseImage()
{
    int total = 1 + (Config::enablePreReading() ? 1 : 0) + CacheNumber;

    ImageWrapper *image;
    if(list.size() < total){
        image = new ImageWrapper();
    }else{
        image = list.at(total - 1);
        list.removeAt(total - 1);
        image->setReady(false);
        image->setHashCode(ImageWrapper::HASH_INVALID);
    }

    return image;
}

ImageWrapper * ImageFactory::findImageByHash(uint hash)
{
    foreach(ImageWrapper *image, list){
        if(image->getHashCode() == hash){
            list.removeOne(image);
            return image;
        }
    }
    return NULL;
}

void ImageFactory::waitForImageReady(ImageWrapper *image)
{
    while(!image->getReady()){
        qApp->processEvents(QEventLoop::AllEvents);
        // wait for pre-reading in another thread
    }
}

ImageWrapper * ImageFactory::getImageWrapper(const QString &filePath)
{
    uint hash = filePath.isEmpty() ? ImageWrapper::HASH_INVALID :
                                     ToolKit::getFileHash(filePath);
    ImageWrapper *image;
    if((image = findImageByHash(hash))){
        list.prepend(image);
        // TODO: update first frame of svg animation format here (if it has been shown before).
        return image;
    }

    image = newOrReuseImage();
    image->setHashCode(hash);
    list.prepend(image);
    if (hash == ImageWrapper::HASH_INVALID) {
        image->setReady(true);
    } else {
        QThreadPool::globalInstance()->start(new Runnable(image, filePath, false));
    }

    return image;
}


void ImageFactory::preReading(const QString &filePath)
{
    if(!Config::enablePreReading())
        return;

    uint hash = ToolKit::getFileHash(filePath);
    ImageWrapper *image;
    if((image = findImageByHash(hash))){
        list.insert(1, image);  /// assert(list.size());
        return;
    }

    image = newOrReuseImage();
    image->setHashCode(hash);
    list.insert(1, image);  /// assert(list.size());
    if (hash == ImageWrapper::HASH_INVALID) {
        image->setReady(true);
    } else {
        QThreadPool::globalInstance()->start(new Runnable(image, filePath));
    }
}

void ImageFactory::freeAllCache()
{
    QThreadPool::globalInstance()->waitForDone();

    foreach(ImageWrapper *image, list)
        freeImage(image);
}

void ImageFactory::freeImage(ImageWrapper *image)
{
    waitForImageReady(image);
    delete image;
}

void ImageFactory::cacheSizeAdjusted()
{
    int total = 1 + (Config::enablePreReading() ? 1 : 0) + CacheNumber;
    while(list.size() > total){
        freeImage(list.last());
        list.removeLast();
    }
}

void ImageFactory::setCacheNumber(int val)
{
    if(val < 0 || CacheNumber == val) return;

    CacheNumber = val;
    cacheSizeAdjusted();
}

void ImageFactory::setPreReadingEnabled(bool /*enabled*/)
{
    cacheSizeAdjusted();
}

