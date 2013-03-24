#ifndef IMAGEFACTORY_H
#define IMAGEFACTORY_H

#include <QThread>

class ImageWrapper;
class ImageFactory : public QThread
{
    Q_OBJECT
public:
    ImageFactory() { qDebug("new prereading thread"); start(); }

    static void preReading(const QString &filePath);

    static void setPreReadingEnabled(bool enabled);
    static bool preReadingEnabled(){ return prThread; }

    static ImageFactory *instance() { return prThread; }

public slots:
//    void preReading(ImageWrapper *cache, const QString &filePath)
//    { cache->readFile(filePath); }

private:
    static ImageFactory *prThread;

//    QTimer preReadingTimer;

public:
    static ImageWrapper * getImageWrapper(const QString &filePath);

    static void setCacheNumber(int val);
    static void freeAllCache();

private:
    static ImageWrapper * newOrReuseImage();
    static ImageWrapper *findImageByHash(uint hash);
    static void freeImage(ImageWrapper *image);

    static void cacheSizeAdjusted();

    static QList<ImageWrapper *> list;
    static int CacheNumber;
};

#endif // IMAGEFACTORY_H
