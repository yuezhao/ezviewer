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
