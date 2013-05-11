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

#ifndef IMAGEHEADER_H
#define IMAGEHEADER_H

#include <QString>

#include "exif.h"


class QFile;
class QImage;

class ImageHeader
{
public:
    enum Orientation {
        NOT_AVAILABLE = 0,
        NORMAL = 1,
        HFLIP = 2,
        ROT_180 = 3,
        VFLIP = 4,
        TRANSPOSE = 5,
        ROT_90 = 6,
        TRANSVERSE = 7,
        ROT_270 = 8
    };

    ImageHeader();
    ~ImageHeader();

    static bool isFormatSupport(const QString &imageFormat);
    // Return true if file type can be detected, otherwise return false.
    bool loadFile(const QString &path);

    bool isJpeg() const { return type & TYPE_JPEG; }
    // Jpeg quality exists in most jpeg file. Even if there is no exif tag, the quality info may also exist.
    bool hasQuality() const { return jpegQualityGuess != -1; }
    int  quality() const { return jpegQualityGuess; }

    bool hasExif() const { return type & TYPE_EXIF; }
    // This will change the @image orientation accroding exif info.
    void autoRotateImage(QImage &image);

    /*
     * Below are exif info.
     */
    bool hasMake() const { return !make().isEmpty(); }
    bool hasModel() const { return !model().isEmpty(); }
    bool hasSoftware() const { return !software().isEmpty(); }
    bool hasOrientation() const { return exif->Orientation > NOT_AVAILABLE && exif->Orientation <= ROT_270; }
    bool hasDateTimeOriginal() const { return !QString(exif->DateTimeOriginal.c_str()).trimmed().isEmpty(); }
    bool hasExposureTime() const { return exif->ExposureTime; }
    bool hasFNumber() const { return exif->FNumber; }
    bool hasISOSpeed() const { return exif->ISOSpeedRatings; }
    bool hasShutterSpeed() const { return exif->ShutterSpeedValue; }
    bool hasSubjectDistance() const { return exif->SubjectDistance; }
    bool hasExposureBias() const { return exif->ExposureBiasValue; }
    bool hasExposureProgram() const { return exif->ExposureProgram >= 1 && exif->ExposureProgram <= 8; }
    bool hasFlash() const { return exif->Flash != 0x40; }
    bool hasWhiteBalance() const { return exif->Whitebalance == 0 || exif->Whitebalance == 1; }
    bool hasLightSource() const { return exif->LightSource >= 1 && exif->LightSource <= 24; }
    bool hasMeteringMode() const { return exif->MeteringMode >= 1 && exif->MeteringMode <= 6; }
    bool hasFocalLength() const { return exif->FocalLength; }
    bool hasFocalLengthIn35mm() const { return exif->FocalLengthIn35mm; }

    bool hasGPSLatitude() const { return exif->GeoLocation.LatComponents.direction; }
    bool hasGPSLongitude() const { return exif->GeoLocation.LonComponents.direction; }
    bool hasGPSAltitude() const { return exif->GeoLocation.AltitudeRef == 0 || exif->GeoLocation.AltitudeRef == 1; }

    QString make() const { return QString(exif->Make.c_str()).trimmed(); }
    QString model() const { return QString(exif->Model.c_str()).trimmed(); }
    QString software() const { return QString(exif->Software.c_str()).trimmed(); }
    QString orientationString() const;
    QString dateTimeOriginal() const;
    QString exposureProgram() const;
    QString flashMode() const;
    QString whiteBalance() const;
    QString lightSource() const;
    QString meteringMode() const;

    ushort  orientation() const { return exif->Orientation; }
    double  exposureTime() const { return exif->ExposureTime; }
    double  fNumber() const { return exif->FNumber; }
    ushort  ISOSpeed() const { return exif->ISOSpeedRatings; }
    double  shutterSpeed() const { return exif->ShutterSpeedValue; }
    double  subjectDistance() const { return exif->SubjectDistance; }
    double  exposureBias() const { return exif->ExposureBiasValue; }
    double  focalLength() const { return exif->FocalLength; }
    ushort  focalLengthIn35mm() const { return exif->FocalLengthIn35mm; }

    double  GPSLatitude() const { return exif->GeoLocation.Latitude; }
    double  GPSLongitude() const { return exif->GeoLocation.Latitude; }
    double  GPSAltitude() const { return exif->GeoLocation.Altitude; }

    QString GPSLatitudeString() const;
    QString GPSLongitudeString() const;
    QString GPSAltitudeString() const;

private:
    static bool isJpegFile(QFile &file);
    // Return ture if file read and parse as a JPEG image success.
    bool parseJpegFile(QFile &file);


    QString lastLoadFile;
    enum ImageType {
        TYPE_UNKNOWN = 0x00,
        TYPE_JPEG = 0x01,
        TYPE_EXIF = 0x02
    };
    int type;
    int jpegQualityGuess;
    EXIFInfo* exif;
};

#endif // IMAGEHEADER_H
