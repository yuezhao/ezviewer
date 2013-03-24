#include "imagefactory.h"

#include "config.h"
#include "imagewrapper.h"
#include "toolkit.h"


/* list:
 * first one hold curCache for PicManager,
 * second one is for pre-reading (if enabled),
 * others are caches for file that had been viewed.
 */
QList<ImageWrapper *> ImageFactory::list;
int ImageFactory::CacheNumber = 0;
ImageFactory *ImageFactory::prThread = NULL;


ImageWrapper * ImageFactory::newOrReuseImage()
{
    int total = 1 + (Config::enablePreReading() ? 1 : 0) + CacheNumber;
    qDebug("cache num is %d, total is %d, now has %d",
           ImageFactory::CacheNumber, total, list.size());

    ImageWrapper *image;
    if(list.size() < total){
        image = new ImageWrapper();
        qDebug("new a cache");
//        if(prThread){
//            QObject::connect(ic, SIGNAL(cacheNeedReading(ImageCache*,QString)),
//                    prThread, SLOT(preReading(ImageCache*,QString)));
//        }
    }else{
        image = list.at(total - 1);
        list.removeAt(total - 1);
        image->isReady = false;
        image->recycle();
        qDebug("reuse a cache");
    }

    return image;
}

ImageWrapper * ImageFactory::findImageByHash(uint hash)
{
    foreach(ImageWrapper *image, list){
        if(image->hashCode == hash){
            list.removeOne(image);
            return image;
        }
    }
    return NULL;
}


ImageWrapper * ImageFactory::getImageWrapper(const QString &filePath)
{
    uint hash = filePath.isEmpty() ? ImageWrapper::HASH_INVALID :
                                     ToolKit::getFileHash(filePath);
    ImageWrapper *image;
    if(image = findImageByHash(hash)){
        if (hash == ImageWrapper::HASH_INVALID)
            image->isReady = true;
        else
            while(!image->isReady) { /// wait for pre-reading in another thread
    ///            qApp->processEvents();
            }

        // TODO: update first frame of svg animation format here (if it has been shown before).
        list.prepend(image);
        return image;
    }

    image = newOrReuseImage();
    image->hashCode = hash;
    if (hash == ImageWrapper::HASH_INVALID)
        image->isReady = true;
    else
        image->readFile(filePath);

    list.prepend(image);
    return image;
}


void ImageFactory::freeAllCache()
{
    foreach(ImageWrapper *image, list)
        freeImage(image);

//    if(prThread){
////        prThread->quit(); // this will cause crash.
//        prThread->deleteLater();
//    }
}

void ImageFactory::freeImage(ImageWrapper *image)
{
    qDebug("delete cache");
    while(!image->isReady){
        // waiting pre-reading thread.
    }
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
    qDebug("after cache size adjust");
}



void ImageFactory::preReading(const QString &filePath)
{
    if(!prThread)
        return;

//    uint hash = ToolKit::getFileHash(filePath);
//    if(ImageCache *ic = findCache(hash)){
//        list.insert(1, ic);  /// assert(list.size());
//        return;
//    }

//    ImageCache *cache = newCache();
//    cache->hashCode = hash;
//    list.insert(1, cache);  /// assert(list.size());

//    cache->callPreReadingThread(filePath);
}

void ImageFactory::setPreReadingEnabled(bool enabled)
{
//    if(enabled){
//        if(!prThread){
//            prThread = new PreReadingThread;
//            foreach(ImageCache *ic, list)
//                QObject::connect(ic, SIGNAL(cacheNeedReading(ImageCache*,QString)),
//                                 prThread, SLOT(preReading(ImageCache*,QString)));
//        }
//    }else{
//        if(prThread){
////            prThread->quit(); // this will cause crash.
//            prThread->deleteLater();
//            prThread = NULL;
            cacheSizeAdjusted();
//        }
//    }
}

