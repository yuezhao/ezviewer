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

#include "imagecache.h"
#include "toolkit.h"

#include <QApplication>
#include <QDateTime>
#include <QHash>

/* list:
 * first one hold curCache for PicManager,
 * second one is for pre-reading (if enabled),
 * others are caches for file that had been viewed.
 */
QList<ImageCache *> ImageCache::list;
int ImageCache::CacheNumber = 0;
PreReadingThread *ImageCache::prThread = NULL;


uint ImageCache::getHashCode(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    uint hash = 0;
    if(fileInfo.exists())
        hash = qHash(filePath) + qHash(fileInfo.size())
                + qHash(fileInfo.created().toTime_t())
                + qHash(fileInfo.lastModified().toTime_t());
    return hash;
}

void ImageCache::freeCache()
{
    foreach(ImageCache *ic, list){
        while(!ic->isReady){
            // waiting pre-reading thread.
        }
        delete ic;
    }

    if(prThread){
//        prThread->quit(); // this will cause crash.
        prThread->deleteLater();
    }
}

ImageCache * ImageCache::newCache()
{
    int total = 1 + (prThread ? 1 : 0) + CacheNumber;

    ImageCache *ic;
    if(list.size() < total){
        ic = new ImageCache;
        if(prThread){
            QObject::connect(ic, SIGNAL(cacheNeedReading(ImageCache*,QString)),
                    prThread, SLOT(preReading(ImageCache*,QString)));
        }
    }else{
        ic = list.at(total - 1);
        list.removeAt(total - 1);
        ic->isReady = false;
        SafeDelete(ic->movie);
    }

    return ic;
}

ImageCache * ImageCache::findCache(uint hash)
{
    foreach(ImageCache *ic, list){
        if(ic->hashCode == hash){
            list.removeOne(ic);
            return ic;
        }
    }
    return NULL;
}

ImageCache * ImageCache::getNullCache()
{
    ImageCache *ic = findCache(0);
    if(!ic){
        ic = newCache();
        if(ic->hashCode == 0){
            ic->isReady = true;
        }else{
            ic->image = QImage();
            ic->format = QString::null;
            ic->frameCount = 0;
        }
    }

    list.prepend(ic);
    return ic;
}

ImageCache * ImageCache::getCache(const QString &filePath)
{
    uint hash = getHashCode(filePath);
    if(ImageCache *ic = findCache(hash)){
        list.prepend(ic);

        while(!ic->isReady) { /// wait for pre-reading in another thread
///            qApp->processEvents();
        }
        return ic;
    }

    ImageCache *cache = newCache();
    cache->hashCode = hash;
    cache->readFile(filePath);
    list.prepend(cache);

    return cache;
}

void ImageCache::preReading(const QString &filePath)
{
    if(!prThread)
        return;

    uint hash = getHashCode(filePath);
    if(ImageCache *ic = findCache(hash)){
        list.insert(1, ic);  /// assert(list.size());
        return;
    }

    ImageCache *cache = newCache();
    cache->hashCode = hash;
    list.insert(1, cache);  /// assert(list.size());

    cache->callPreReadingThread(filePath);
}

void ImageCache::cacheSizeAdjusted()
{
    int total = 1 + (prThread ? 1 : 0) + CacheNumber;
    while(list.size() > total){
        ImageCache *ic = list.last();
        list.removeLast();
//!        while(!ic->isReady) { // if in using by pre-reading ?
//!            // do nothing
//!        }
        delete ic;
    }
}

void ImageCache::setCacheNumber(int val)
{
    if(val < 0 || CacheNumber == val) return;

    CacheNumber = val;
    cacheSizeAdjusted();
}

void ImageCache::setPreReadingEnabled(bool enabled)
{
    if(enabled){
        if(!prThread){
            prThread = new PreReadingThread;
            foreach(ImageCache *ic, list)
                QObject::connect(ic, SIGNAL(cacheNeedReading(ImageCache*,QString)),
                                 prThread, SLOT(preReading(ImageCache*,QString)));
        }
    }else{
        if(prThread){
//            prThread->quit(); // this will cause crash.
            prThread->deleteLater();
            prThread = NULL;
            cacheSizeAdjusted();
        }
    }
}

void ImageCache::readFile(const QString &filePath)
{
    QImageReader reader(filePath);
    reader.setDecideFormatFromContent(true);
    format = reader.format();
    frameCount = reader.imageCount();

    qDebug("format is %s", qPrintable(format));

    if(format == "gif" || format == "mng"){   /// if(isAnimation())...///////////
        movie = new QMovie(filePath);
        if(movie->isValid()){
            if(movie->state() == QMovie::NotRunning)
                movie->start(); ///
            image = movie->currentImage();
            movie->stop();  ///
        }else{    //cannot read image, so delete
            SafeDelete(movie);
        }
    }

    if(!movie){
        if(format == "ico"){//! is ico image has the same height and width?
            reader.read(&image);
            int maxIndex = 0;
            int maxWidth = image.width();
            for(int i=1; i < frameCount; ++i){
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

    }else if(/*format == "gif" && */ frameCount == 1){
        SafeDelete(movie);
    }

    if(image.isNull()){
        format = "";
        frameCount = 0;
    }
    isReady = true;
}
