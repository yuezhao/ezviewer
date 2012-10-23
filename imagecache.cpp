#include "imagecache.h"
#include "global.h"


ImageCache * ImageCache::getCache(const QString &filePath)
{
    ImageCache *result = new ImageCache();
    QImage   &image = result->image;
    QMovie * &movie = result->movie;
    QString &format = result->format;
    int &frameCount = result->frameCount;

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

        if (!reader.read(&image)) // cannot read image
            image = QImage();   ///

    }else if(/*format == "gif" && */ frameCount == 1){
        SafeDelete(movie);
    }

    return result;
}
