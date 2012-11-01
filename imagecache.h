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

#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#include <QMovie>
#include <QString>
#include <QImage>


class ImageCache {
public:
    ImageCache() : movie(NULL), frameCount(0) {}
    ~ImageCache()
    {
        if(movie) delete movie;
    }

    static ImageCache * getCache(const QString &filePath);

    QImage  image;
    QMovie *movie;
    QString format;
    int frameCount;

private:
    ImageCache(const ImageCache &r);
    const ImageCache & operator=(const ImageCache &r);
};

#endif // IMAGECACHE_H
