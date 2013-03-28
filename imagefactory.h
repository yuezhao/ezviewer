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

#ifndef IMAGEFACTORY_H
#define IMAGEFACTORY_H

#include <QString>
#include <QList>


class ImageWrapper;
class ImageFactory
{
public:
    static ImageWrapper * getImageWrapper(const QString &filePath);
    static void preReading(const QString &filePath);

    static void setCacheNumber(int val);
    static void setPreReadingEnabled(bool enabled);

    static void freeAllCache();

private:
    static ImageWrapper * newOrReuseImage();
    static ImageWrapper *findImageByHash(uint hash);
    static void freeImage(ImageWrapper *image);
    static void cacheSizeAdjusted();
    static void waitForImageReady(ImageWrapper *image);

    static QList<ImageWrapper *> list;
    static int CacheNumber;
};

#endif // IMAGEFACTORY_H
