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

#include "imagecache.h"
#include "toolkit.h"

#include <QDateTime>
#include <QHash>

const int CacheNumber = 4;

QList<ImageCache *> ImageCache::list;

PreReadingThread ImageCache::prThread;

// 复用cache?
// 更多线程？

uint ImageCache::getHashCode(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    uint hash = 0;
    if(fileInfo.exists())
        hash = qHash(filePath) + qHash(fileInfo.size())
                + qHash(fileInfo.created().toTime_t())
                + qHash(fileInfo.lastModified().toTime_t())
                + qHash(fileInfo.lastRead().toTime_t());
    return hash;
}

ImageCache * ImageCache::getNullCache()
{
    ImageCache *result = new ImageCache();
    result->isReady = true;

    if(list.size() == CacheNumber){
        delete list.at(3);
        list.removeAt(3);
    }
    list.prepend(result);

    qDebug("new a null ImageCache, now list size is %d", list.size());

    return result;
}

void ImageCache::freeCache()    /// 需要互斥？
{
    foreach(ImageCache *ic, list)
        delete ic;

    qDebug("free all ImageCache");

    prThread.quit();
}

ImageCache * ImageCache::getCache(const QString &filePath)
{
    uint hash = getHashCode(filePath);
    foreach(ImageCache *ic, list){
        if(ic->hashCode == hash){
            list.removeOne(ic);       /// 需要互斥？
            list.prepend(ic);       /// 需要互斥？

            qDebug("find exist ImageCache, now list size is %d", list.size());

            while(!ic->isReady) {} /// wait for pre-reading in another thread

            return ic;
        }
    }

    ImageCache *result = new ImageCache();
    result->hashCode = hash;
    result->readFile(filePath);

    if(list.size() == CacheNumber){
        delete list.at(3);
        list.removeAt(3);

        qDebug("delete 4th ImageCache, now list size is %d", list.size());
    }
    list.prepend(result);

    qDebug("new a ImageCache, now list size is %d", list.size());

    return result;
}

void ImageCache::preReading(const QString &filePath)
{
    uint hash = getHashCode(filePath);
    foreach(ImageCache *ic, list){
        if(ic->hashCode == hash){
            list.removeOne(ic);       /// 需要互斥？
            list.prepend(ic);       /// 需要互斥？

            qDebug("prereading: find exist ImageCache, now list size is %d", list.size());

            return;
        }
    }

    ImageCache *result = new ImageCache();
    result->hashCode = hash;

    if(list.size() == CacheNumber){
        delete list.at(3);
        list.removeAt(3);

        qDebug("prereading: delete 4th ImageCache, now list size is %d", list.size());
    }
    list.prepend(result);

    qDebug("prereading: new a ImageCache, now list size is %d", list.size());

    result->callOtherThread(filePath);
}

void ImageCache::callOtherThread(const QString &filePath)
{
    connect(this, SIGNAL(cacheNeedReading(ImageCache*,QString)),
            &prThread, SLOT(preReading(ImageCache*,QString)));
    emit cacheNeedReading(this, filePath);
}

void ImageCache::readFile(const QString &filePath)
{
    QImageReader reader(filePath);
    reader.setDecideFormatFromContent(true);
    format = reader.format();
    frameCount = reader.imageCount();

    if(format == "gif"){
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

        reader.read(&image);
//        if (!reader.read(&image)) // cannot read image
//            image = QImage();   ///

    }else if(/*format == "gif" && */ frameCount == 1){
        SafeDelete(movie);
    }

    isReady = true;
}

/**********************************************************************
 *
 * PrefetchThread
 *
 **********************************************************************/

void PreReadingThread::preReading(ImageCache *cache, const QString &filePath)
{
    cache->readFile(filePath);
    qDebug("pre reading file success");
}
