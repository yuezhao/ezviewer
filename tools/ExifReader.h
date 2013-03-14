/******************************************************************************
	ExifReader: reading the exif information using libexif
	Copyright (C) 2012 Wang Bin <wbsecg1@gmail.com>
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License along
	with this program; if not, write to the Free Software Foundation, Inc.,
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
******************************************************************************/


#ifndef PHOTOKIT_EXIFREADER_H
#define PHOTOKIT_EXIFREADER_H

#include <QMap>
#include <QString>
namespace PhotoKit {
class ExifReaderPrivate;
class ExifReader
{
public:
	enum IFD {
		IFD_0, IFD_1, EXIF, GPS, Interoperability
	};
	enum Tag {
		FNumber, ExposureTime, ExposureMode, FocalLength, Flash, ISOSpeed, MeteringMode
		, DateTimeOrigin, WhiteBalance, Manufacturer, Model, Software, Copyright
		, LatitudeRef, Latitude, LongitudeRef, Longitude, AltitudeRef, Altitude, ImageDirectionRef
		, ImageDirection
	};
	typedef QMap<QString/*name*/, QString/*value*/> TagInfo;
	ExifReader(const QString& fileName = QString());
	~ExifReader();

	bool hasData() const;

	bool hasIFD(IFD ifd) const;
	bool hasIFD0() const;
	bool hasIFD1() const;
	bool hasIFDExif() const;
	bool hasIFDGPS() const;
	bool hasIFDInteroperability() const;

	void loadFile(const QString& fileName);
//QList<QPair<QString, QString> >
	TagInfo dumpAll() const;
	TagInfo getIFD0Brief() const;
	TagInfo getExifBrief() const;
	TagInfo getGpsBrief() const;

	//QString value(Tag tag) const;

//IFD_0
	QString manufacturer() const;
	QString model() const;
	QString software() const;
	QString copyright() const;

//IFD_EXIF
	QString fNumber() const;
	QString exposureTime() const;
	QString exposureMode() const;
	QString focalLength() const;
	QString flash() const;
	QString isoSpeed() const;
	QString dateTimeOrigin() const;
	QString meteringMode() const;
	QString whiteBalance() const;

private:
	ExifReaderPrivate *d;
};

} //namespace PhotoKit

#endif //PHOTOKIT_EXIFREADER_H
