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

#include "imageheader.h"

#include <QDataStream>
#include <QDateTime>
#include <QFile>
#include <QImage>
#include <QMatrix>


const uchar M_SOI = 0xD8;   // Start Of Image (beginning of datastream)
const uchar M_EOI = 0xD9;   // End Of Image (end of datastream)
const uchar M_SOS = 0xDA;   // Start Of Scan (begins compressed data)
const uchar M_EXIF = 0xE1;  // Exif marker.  Also used for XMP data!
const uchar M_DQT = 0xDB;   // Define Quantization Table


// Return the jpeg quality guessed. If return -1, means cann't get jpeg quility.
extern int process_DQT (const uchar * Data, int length); // from jpgguess.c


ImageHeader::ImageHeader()
    : type(TYPE_UNKNOWN), jpegQualityGuess(-1), exif(new EXIFInfo())
{
}

ImageHeader::~ImageHeader()
{
    delete exif;
}

bool ImageHeader::isFormatSupport(const QString &imageFormat)
{
    QString format = imageFormat.toLower();
    return format == "jpg" || format == "jpeg";
}

// Return true if file type can be detected. Otherwise return false.
bool ImageHeader::loadFile(const QString &path)
{
    if (lastLoadFile == path)    // this file has been load.
        return type;

    lastLoadFile = path;
    type = TYPE_UNKNOWN;
    jpegQualityGuess = -1;
    exif->clear();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    if (parseJpegFile(file)) {
        type |= TYPE_JPEG;
        return true;
    }

    return false;
}


// This function reference : https://abf.rosalinux.ru/uxteam/KLook/blob/master/src/rotatedimage.cpp
/* Explanation extracted from http://sylvana.net/jpegcrop/exif_orientation.html

   For convenience, here is what the letter F would look like if it were tagged
   correctly and displayed by a program that ignores the orientation tag (thus
   showing the stored image):

  1        2       3      4         5            6           7          8

888888  888888      88  88      8888888888  88                  88  8888888888
88          88      88  88      88  88      88  88          88  88      88  88
8888      8888    8888  8888    88          8888888888  8888888888          88
88          88      88  88
88          88  888888  888888

*/
void ImageHeader::autoRotateImage(QImage &image)
{
    if (exif->Orientation <= NORMAL || exif->Orientation > ROT_270)
        return;

    QMatrix matrix;
    switch(exif->Orientation)
    {
    case HFLIP:
        matrix.scale(-1, 1);
        break;
    case ROT_180:
        matrix.rotate(180);
        break;
    case VFLIP:
        matrix.scale(1, -1);
        break;
    case TRANSPOSE:
        matrix.scale(-1, 1);
        matrix *= QMatrix().rotate(90);
        break;
    case ROT_90:
        matrix.rotate(90);
        break;
    case TRANSVERSE:
        matrix.scale(1, -1);
        matrix *= QMatrix().rotate(90);
        break;
    case ROT_270:
        matrix.rotate(-90);
        break;
    }

    image = image.transformed(matrix, Qt::SmoothTransformation);
}


QString ImageHeader::orientationString() const
{
    switch (exif->Orientation) {
    case NORMAL:    return QObject::tr("normal");
    case HFLIP:     return QObject::tr("flip horizontal");  // left right reversed mirror
    case ROT_180:   return QObject::tr("rotate 180");
    case VFLIP:     return QObject::tr("flip vertical");    // upside down mirror
    case TRANSPOSE: return QObject::tr("transpose");        // Flipped about top-left <--> bottom-right axis.
    case ROT_90:    return QObject::tr("rotate 90 CW");     // rotate 90 cw to right it.
    case TRANSVERSE: return QObject::tr("transverse");       // flipped about top-right <--> bottom-left axis
    case ROT_270:   return QObject::tr("rotate 270 CW");    // rotate 270 to right it.
    default:        return QString::null;
    }
}

QString ImageHeader::dateTimeOriginal() const
{
    QString original = QString(exif->DateTimeOriginal.c_str()).trimmed();
    const char *OriginalFormat = "yyyy:MM:dd hh:mm:ss";
    QDateTime datetime = QDateTime::fromString(original, OriginalFormat);
    if (!datetime.isValid())
        return original;

    return datetime.toString(QObject::tr("yyyy-MM-dd, hh:mm:ss"));
}

QString ImageHeader::flashMode() const
{
    if (exif->Flash == -1)
        return QString::null;

    QString str;
    if (exif->Flash & 1){
        str = QObject::tr("flash fired");
        switch (exif->Flash){
        case 0x5: str += QObject::tr(", strobe light not detected"); break;
        case 0x7: str += QObject::tr(", strobe light detected"); break;
        case 0x9: str += QObject::tr(", compulsory"); break;
        case 0xd: str += QObject::tr(", compulsory, return light not detected"); break;
        case 0xf: str += QObject::tr(", compulsory, return light detected"); break;
        case 0x19:str += QObject::tr(", auto"); break;
        case 0x1d:str += QObject::tr(", auto, return light not detected"); break;
        case 0x1f:str += QObject::tr(", auto, return light detected"); break;
        case 0x41:str += QObject::tr(", red-eye reduction mode"); break;
        case 0x45:str += QObject::tr(", red-eye reduction mode, return light not detected"); break;
        case 0x47:str += QObject::tr(", red-eye reduction mode, return light detected"); break;
        case 0x49:str += QObject::tr(", compulsory, red-eye reduction mode"); break;
        case 0x4d:str += QObject::tr(", compulsory, red-eye reduction mode, return light not detected"); break;
        case 0x4f:str += QObject::tr(", red-eye reduction mode, return light detected"); break;
        case 0x59:str += QObject::tr(", auto, red-eye reduction mode"); break;
        case 0x5d:str += QObject::tr(", auto, red-eye reduction mode, return light not detected"); break;
        case 0x5f:str += QObject::tr(", auto, red-eye reduction mode, return light detected"); break;
        }
    }else{
        str = QObject::tr("no flash");
        switch (exif->Flash){
        case 0x10:str += QObject::tr(", compulsory"); break;
        case 0x18:str += QObject::tr(", auto"); break;
        }
    }

    return str;
}

QString ImageHeader::exposureProgram() const
{
    switch (exif->ExposureProgram) {
    case 1: return QObject::tr("manual");
    case 2: return QObject::tr("normal program");
    case 3: return QObject::tr("aperture priority");
    case 4: return QObject::tr("shutter priority");
    case 5: return QObject::tr("creative program (biased toward depth of field)");
    case 6: return QObject::tr("action program (biased toward fast shutter speed)");
    case 7: return QObject::tr("portrait mode");  // (for closeup photos with the background out of focus)
    case 8: return QObject::tr("landscape mode"); // (for landscape photos with the background in focus)
    default: return QString::null;
    }
}

QString ImageHeader::whiteBalance() const
{
    switch(exif->Whitebalance) {
    case 1: return QObject::tr("manual");
    case 0: return QObject::tr("auto");
    default: return QString::null;
    }
}

QString ImageHeader::lightSource() const
{
    switch(exif->LightSource) {
    case 1: return QObject::tr("daylight");
    case 2: return QObject::tr("fluorescent");
    case 3: return QObject::tr("tungsten (incandescent light) ");
    case 4: return QObject::tr("flash");
    case 9: return QObject::tr("fine weather");
    case 10: return QObject::tr("cloudy weather");
    case 11: return QObject::tr("shade");
    case 12: return QObject::tr("daylight fluorescent (D 5700 – 7100K)");
    case 13: return QObject::tr("day white fluorescent (N 4600 – 5400K)");
    case 14: return QObject::tr("cool white fluorescent (W 3900 – 4500K)");
    case 15: return QObject::tr("white fluorescent (WW 3200 – 3700K)");
    case 17: return QObject::tr("standard light A");
    case 18: return QObject::tr("standard light B");
    case 19: return QObject::tr("standard light C");
    case 20: return QObject::tr("D55");
    case 21: return QObject::tr("D65");
    case 22: return QObject::tr("D75");
    case 23: return QObject::tr("D50");
    case 24: return QObject::tr("ISO studio tungsten");
    case 255: return QObject::tr("other");
    default: return QString::null;
    }
}

QString ImageHeader::meteringMode() const
{
    switch(exif->MeteringMode) {
    case 1: return QObject::tr("average");
    case 2: return QObject::tr("center weighted average");
    case 3: return QObject::tr("spot"); break;
    case 4: return QObject::tr("multi-spot");
    case 5: return QObject::tr("pattern");
    case 6: return QObject::tr("partial");
    case 255: return QObject::tr("other");
    default: return QString::null;
    }
}

QString ImageHeader::GPSLatitudeString() const
{
    QString direction;
    switch (exif->GeoLocation.LatComponents.direction) {
    case 'N': direction += QObject::tr("North latitude"); break;
    case 'S': direction += QObject::tr("South latitude"); break;
    default: return QString::null;
    }

    if (exif->GeoLocation.LatComponents.seconds == 0)
        return QObject::tr("%1 %2d %3m")
                .arg(direction)
                .arg(exif->GeoLocation.LatComponents.degrees)
                .arg(exif->GeoLocation.LatComponents.minutes);

    return QObject::tr("%1 %2d %3m %4s")
            .arg(direction)
            .arg(exif->GeoLocation.LatComponents.degrees)
            .arg(exif->GeoLocation.LatComponents.minutes)
            .arg(exif->GeoLocation.LatComponents.seconds);
}

QString ImageHeader::GPSLongitudeString() const
{
    QString direction;
    switch (exif->GeoLocation.LonComponents.direction) {
    case 'E': direction += QObject::tr("East longitude"); break;
    case 'W': direction += QObject::tr("West longitude"); break;
    default: return QString::null;
    }

    if (exif->GeoLocation.LonComponents.seconds == 0)
        return QObject::tr("%1 %2d %3m")
                .arg(direction)
                .arg(exif->GeoLocation.LonComponents.degrees)
                .arg(exif->GeoLocation.LonComponents.minutes);

    return QObject::tr("%1 %2d %3m %4s")
            .arg(direction)
            .arg(exif->GeoLocation.LonComponents.degrees)
            .arg(exif->GeoLocation.LonComponents.minutes)
            .arg(exif->GeoLocation.LonComponents.seconds);
}

QString ImageHeader::GPSAltitudeString() const
{
    if (exif->GeoLocation.AltitudeRef == 0 || exif->GeoLocation.AltitudeRef == 1)
        return QObject::tr("%1 m").arg(exif->GeoLocation.Altitude);

    return QString::null;
}


bool ImageHeader::isJpegFile(QFile &file)
{
    // Jpeg file starts with '0xFFD8' and ends with '0xFFD9'.
    uchar ch;
    if (file.seek(0)
            && file.getChar((char*)&ch) && ch == 0xff
            && file.getChar((char*)&ch) && ch == M_SOI
            /*!
             * Some jpeg file ends with extra padding bytes.
             * So we just detect the jpeg header.
             */
//            && file.seek(file.size() - 2)           // jump to file end
//            && file.getChar((char*)&ch) && ch == 0xff
//            && file.getChar((char*)&ch) && ch == M_EOI
            ) {
        return true;
    }

    return false;
}

// Return ture if file read and parse as a JPEG image success.
bool ImageHeader::parseJpegFile(QFile &file)
{
    if (!isJpegFile(file))
        return false;

    file.seek(2);   // skip JPEG header : '0xFFD8'.

    int sectionLen;
    int dataLen;
    qint64 nextSectionPos;
    const qint64 fileLength = file.size();
    uchar prev;
    uchar marker;
    uchar ll,lh;
    while (true) {
        prev = 0;
        for (int padding = 0; padding <= 10; ++padding){  // find the marker follow to 0xff.
            if (!file.getChar((char*)&marker))
                return false;
            if (marker != 0xff && prev == 0xff)
                break;
            prev = marker;
        }

        if (marker == 0xff || prev != 0xff) // Too much padding bytes before section
            return false;

        // Read the length of the section.
        if (!file.getChar((char*)&lh) || !file.getChar((char*)&ll))
            return false;

        sectionLen = ((int)(lh) << 8) | ll;
        dataLen = sectionLen - 2;
        nextSectionPos = file.pos() + dataLen;
        if (sectionLen < 2 || nextSectionPos > fileLength)
            return false;

        switch(marker){
        case M_SOS: // stop before hitting compressed data
        case M_EOI:
            return true;
        case M_DQT:  // Use for jpeg quality guessing
        {
            if (jpegQualityGuess == -1) {
                uchar *Data = new uchar[sectionLen];
                if (file.read((char*)Data + 2, dataLen) != dataLen) {
                    delete [] Data;
                    return false;
                }
                Data[0] = (uchar)lh;
                Data[1] = (uchar)ll;
                jpegQualityGuess = process_DQT(Data, sectionLen);
                delete [] Data;
            }
            break;
        }
        case M_EXIF: // There can be different section using the same marker.
        {
            if (!(type & TYPE_EXIF)) {
                uchar *Data = new uchar[sectionLen];
                if (file.read((char*)Data + 2, dataLen) != dataLen) {
                    delete [] Data;
                    return false;
                }
                Data[0] = (uchar)lh;
                Data[1] = (uchar)ll;

                // Parse EXIF
                int code = exif->parseFrom(Data, sectionLen);
                if (code == 0 || code == PARSE_EXIF_ERROR_CORRUPT)
                    type |= TYPE_EXIF;

                delete [] Data;
            }
            break;
        }
        default: // Skip any other sections.
            break;
        }

        if (!file.seek(nextSectionPos)) // Skip this section.
            return false;
    }

    return true;
}
