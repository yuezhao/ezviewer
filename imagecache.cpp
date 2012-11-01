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
