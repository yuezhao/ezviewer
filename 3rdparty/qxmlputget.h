/***************************************************************************
**                                                                        **
**  QXmlPutGet, classes for conveniently handling XML with Qt             **
**  Copyright (C) 2012 Emanuel Eichhammer                                 **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Emanuel Eichhammer                                   **
**  Website/Contact: http://www.WorksLikeClockwork.com/                   **
**             Date: 05.03.12                                             **
****************************************************************************/

#ifndef QXMLPUTGET_H
#define QXMLPUTGET_H

#include <QtXml>
#include <QColor>
#include <QImage>
#include <QPen>
#include <QBrush>
#include <QFont>

// define FUNCNAME macro to mean the function name for debug output on different compilers:
#if defined(Q_CC_GNU)
#  define FUNCNAME __PRETTY_FUNCTION__
#elif defined(Q_CC_MSVC)
#  define FUNCNAME __FUNCSIG__
#else
#  define FUNCNAME __func__
#endif

/*! \file */ 

/*! \namespace QXmlPutGet */
namespace QXmlPutGet
{
  /*!
    The possible formats for reading/writing boolean values from/to strings in the XML document.
  */
  enum BoolFormat {bfYesNo     = 0x01, ///< boolean values are represented with "yes" and "no".
                   bfTrueFalse = 0x02, ///< boolean values are represented with "true" and "false".
                   bf10        = 0x04, ///< boolean values are represented with "1" and "0".
                   bfAll       = 0xFF  ///< When reading boolean values, all formats are allowed. 
                  };
  Q_DECLARE_FLAGS(BoolFormats, BoolFormat)
}
Q_DECLARE_OPERATORS_FOR_FLAGS(QXmlPutGet::BoolFormats)

class QXmlGet;

class QXmlPut
{
public:
  QXmlPut(const QString &rootTag);
  QXmlPut(const QString &rootTag, const QString &xmlVersion, const QString &encoding, bool standalone, const QString &docType=QString(), const QString &publicId=QString(), const QString &systemId=QString());
  QXmlPut(const QXmlGet &xmlGet);
  ~QXmlPut();
  QDomDocument document() const { return mDocument; }
  
  void putComment(const QString &comment);
  void putSingleTag(const QString &tagName);
  void putString(const QString &tagName, const QString &value, bool asCDATA=false);
  void putStringList(const QString &tagName, const QStringList &values, bool asCDATA=false);
  void putInt(const QString &tagName, int value);
//  void putIntVector(const QString &tagName, const QVector<int> &values);
  void putDouble(const QString &tagName, double value);
//  void putDoubleVector(const QString &tagName, const QVector<double> &values);
  void putBool(const QString &tagName, bool value, QXmlPutGet::BoolFormat format=QXmlPutGet::bfYesNo);
//  void putBoolVector(const QString &tagName, const QVector<bool> &values, QXmlPutGet::BoolFormat format=QXmlPutGet::bfYesNo);
//  void putColor(const QString &tagName, const QColor &value);
//  void putSize(const QString &tagName, const QSize &value);
//  void putSizeF(const QString &tagName, const QSizeF &value);
//  void putPoint(const QString &tagName, const QPoint &value);
//  void putPointF(const QString &tagName, const QPointF &value);
//  void putRect(const QString &tagName, const QRect &value);
//  void putRectF(const QString &tagName, const QRectF &value);
//  void putDate(const QString &tagName, const QDate &value);
//  void putTime(const QString &tagName, const QTime &value);
//  void putDateTime(const QString &tagName, const QDateTime &value);
//  void putByteArray(const QString &tagName, const QByteArray &value, int blockWidth=2048, int compression=9);
//  void putImage(const QString &tagName, const QImage &value, const QString format=QString("PNG"), int blockWidth=2048);
//  void putPen(const QString &tagName, const QPen &value);
//  void putBrush(const QString &tagName, const QBrush &value);
//  void putFont(const QString &tagName, const QFont &value);
  
  QXmlPut restricted();
  void descend(const QString &tagName);
  QXmlPut descended(const QString &tagName);
  bool rise();
  bool goTo(QDomElement parentElement);
  QDomElement element() const { return mCurrentElement; }
  
  void setAttributeString(const QString &name, const QString &value);
  void setAttributeInt(const QString &name, int value);
//  void setAttributeIntVector(const QString &name, const QVector<int> &value);
  void setAttributeDouble(const QString &name, double value);
//  void setAttributeDoubleVector(const QString &name, const QVector<double> &value);
  void setAttributeBool(const QString &name, bool value, QXmlPutGet::BoolFormat format=QXmlPutGet::bfYesNo);
//  void setAttributeBoolVector(const QString &name, const QVector<bool> &value, QXmlPutGet::BoolFormat format=QXmlPutGet::bfYesNo);
//  void setAttributeColor(const QString &name, const QColor &value);
  
  QString toString(int spacesPerIndent=2) const;
  bool save(const QString &fileName, int spacesPerIndent=2) const;
  
private:
  static QString boolToStr(bool value, QXmlPutGet::BoolFormat format=QXmlPutGet::bfTrueFalse);
//  static QString colorToStr(const QColor &color);
//  static QString intVectorToStr(const QVector<int> &value);
//  static QString doubleVectorToStr(const QVector<double> &value);
//  static QString boolVectorToStr(const QVector<bool> &value, QXmlPutGet::BoolFormat format=QXmlPutGet::bfTrueFalse);
  QDomDocument mDocument;
  QDomElement mBarrierNode; // this instance isn't allowed to rise beyond this node
  QDomElement mCurrentParent;
  QDomElement mCurrentElement;
  QString mXmlDeclaration;
  QString mXmlVersion;
  QString mEncoding;
  bool mStandalone;
  
  friend class QXmlGet;
};


class QXmlGet
{
public:
  QXmlGet();
  QXmlGet(QDomDocument document);
  QXmlGet(const QXmlPut &xmlPut);
  ~QXmlGet();
  QDomDocument document() const { return mDocument; }
  QString docType() const;
  QString publicId() const;
  QString systemId() const;
  QString xmlDeclaration() const { return mXmlDeclaration; }
  QString xmlVersion() const { return mXmlVersion; }
  QString encoding() const { return mEncoding; }
  bool standalone() const { return mStandalone; }
  
  bool find(const QString &tagName);
  bool findAndDescend(const QString &tagName);
  void findReset();
  bool findNext(const QString &tagName=QString());
  bool findNextAndDescend(const QString &tagName=QString());
  int childCount(const QString &tagName=QString()) const;
  bool hasChildren() const;
  
  QString tagName() const;
  QString getString(const QString &defaultValue=QString()) const;
  QStringList getStringList(const QStringList &defaultValue=QStringList());
  int getInt(int defaultValue=0) const;
//  QVector<int> getIntVector(const QVector<int> &defaultValue=QVector<int>()) const;
  double getDouble(double defaultValue=0) const;
//  QVector<double> getDoubleVector(const QVector<double> &defaultValue=QVector<double>()) const;
  bool getBool(bool defaultValue=false, QXmlPutGet::BoolFormats formats=QXmlPutGet::bfAll) const;
//  QVector<bool> getBoolVector(const QVector<bool> &defaultValue=QVector<bool>(), QXmlPutGet::BoolFormats formats=QXmlPutGet::bfAll) const;
//  QColor getColor(const QColor &defaultValue=QColor()) const;
//  QSize getSize(const QSize &defaultValue=QSize()) const;
//  QSizeF getSizeF(const QSizeF &defaultValue=QSizeF()) const;
//  QPoint getPoint(const QPoint &defaultValue=QPoint()) const;
//  QPointF getPointF(const QPointF &defaultValue=QPointF()) const;
//  QRect getRect(const QRect &defaultValue=QRect()) const;
//  QRectF getRectF(const QRectF &defaultValue=QRectF()) const;
//  QDate getDate(const QDate &defaultValue=QDate()) const;
//  QTime getTime(const QTime &defaultValue=QTime()) const;
//  QDateTime getDateTime(const QDateTime &defaultValue=QDateTime()) const;
//  QByteArray getByteArray(const QByteArray &defaultValue=QByteArray()) const;
//  QImage getImage(const QImage &defaultValue=QImage()) const;
//  QPen getPen(const QPen &defaultValue=QPen()) const;
//  QBrush getBrush(const QBrush &defaultValue=QBrush()) const;
//  QFont getFont(const QFont &defaultValue=QFont()) const;
  
  bool hasAttribute(const QString &name) const;
  QString getAttributeString(const QString &name, const QString &defaultValue=QString()) const;
  int getAttributeInt(const QString &name, int defaultValue=0) const;
//  QVector<int> getAttributeIntVector(const QString &name, const QVector<int> &defaultValue=QVector<int>()) const;
  double getAttributeDouble(const QString &name, double defaultValue=0) const;
//  QVector<double> getAttributeDoubleVector(const QString &name, const QVector<double> &defaultValue=QVector<double>()) const;
  bool getAttributeBool(const QString &name, bool defaultValue=false, QXmlPutGet::BoolFormats formats=QXmlPutGet::bfAll) const;
//  QVector<bool> getAttributeBoolVector(const QString &name, const QVector<bool> &defaultValue=QVector<bool>(), QXmlPutGet::BoolFormats format=QXmlPutGet::bfAll) const;
//  QColor getAttributeColor(const QString &name, const QColor &defaultValue=QColor()) const;
  
  bool fromString(const QString &str, QString *errorMessage=0, int *errorLine=0, int *errorColumn=0);
  bool load(const QString &fileName, QString *errorMessage=0, int *errorLine=0, int *errorColumn=0);
  
  QXmlGet restricted();
  void descend();
  QXmlGet descended();
  bool rise();
  bool goTo(QDomElement parentElement);
  QDomElement element() const { return mCurrentElement; }

private:
  static bool strToBool(QString value, bool *ok=0, QXmlPutGet::BoolFormats formats=QXmlPutGet::bfAll);
//  static QColor strToColor(QString value, bool *ok=0);
//  static QVector<int> strToIntVector(QString value, bool *ok=0);
//  static QVector<double> strToDoubleVector(QString value, bool *ok=0);
//  static QVector<bool> strToBoolVector(QString value, bool *ok=0, QXmlPutGet::BoolFormats formats=QXmlPutGet::bfAll);
  QDomDocument mDocument;
  QDomElement mBarrierNode; // this instance isn't allowed to rise beyond this node
  QDomElement mCurrentParent;
  QDomElement mCurrentElement;
  QString mXmlDeclaration;
  QString mXmlVersion;
  QString mEncoding;
  bool mStandalone;
  
  friend class QXmlPut;
};

#endif // QXMLPUTGET_H

