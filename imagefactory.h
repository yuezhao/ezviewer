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
