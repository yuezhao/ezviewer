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
#include <QThread>


class ImageCache;
class PreReadingThread : public QThread
{
    Q_OBJECT
public:
    PreReadingThread() { start(); }

public slots:
    void preReading(ImageCache *cache, const QString &filePath);

//private:
//    void run() { qDebug("enter event loop..."); exec(); }
};

class ImageCache : public QObject
{
    Q_OBJECT
public:
    static ImageCache * getCache(const QString &filePath);
    static ImageCache * getNullCache();
    static void freeCache();
    static void preReading(const QString &filePath);

    QImage  image;
    QMovie *movie;
    QString format;
    int frameCount;

    friend class PreReadingThread;
    static PreReadingThread prThread;

signals:
   void cacheNeedReading(ImageCache *ic, const QString &filePath);

private:
   ImageCache()
       : movie(NULL), frameCount(0), hashCode(0), isReady(false)
   {}
    ~ImageCache()
    {
        if(movie) delete movie;
    }

   void callOtherThread(const QString &filePath);

   void readFile(const QString &filePath);

   static uint getHashCode(const QString &filePath);

   static QList<ImageCache *> list;

   uint hashCode;
   bool isReady;

private: // do not copy object
    ImageCache(const ImageCache &r);
    const ImageCache & operator=(const ImageCache &r);
};


#endif // IMAGECACHE_H
