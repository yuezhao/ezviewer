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


#include "ExifReader.h"
#include <cstdio>
#include <libexif/exif-data.h>
#include <QtCore/QObject>
#include <QtDebug>
namespace PhotoKit {

//static void retranslateUi() {
//        qDebug() << QObject::tr("Manufacturer") << QObject::tr("Model") <<QObject::tr("Software")
//                        << QObject::tr("Copyright") << QObject::tr("Exif Verion") << QObject::tr("F-Number")
//                        << QObject::tr("Exposure time") << QObject::tr("Exposure mode") <<QObject::tr("Focal length")
//                        << QObject::tr("Flash") << QObject::tr("ISO Speed Ratings") << QObject::tr("Date and Time (Original)")
//                        << QObject::tr("Metering mode") << QObject::tr("White balance")
//                        << QObject::tr("North or South") << QObject::tr("Latitude") << QObject::tr("East or West")
//                        << QObject::tr("Longitude") << QObject::tr("Altitude refrence") << QObject::tr("Altitude")
//                        << QObject::tr("Image Direction Refrence") << QObject::tr("Image Direction");
//}


typedef struct {
	int tag;
	const char* name;
} tag_info;


//TODO: not translated?
static const tag_info idf0_tags[] = {
	{EXIF_TAG_MAKE, 			QT_TR_NOOP("Manufacturer")}, //QT_TR_NOOP()
	{EXIF_TAG_MODEL,			QT_TR_NOOP("Model")},
//	{EXIF_TAG_SOFTWARE,			QT_TR_NOOP("Software")},
//	{EXIF_TAG_COPYRIGHT,		QT_TR_NOOP("Copyright")},
};

static const tag_info idf_exif_tags[] = {
///	{EXIF_TAG_EXIF_VERSION,			QT_TR_NOOP("Exif Verion")},
	{EXIF_TAG_FNUMBER,				QT_TR_NOOP("F-Number")},
	{EXIF_TAG_EXPOSURE_TIME,		QT_TR_NOOP("Exposure time")},
	{EXIF_TAG_EXPOSURE_MODE,		QT_TR_NOOP("Exposure mode")},
	{EXIF_TAG_FOCAL_LENGTH,			QT_TR_NOOP("Focal length")},
	{EXIF_TAG_FLASH,				QT_TR_NOOP("Flash")},
	{EXIF_TAG_ISO_SPEED_RATINGS,	QT_TR_NOOP("ISO Speed Ratings")},
	{EXIF_TAG_DATE_TIME_ORIGINAL,	QT_TR_NOOP("Date and Time (Original)")},
//	{EXIF_TAG_METERING_MODE,		QT_TR_NOOP("Metering mode")},
	{EXIF_TAG_WHITE_BALANCE,		QT_TR_NOOP("White balance")}
};

static const tag_info idf_gps_tags[] = {
	{EXIF_TAG_GPS_LATITUDE_REF, 	QT_TR_NOOP("North or South")},
	{EXIF_TAG_GPS_LATITUDE,			QT_TR_NOOP("Latitude")},
	{EXIF_TAG_GPS_LONGITUDE_REF,	QT_TR_NOOP("East or West")},
	{EXIF_TAG_GPS_LONGITUDE,		QT_TR_NOOP("Longitude")},
	{EXIF_TAG_GPS_ALTITUDE_REF,		QT_TR_NOOP("Altitude refrence")},
	{EXIF_TAG_GPS_ALTITUDE,			QT_TR_NOOP("Altitude")},
	{EXIF_TAG_GPS_IMG_DIRECTION_REF,QT_TR_NOOP("Image Direction Refrence")},
	{EXIF_TAG_GPS_IMG_DIRECTION,	QT_TR_NOOP("Image Direction")}
};


static void read_exif_entry(ExifEntry *ee, void* info)
{
	char v[1024];
//	strncpy(t, exif_tag_get_title_in_ifd(ee->tag, exif_entry_get_ifd(ee)), sizeof(t));
//	strncpy(t, exif_tag_get_title_in_ifd(ee->tag, *((ExifIfd*)ifd)), sizeof(t));
	//trim t
/*	printf("[%s] (%s): %s\n  %s\n"
			, exif_tag_get_name_in_ifd(ee->tag, *((ExifIfd*)ifd))
			, exif_tag_get_title_in_ifd(ee->tag, *((ExifIfd*)ifd))
			, exif_entry_get_value(ee, v, sizeof(v))
			, exif_tag_get_description_in_ifd(ee->tag, *((ExifIfd*)ifd))
			);*/
	((ExifReader::TagInfo*)info)->insert(exif_tag_get_title_in_ifd(ee->tag, exif_entry_get_ifd(ee))
						, exif_entry_get_value(ee, v, sizeof(v)));
}

void read_exif_content(ExifContent *ec, void *user_data)
{
	ExifIfd ifd = exif_content_get_ifd(ec);
	if (ifd == EXIF_IFD_COUNT)
		fprintf(stderr, "No EXIF data. exif_content_get_ifd error");
	exif_content_foreach_entry(ec, read_exif_entry, user_data);
}

class ExifReaderPrivate
{
public:
	ExifReaderPrivate():data(0){}
	~ExifReaderPrivate(){
		if (data)
			exif_data_unref(data);
		data = 0;
	}

	ExifData* data;
	QString path;
};

ExifReader::ExifReader(const QString& fileName)
	:d(new ExifReaderPrivate)
{
	if (!fileName.isEmpty())
		loadFile(fileName);
}

ExifReader::~ExifReader()
{
	if (d) {
		delete d;
		d = 0;
	}
}

bool ExifReader::hasData() const
{
	return d->data != 0;
}


bool ExifReader::hasIFD(IFD ifd) const
{
	if ((ExifIfd)ifd >= EXIF_IFD_COUNT)
		return false;
	ExifContent *c = d->data->ifd[(ExifIfd)ifd];
	return c->count > 0;
}

bool ExifReader::hasIFD0() const
{
	return hasIFD(IFD_0);
}

bool ExifReader::hasIFD1() const
{
	return hasIFD(IFD_1);
}

bool ExifReader::hasIFDExif() const
{
	return hasIFD(EXIF);
}

bool ExifReader::hasIFDGPS() const
{
	return hasIFD(GPS);
}

bool ExifReader::hasIFDInteroperability() const
{
	return hasIFD(Interoperability);
}

void ExifReader::loadFile(const QString &fileName)
{
	d->path = fileName;
	if (!d->data)
		exif_data_unref(d->data);
	d->data = exif_data_new_from_file(fileName.toLocal8Bit().constData());
///	if (!d->data)
///		qDebug("No exif data");
}

ExifReader::TagInfo ExifReader::dumpAll() const
{
	TagInfo info;
	exif_data_foreach_content(d->data, read_exif_content, &info);
	return info;
}

ExifReader::TagInfo ExifReader::getIFD0Brief() const
{
	TagInfo info;
	ExifContent *c = d->data->ifd[EXIF_IFD_0];
	if(c->count == 0)
		return info;
	QByteArray v(1024, 0);
	int s = sizeof(idf0_tags)/sizeof(tag_info);
	for(int i = 0; i < s; ++i) {
		v.fill(0);
		ExifEntry *entry = exif_content_get_entry(c, (ExifTag)idf0_tags[i].tag);
		exif_entry_get_value(entry, v.data(), v.size());
		v.trimmed();
		info.insert(QObject::tr(idf0_tags[i].name), QString::fromUtf8(v.constData())); //libexif use utf8
		//info.insert(QString::fromUtf8(exif_tag_get_title_in_ifd(entry->tag, exif_entry_get_ifd(entry))), QString::fromUtf8(v.constData()));
	}
	return info;
}

ExifReader::TagInfo ExifReader::getExifBrief() const
{
	TagInfo info;
	ExifContent *c = d->data->ifd[EXIF_IFD_EXIF];
	if(c->count == 0)
		return info;
	QByteArray v(1024, 0);
	int s = sizeof(idf_exif_tags)/sizeof(tag_info);
	for(int i = 0; i < s; ++i) {
		v.fill(0);
		ExifEntry *entry = exif_content_get_entry(c, (ExifTag)idf_exif_tags[i].tag);
		exif_entry_get_value(entry, v.data(), v.size());
		v.trimmed();
		info.insert(QObject::tr(idf_exif_tags[i].name),  QString::fromUtf8(v.constData()));
		//info.insert(QString::fromUtf8(exif_tag_get_title_in_ifd(entry->tag, exif_entry_get_ifd(entry))), QString::fromUtf8(v.constData()));
	}
	return info;
}

ExifReader::TagInfo ExifReader::getGpsBrief() const
{
	TagInfo info;
	ExifContent *c = d->data->ifd[EXIF_IFD_GPS];
	if(c->count == 0)
		return info;
	QByteArray v(1024, 0);
	int s = sizeof(idf_gps_tags)/sizeof(tag_info);
	for(int i = 0; i < s; ++i) {
		v.fill(0);
		ExifEntry *entry = exif_content_get_entry(c, (ExifTag)idf_gps_tags[i].tag);
		exif_entry_get_value(entry, v.data(), v.size());
		v.trimmed();
		info.insert(QObject::tr(idf_gps_tags[i].name),  QString::fromUtf8(v.constData()));
		//crash
		//info.insert(QString::fromUtf8(exif_tag_get_title_in_ifd(entry->tag, exif_entry_get_ifd(entry))), QString::fromUtf8(v.constData()));
	}
	//TODO: i18n
	info[QObject::tr("Longitude")] = info[QObject::tr("Longitude")] + " " + info[QObject::tr("East or West")];
	info.remove(QObject::tr("East or West"));
	info[QObject::tr("Latitude")] = info[QObject::tr("Latitude")] + " " + info[QObject::tr("North or South")];
	info.remove(QObject::tr("North or South"));
	info[QObject::tr("Image Direction")] = info[QObject::tr("Image Direction")] + " " + info[QObject::tr("Image Direction Refrence")];
	info.remove(QObject::tr("Image Direction Refrence"));
	info[QObject::tr("Altitude")] = info[QObject::tr("Altitude")] + " " + info[QObject::tr("Altitude refrence")];
	info.remove(QObject::tr("Altitude refrence"));
	return info;
}

} //namespace PhotoKit
