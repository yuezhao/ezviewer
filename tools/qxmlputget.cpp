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

#include "qxmlputget.h"

/*! \mainpage
  
  \section introduction Introduction

  %QXmlPutGet is a library for convenient and intuitive writing and reading of XML. This is a quick
  and more technical introduction to %QXmlPutGet. For a practically oriented tutorial, visit
  
  http://www.workslikeclockwork.com/index.php/components/xml-classes-for-qt/
  
  \section overview Overview
  The library consists of two classes: writing is done by QXmlPut and reading by QXmlGet.
  
  Both classes work on their <i>current element</i>. In QXmlPut this is the element that was
  previously created (e.g. with QXmlPut::putString), in QXmlGet this is the element which was
  previously navigated to (e.g. with \ref QXmlGet::find). Due to this concept, writing and
  especially reading may not always happen in one single call, but multiple calls, that base on the
  internal state (the <i>current element</i>) of the QXmlPut/QXmlGet instance.
  
  In QXmlPut the navigation is tied to the creation of tags (\ref QXmlPut::putSingleTag, \ref
  QXmlPut::putString, \ref QXmlPut::descend,...).

  In QXmlGet the navigation is done by finding tags by name (QXmlGet::find, QXmlGet::findNext), and
  possibly descending into them (\ref QXmlGet::descend, \ref QXmlGet::descended).
  
  Both classes have functions that allow direct jumps inside the underlying QDomDocument (\ref
  QXmlGet::goTo, \ref QXmlPut::goTo, \ref QXmlGet::element, \ref QXmlPut::element). Use these
  functions only when necessary, as it's not the way normal linear navigation should be carried
  out.
  
  In QXmlGet, once there is a current element available, it can be accessed e.g. with \ref
  QXmlGet::getString, \ref QXmlGet::getInt, \ref QXmlGet::getAttributeString etc.
  
  In QXmlPut, the current element (which was created with \ref QXmlPut::putString("tagName",
  "string content"), for example), can further be modified by setting attributes, e.g. with \ref
  QXmlPut::setAttributeString.
*/

// ================================================================================
// =================== QXMLPut
// ================================================================================

/* start documentation of inline functions */

/*! \fn QDomElement QXmlPut::element() const
  
  Returns the current element as a QDomElement. This can be used as a jump target for \ref goTo, or
  some low-level manipulation of the node with the Qt-DOM-interface.
*/

/*! \fn QDomDocument QXmlPut::document() const
  
  Returns the QDomDocument this QXmlPut instance is using.
*/

/* end documentation of inline functions */

/*!
  Creates a new XML document with a root tag named \a rootTag. XML version is set to 1.0, encoding
  to UTF-8 and standalone to false.
*/
QXmlPut::QXmlPut(const QString &rootTag) :
  mXmlDeclaration("xml"),
  mXmlVersion("1.0"),
  mEncoding("UTF-8"),
  mStandalone(false)
{
  if (rootTag.isEmpty())
    qDebug() << FUNCNAME << "root tag can't be empty";
  mDocument.appendChild(mDocument.createProcessingInstruction(mXmlDeclaration, QString("version=\"%1\" encoding=\"%2\"").arg(mXmlVersion).arg(mEncoding)));
  mDocument.appendChild(mDocument.createElement(rootTag));
  mCurrentParent = mDocument.documentElement();
  mCurrentElement = mCurrentParent;
  mBarrierNode = mCurrentParent;
}

/*!
  Creates a new XML document with a root tag named \a rootTag. And the specified XML parameters.
  
  \b example:
  
  The call
  \code
  QXmlPut xmlPut("myRoot", "1.0", "UTF-8", true, "myDocType", "myPublicId", "mySystemId");
  \endcode
  creates the following XML document:
  \code
  <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
  <!DOCTYPE myDocType PUBLIC 'myPublicId' 'mySystemId'>
  <myRoot>
   (...)
  </myRoot>
  \endcode
*/
QXmlPut::QXmlPut(const QString &rootTag, const QString &xmlVersion, const QString &encoding,  bool standalone, const QString &docType, const QString &publicId, const QString &systemId):
  mDocument(),
  mXmlDeclaration("xml"),
  mXmlVersion(xmlVersion),
  mEncoding(encoding),
  mStandalone(standalone)
{
  if (rootTag.isEmpty())
    qDebug() << FUNCNAME << "rootTag can't be empty";
  if (mXmlVersion.isEmpty())
    qDebug() << FUNCNAME << "xmlVersion can't be empty";
  if (mEncoding.isEmpty())
    qDebug() << FUNCNAME << "encoding can't be empty";
  
  if (mStandalone)
    mDocument.appendChild(mDocument.createProcessingInstruction(mXmlDeclaration, QString("version=\"%1\" encoding=\"%2\" standalone=\"yes\"").arg(mXmlVersion).arg(mEncoding)));
  else
    mDocument.appendChild(mDocument.createProcessingInstruction(mXmlDeclaration, QString("version=\"%1\" encoding=\"%2\"").arg(mXmlVersion).arg(mEncoding)));
  if (!docType.isEmpty())
    mDocument.appendChild(QDomImplementation().createDocumentType(docType, publicId, systemId));
  mDocument.appendChild(mDocument.createElement(rootTag));
  mCurrentParent = mDocument.documentElement();
  mCurrentElement = mCurrentParent;
  mBarrierNode = mCurrentParent;
}

/*!
  Returns an QXmlPut instance located at the current position. This can be used to read and write
  to the XML document at the same time.
*/
QXmlPut::QXmlPut(const QXmlGet &xmlGet) :
  mDocument(xmlGet.mDocument),
  mBarrierNode(xmlGet.mBarrierNode),
  mCurrentParent(xmlGet.mCurrentParent),
  mCurrentElement(xmlGet.mCurrentElement),
  mXmlDeclaration(xmlGet.mXmlDeclaration),
  mXmlVersion(xmlGet.mXmlVersion),
  mEncoding(xmlGet.mEncoding),
  mStandalone(xmlGet.mStandalone)
{
}

QXmlPut::~QXmlPut()
{
}

/*!
  Inserts a comment. Comments can't be read by QXmlGet and are meant for other humans reading the
  XML document in a text editor.
  
  <b>example output:</b>
  \code
  <!-- example comment -->
  \endcode
*/
void QXmlPut::putComment(const QString &comment)
{
  mCurrentParent.appendChild(mDocument.createComment(comment));
}

/*!
  Inserts a single tag with name \a tagName. As with every tag, you can attach attributes to it by
  subsequent calls to setAttribute(...) functions.
  
  <b>example output:</b>
  \code
  <tagName>
  \endcode
*/
void QXmlPut::putSingleTag(const QString &tagName)
{
  QDomElement el = mDocument.createElement(tagName);
  mCurrentParent.appendChild(el);
  mCurrentElement = el;
}

/*!
  Inserts a tag with name \a tagName which contains the string \a value. If the string contains
  many special characters like "<", ">" and linebreaks, you should consider setting \a
  asCDATA to true, so the string will be embedded in a CDATA-section which avoids escaping most of
  the special characters and thus makes it easier for users to read/modify the resulting XML
  document in a text editor.
  
  Note that due to escaping mechanisms any content is allowed for \a value. Even if \a asCDATA is
  set to true and \a value itself contains a string like "<![CDATA[trick]]>", it will be saved in
  two disjoint CDATA sections, splitting the \a value string into "<![CDATA[trick]]" and ">",
  preserving the resulting XML validity aswell as the saved \a value.
  
  <b>example output:</b>
  \code
  <tagName>example text</tagName>
  \endcode
  \code
  <tagName><![CDATA[example text]]></tagName>
  \endcode
  
  \see putStringList
*/
void QXmlPut::putString(const QString &tagName, const QString &value, bool asCDATA)
{
  QDomElement el = mDocument.createElement(tagName);
  mCurrentParent.appendChild(el);
  mCurrentElement = el;
  if (asCDATA)
  {
    int p = value.indexOf("]]>");
    if (p > -1) // split into multiple CDATA sections, which separate the "]]>" occurences
    {
      el.appendChild(mDocument.createCDATASection(value.mid(0, p+2))); // +2 so we split between "]" and ">"
      while (p > -1)
      {
        int nextp = value.indexOf("]]>", p+3);
        if (nextp > -1)
          el.appendChild(mDocument.createCDATASection(value.mid(p+2, nextp+2-(p+2))));
        else
          el.appendChild(mDocument.createCDATASection(value.mid(p+2)));
        p = nextp;
      }
    } else
      el.appendChild(mDocument.createCDATASection(value));
  }
  else
    el.appendChild(mDocument.createTextNode(value));
}

/*!
  Inserts a tag with name \a tagName which contains the string list \a values. If the strings
  contain many special characters like "<", ">" and linebreaks, you should consider setting \a
  asCDATA to true, so the strings will be embedded in a CDATA-section which avoids escaping most of
  the special characters and thus makes it easier for users to read/modify the resulting XML
  document in a text editor.
  
  <b>example output:</b>
  \code
  <tagName>
    <li>first line</li>
    <li>second line</li>
    <li>third line</li>
  </tagName>
  \endcode
  
  \see putString
*/
void QXmlPut::putStringList(const QString &tagName, const QStringList &values, bool asCDATA)
{
  descend(tagName);
  for (int i=0; i<values.size(); ++i)
    putString("li", values.at(i), asCDATA);
  rise();
}

/*!
  Inserts a tag with name \a tagName which contains the integer \a value.
  
  <b>example output:</b>
  \code
  <tagName>42</tagName>
  \endcode
  
  \see putDouble, putIntVector
*/
void QXmlPut::putInt(const QString &tagName, int value)
{
  QDomElement el = mDocument.createElement(tagName);
  mCurrentParent.appendChild(el);
  mCurrentElement = el;
  el.appendChild(mDocument.createTextNode(QString::number(value)));
}

/*!
  Inserts a tag with name \a tagName which contains the integer vector \a value.
  
  <b>example output:</b>
  \code
  <tagName>0;1;1;2;3;5;8;13</tagName>
  \endcode
  
  \see putInt
*/
//void QXmlPut::putIntVector(const QString &tagName, const QVector<int> &values)
//{
//  QDomElement el = mDocument.createElement(tagName);
//  mCurrentParent.appendChild(el);
//  mCurrentElement = el;
//  el.appendChild(mDocument.createTextNode(intVectorToStr(values)));
//}

/*!
  Inserts a tag with name \a tagName which contains the double \a value.
  
  <b>example output:</b>
  \code
  <tagName>3.1415</tagName>
  \endcode
  
  \see putInt, putDoubleVector
*/
void QXmlPut::putDouble(const QString &tagName, double value)
{
  QDomElement el = mDocument.createElement(tagName);
  mCurrentParent.appendChild(el);
  mCurrentElement = el;
  el.appendChild(mDocument.createTextNode(QString::number(value)));
}

/*!
  Inserts a tag with name \a tagName which contains the double vector \a value.
  
  <b>example output:</b>
  \code
  <tagName>0.1;0.24;0.52;-0.22</tagName>
  \endcode
  
  \see putDouble
*/
//void QXmlPut::putDoubleVector(const QString &tagName, const QVector<double> &values)
//{
//  QDomElement el = mDocument.createElement(tagName);
//  mCurrentParent.appendChild(el);
//  mCurrentElement = el;
//  el.appendChild(mDocument.createTextNode(doubleVectorToStr(values)));
//}

/*!
  Inserts a tag with name \a tagName which contains the bool \a value.
  
  Depending on the specified \a format (see \ref QXmlPutGet::BoolFormat), the boolean value is
  represented with different strings.
  
  <b>example output:</b>
  \code
  <tagName>yes</tagName>
  \endcode
  
  \see putBoolVector
*/
void QXmlPut::putBool(const QString &tagName, bool value, QXmlPutGet::BoolFormat format)
{
  QDomElement el = mDocument.createElement(tagName);
  mCurrentParent.appendChild(el);
  mCurrentElement = el;
  el.appendChild(mDocument.createTextNode(boolToStr(value, format)));
}

/*!
  Inserts a tag with name \a tagName which contains the bool vector \a value.
  
  Depending on the specified \a format (see \ref QXmlPutGet::BoolFormat), the boolean values are
  represented with different strings.
  
  <b>example output:</b>
  \code
  <tagName>yes;no;no;yes;no;yes</tagName>
  \endcode
  
  \see putBool
*/
//void QXmlPut::putBoolVector(const QString &tagName, const QVector<bool> &values, QXmlPutGet::BoolFormat format)
//{
//  QDomElement el = mDocument.createElement(tagName);
//  mCurrentParent.appendChild(el);
//  mCurrentElement = el;
//  QString value;
//  el.appendChild(mDocument.createTextNode(boolVectorToStr(values, format)));
//}

/*!
  Inserts a tag with name \a tagName which contains the QColor \a value.
  
  If the color contains no transparency component (i.e. alpha is 255), the color is saved in the
  format "#rrggbb". If it contains alpha, it is saved as "#rrggbbaa".
  
  <b>example output:</b>
  \code
  <tagName>#2280ff</tagName>
  \endcode
*/
//void QXmlPut::putColor(const QString &tagName, const QColor &value)
//{
//  QDomElement el = mDocument.createElement(tagName);
//  mCurrentParent.appendChild(el);
//  mCurrentElement = el;
//  el.appendChild(mDocument.createTextNode(colorToStr(value)));
//}

///*!
//  Inserts a tag with name \a tagName which contains the QSize \a value.

//  <b>example output:</b>
//  \code
//  <tagName width="640" height="480">
//  \endcode
  
//  \see putSizeF
//*/
//void QXmlPut::putSize(const QString &tagName, const QSize &value)
//{
//  putSingleTag(tagName);
//  setAttributeInt("width", value.width());
//  setAttributeInt("height", value.height());
//}

///*!
//  Inserts a tag with name \a tagName which contains the QSizeF \a value.

//  <b>example output:</b>
//  \code
//  <tagName width="122.4" height="10.95">
//  \endcode
  
//  \see putSize
//*/
//void QXmlPut::putSizeF(const QString &tagName, const QSizeF &value)
//{
//  putSingleTag(tagName);
//  setAttributeDouble("width", value.width());
//  setAttributeDouble("height", value.height());
//}

///*!
//  Inserts a tag with name \a tagName which contains the QPoint \a value.

//  <b>example output:</b>
//  \code
//  <tagName x="640" y="480">
//  \endcode
  
//  \see putPointF
//*/
//void QXmlPut::putPoint(const QString &tagName, const QPoint &value)
//{
//  putSingleTag(tagName);
//  setAttributeInt("x", value.x());
//  setAttributeInt("y", value.y());
//}

///*!
//  Inserts a tag with name \a tagName which contains the QPointF \a value.

//  <b>example output:</b>
//  \code
//  <tagName x="122.4" y="10.95">
//  \endcode
  
//  \see putPoint
//*/
//void QXmlPut::putPointF(const QString &tagName, const QPointF &value)
//{
//  putSingleTag(tagName);
//  setAttributeDouble("x", value.x());
//  setAttributeDouble("y", value.y());
//}

///*!
//  Inserts a tag with name \a tagName which contains the QRect \a value.

//  <b>example output:</b>
//  \code
//  <tagName left="10" top="15" width="640" height="480">
//  \endcode
  
//  \see putRectF
//*/
//void QXmlPut::putRect(const QString &tagName, const QRect &value)
//{
//  putSingleTag(tagName);
//  setAttributeInt("left", value.left());
//  setAttributeInt("top", value.top());
//  setAttributeInt("width", value.width());
//  setAttributeInt("height", value.height());
//}

///*!
//  Inserts a tag with name \a tagName which contains the QRectF \a value.

//  <b>example output:</b>
//  \code
//  <tagName left="10.5" top="14.99" width="122.4" height="10.95">
//  \endcode
  
//  \see putRect
//*/
//void QXmlPut::putRectF(const QString &tagName, const QRectF &value)
//{
//  putSingleTag(tagName);
//  setAttributeDouble("left", value.left());
//  setAttributeDouble("top", value.top());
//  setAttributeDouble("width", value.width());
//  setAttributeDouble("height", value.height());
//}

///*!
//  Inserts a tag with name \a tagName which contains the QDate \a value.

//  <b>example output:</b>
//  \code
//  <tagName>2012-04-03</tagName>
//  \endcode
  
//  \see putTime, putDateTime
//*/
//void QXmlPut::putDate(const QString &tagName, const QDate &value)
//{
//  QDomElement el = mDocument.createElement(tagName);
//  mCurrentParent.appendChild(el);
//  mCurrentElement = el;
//  el.appendChild(mDocument.createTextNode(value.toString(Qt::ISODate)));
//}

///*!
//  Inserts a tag with name \a tagName which contains the QTime \a value.

//  <b>example output:</b>
//  \code
//  <tagName>09:50:27</tagName>
//  \endcode
  
//  \see putDate, putDateTime
//*/
//void QXmlPut::putTime(const QString &tagName, const QTime &value)
//{
//  QDomElement el = mDocument.createElement(tagName);
//  mCurrentParent.appendChild(el);
//  mCurrentElement = el;
//  el.appendChild(mDocument.createTextNode(value.toString(Qt::ISODate)));
//}

///*!
//  Inserts a tag with name \a tagName which contains the QDateTime \a value.

//  <b>example output:</b>
//  \code
//  <tagName>2012-04-03T09:50:27</tagName>
//  \endcode
  
//  \see putDate, putTime
//*/
//void QXmlPut::putDateTime(const QString &tagName, const QDateTime &value)
//{
//  QDomElement el = mDocument.createElement(tagName);
//  mCurrentParent.appendChild(el);
//  mCurrentElement = el;
//  el.appendChild(mDocument.createTextNode(value.toString(Qt::ISODate)));
//}

///*!
//  Inserts a tag with name \a tagName which contains the QByteArray \a value.
  
//  The data of the QByteArray is saved in base-64. By setting \a compression to a number between 0
//  and 9, the data may be saved uncompressed to highly compressed.
//  To improve handling with text editors, every \a blockWidth characters, a linebreak is inserted
//  into the XML output.

//  <b>example output:</b>
//  \code
//  <tagName compression="9"><![CDATA[AAAPmnjatVdrUFNnGoa6u7h1KZ261k4i4
//oxbnN2t0B0I4SLEdqhgUXBXuVTCZU2rLi
//cBjykBBUldRi2yR7brKlVAVFQkgYOEnnB
///tk/O+bHmeR87/fN9z==
//]]></tagName>
//  \endcode
  
//  \see putImage
//*/
//void QXmlPut::putByteArray(const QString &tagName, const QByteArray &value, int blockWidth, int compression)
//{
//  QDomElement el = mDocument.createElement(tagName);
//  mCurrentParent.appendChild(el);
//  mCurrentElement = el;
//  compression = qBound(0, compression, 9);
//  QByteArray data;
//  if (compression > 0)
//  {
//    el.setAttribute("compression", QString::number(compression));
//    data = qCompress(value, compression).toBase64();
//  } else
//    data = value.toBase64();
  
//  QString newLine = "\n";
//  QByteArray sepData;
//  sepData.reserve(data.size() + (int)(data.size()/(double)blockWidth+0.5)*newLine.size());
//  int p = 0;
//  while (p < data.size())
//  {
//    int nextBlock = qMin(blockWidth, data.size()-p);
//    sepData.append(data.data()+p, nextBlock);
//    sepData.append(newLine);
//    p += nextBlock;
//  }
//  el.appendChild(mDocument.createCDATASection(QString(sepData)));
//}

///*!
//  Inserts a tag with name \a tagName which contains the QImage \a value.
  
//  The data of the QImage is saved in base-64. Compression is determined depending on the \a format,
//  "JPEG" and "JPG" are saved uncompressed (because they are compressed already), all others are
//  compressed. To improve handling with text editors, every \a blockWidth characters, a linebreak is
//  inserted into the XML output.
  
//  Available formats typically are: BMP, GIF, JPG, JPEG, PNG, PBM, PGM, PPM, TIFF, XBM, XPM.
//  (see QImageReader::supportedImageFormats())

//  <b>example output:</b>
//  \code
//  <tagName format="PNG" compression="9"><![CDATA[AAAPmnjatVdrUFNnGoa6u7h1KZ261k4i4
//oxbnN2t0B0I4SLEdqhgUXBXuVTCZU2rLi
//cBjykBBUldRi2yR7brKlVAVFQkgYOEnnB
///tk/O+bHmeR87/fN9z==
//]]></tagName>
//  \endcode
  
//  \see putImage
//*/
//void QXmlPut::putImage(const QString &tagName, const QImage &value, QString format, int blockWidth)
//{
//  QByteArray data;
//  QBuffer buff(&data);
//  buff.open(QBuffer::ReadWrite);
//  if (!value.save(&buff, format.toAscii().constData()))
//  {
//    qDebug() << FUNCNAME << "Couldn't write image to buffer with format" << format;
//    return;
//  }
//  buff.close();
//  QStringList noCompressFormats;
//  noCompressFormats << "JPG" << "JPEG";
//  bool compress = !noCompressFormats.contains(format.toUpper());
//  putByteArray(tagName, data, blockWidth, compress ? 9 : 0);
//  setAttributeString("format", format);
//}

///*!
//  Inserts a tag with name \a tagName which contains the QPen \a value.

//  <b>example output:</b>
//  \code
//  <tagName width="2" joinstyle="0" capstyle="32" miterlimit="2" color="#5018ff" penstyle="3"/>
//  \endcode
  
//  \see putBrush, putFont
//*/
//void QXmlPut::putPen(const QString &tagName, const QPen &value)
//{
//  putSingleTag(tagName);
//  setAttributeColor("color", value.color());
//  setAttributeInt("penstyle", value.style());
//  setAttributeDouble("width", value.widthF());
  
//  if (value.style() == Qt::CustomDashLine)
//  {
//    QVector<qreal> rv = value.dashPattern(); // just in case we're on ARM
//    QVector<double> dv(rv.size());
//    qCopy(rv.constBegin(), rv.constEnd(), dv.begin());
//    setAttributeDoubleVector("dashpattern", dv);
//    setAttributeDouble("dashoffset", value.dashOffset());
//  }
//  if (value.capStyle() != Qt::SquareCap)
//    setAttributeInt("capstyle", value.capStyle());
//  if (value.joinStyle() != Qt::BevelJoin)
//    setAttributeInt("joinstyle", value.joinStyle());
//  if (value.joinStyle() == Qt::MiterJoin)
//    setAttributeDouble("miterlimit", value.miterLimit());
//}

///*!
//  Inserts a tag with name \a tagName which contains the QBrush \a value.

//  <b>example output:</b>
//  \code
//  <tagName brushstyle="14" color="#5018ff"/>
//  \endcode
  
//  \see putPen, putFont
//*/
//void QXmlPut::putBrush(const QString &tagName, const QBrush &value)
//{
//  putSingleTag(tagName);
//  setAttributeColor("color", value.color());
//  setAttributeInt("brushstyle", value.style());
//}

///*!
//  Inserts a tag with name \a tagName which contains the QFont \a value.

//  <b>example output:</b>
//  \code
//  <tagName fontdescription="Monospace,24,-1,5,75,1,0,0,0,0"/>
//  \endcode
  
//  \see putPen, putBrush
//*/
//void QXmlPut::putFont(const QString &tagName, const QFont &value)
//{
//  putSingleTag(tagName);
//  setAttributeString("fontdescription", value.toString());
//}

/*!
  Returns a QXmlPut instance on the same Document and at the same position as this instance, but
  which is restricted to this hierarchy level (and the levels below). This means, the returned
  QXmlPut instance isn't allowed to \ref rise above the current hierarchy level.
  
  This is useful if you wish that subroutines can handle their own XML work without possibly
  interfering with the rest. By passing a restricted instance, it's guaranteed the subroutines
  don't accidentally write/read outside their designated XML element.
  
  If only the subroutine needs to write to/read from a specific element, consider using \ref
  descended.
  
  \b example:
  \code
  xmlPut.descend("toptag");
  xmlPut.putString("exampleTag", "test");
  writeOtherContent(xmlPut.restricted()); // A subroutine that writes to the <toptag> level
  xmlPut.rise();
  }
  \endcode
  
  \see descended
*/
QXmlPut QXmlPut::restricted()
{
  QXmlPut result(*this);
  result.mBarrierNode = result.mCurrentParent;
  return result;
}

/*!
  Creates a tag with the name \a tagName and descends into it. Child elements can then be created.
  
  Once the work in the lower hierarchy level is done, you can return to the previous position in
  the parent hierarchy level by calling \ref rise.
  
  If a subroutine needs to write to/read from a specific element, consider using \ref descended
  instead of a descend-rise-pair.
  
  \b example:
  \code
  xmlPut.descend("toptag");
  xmlPut.putString("exampleTag", "test");
  xmlPut.descend("subtag");
  xmlPut.putInt("answer", 42);
  xmlPut.rise();
  xmlPut.rise();
  \endcode
  
  \see rise, descended
*/
void QXmlPut::descend(const QString &tagName)
{
  QDomElement el = mDocument.createElement(tagName);
  mCurrentParent.appendChild(el);
  mCurrentElement = el;
  mCurrentParent = el;
}

/*!
  Returns a QXmlPut instance that is descended into and restricted to the current element. Child
  elements can then be created with the returned instance normally via \ref putString etc.
  
  Due to the restriction, the returned instance can't rise above its initial hierarchy level, i.e.
  into or above the hierarchy level of the instance this function is called on.
  
  When descending into elements like this, there is no need to call \ref rise (and thus no
  possibility to forget a \ref rise), because the current instance isn't influenced. Whatever
  descending/rising the subroutine does with the returned instance can't break the callers XML
  handling code.
  
  \b example:
  \code
  writeHeaderSubroutine(xmlPut.descended("header"));
  writeBodySubroutine(xmlPut.descended("body"));
  writeFooterSubroutine(xmlPut.descended("footer"));
  \endcode
  
  \see restricted, descend
*/
QXmlPut QXmlPut::descended(const QString &tagName)
{
  QXmlPut result(*this);
  result.descend(tagName);
  result.mBarrierNode = result.mCurrentParent;
  mCurrentElement = result.mCurrentParent;
  return result;
}

/*!
  Rises to the previous position in the parent hierarchy level. This finishes the work in a lower
  hierarchy level that was started with \ref descend earlier.
  
  If a subroutine needs to write to/read from a specific element, consider using \ref descended
  instead of a descend-rise-pair.
  
  If this instance is restricted (see \ref descended and \ref restricted) and is already inside its
  highest allowed hierarchy level, a further attempt to \ref rise will return false without
  changing the current hierarchy level, and cause a corresponding qDebug output.
  
  \b example:
  \code
  xmlPut.descend("toptag");
  xmlPut.putString("exampleTag", "test");
  xmlPut.descend("subtag");
  xmlPut.putInt("answer", 42);
  xmlPut.rise();
  xmlPut.rise();
  \endcode
  
  \see descend
*/
bool QXmlPut::rise()
{
  if (mCurrentParent == mBarrierNode)
  {
    qDebug() << FUNCNAME << "attept to rise beyond allowed node";
    return false;
  }
  
  QDomNode node = mCurrentParent.parentNode();
  if (!node.isNull())
  {
    if (node.isElement())
    {
      QDomElement newParent = node.toElement();
      mCurrentElement = mCurrentParent;
      mCurrentParent = newParent;
      if (mCurrentElement.isNull())
        mCurrentElement = newParent;
      return true;
    } else
      qDebug() << FUNCNAME << "Attempt to rise into non-element node";
  } else
    qDebug() << FUNCNAME << "Attempt to rise above document node";
  return false;
}

/*!
  Changes the current parent element to \a parentElement. It must be in the current QDomDocument
  already.
  
  If this instance is restricted (see \ref descended and \ref restricted) and \a parentElement is
  not inside the allowed hierarchy, this function will return false without changing the current
  position, and cause a corresponding qDebug output.
  
  You probably won't use this function very often, since normal, linear QXmlPutGet navigation
  should be done with \ref descend, \ref rise, \ref descended, etc. However, it's useful
  for nonlinear XML navigation, where frequent jumps between different locations in the XML
  hierarchy need to be done.
  
  \see element
*/
bool QXmlPut::goTo(QDomElement parentElement)
{
  // check whether this instance is allowed to jump to the element:
  QDomElement el = parentElement;
  while (!el.isNull() && el != mBarrierNode)
    el = el.parentNode().toElement();
  if (el.isNull())
  {
    qDebug() << FUNCNAME << "attempt to jump outside of allowed tree";
    return false;
  }
         
  // perform jump:
  if (parentElement.ownerDocument() == mDocument)
  {
    mCurrentParent = parentElement;
    mCurrentElement = parentElement.lastChildElement();
    if (mCurrentElement.isNull())
      mCurrentElement = parentElement;
    return true;
  } else
    qDebug() << FUNCNAME << "Attempt to go to element not in document";
  return false;
}

/*!
  Adds an attribute with \a name to the current element. The attribute will carry the string \a
  value.
  
  <b>example output:</b>
  \code
  <tagName name="example string"/>
  \endcode
*/
void QXmlPut::setAttributeString(const QString &name, const QString &value)
{
  mCurrentElement.setAttribute(name, value);
}

/*!
  Adds an attribute with \a name to the current element. The attribute will carry the int \a value.
  
  <b>example output:</b>
  \code
  <tagName name="2"/>
  \endcode
*/
void QXmlPut::setAttributeInt(const QString &name, int value)
{
  mCurrentElement.setAttribute(name, QString::number(value));
}

/*!
  Adds an attribute with \a name to the current element. The attribute will carry the int vector \a
  value.
  
  <b>example output:</b>
  \code
  <tagName name="2;4;6;8;10"/>
  \endcode
*/
//void QXmlPut::setAttributeIntVector(const QString &name, const QVector<int> &value)
//{
//  mCurrentElement.setAttribute(name, intVectorToStr(value));
//}

/*!
  Adds an attribute with \a name to the current element. The attribute will carry the double \a
  value.
  
  <b>example output:</b>
  \code
  <tagName name="3.623"/>
  \endcode
*/
void QXmlPut::setAttributeDouble(const QString &name, double value)
{
  mCurrentElement.setAttribute(name, QString::number(value));
}

/*!
  Adds an attribute with \a name to the current element. The attribute will carry the double vector
  \a value.
  
  <b>example output:</b>
  \code
  <tagName name="3.6;4.2;9.9"/>
  \endcode
*/
//void QXmlPut::setAttributeDoubleVector(const QString &name, const QVector<double> &value)
//{
//  mCurrentElement.setAttribute(name, doubleVectorToStr(value));
//}

/*!
  Adds an attribute with \a name to the current element. The attribute will carry the bool \a value.
  
  Depending on the specified \a format (see \ref QXmlPutGet::BoolFormat), the boolean value is
  represented with different strings.
  
  <b>example output:</b>
  \code
  <tagName name="no"/>
  \endcode
*/
void QXmlPut::setAttributeBool(const QString &name, bool value, QXmlPutGet::BoolFormat format)
{
  mCurrentElement.setAttribute(name, boolToStr(value, format));
}

/*!
  Adds an attribute with \a name to the current element. The attribute will carry the bool vector \a value.
  
  Depending on the specified \a format (see \ref QXmlPutGet::BoolFormat), the boolean value is
  represented with different strings.
  
  <b>example output:</b>
  \code
  <tagName name="no;yes;yes;no;yes"/>
  \endcode
*/
//void QXmlPut::setAttributeBoolVector(const QString &name, const QVector<bool> &value, QXmlPutGet::BoolFormat format)
//{
//  mCurrentElement.setAttribute(name, boolVectorToStr(value, format));
//}

/*!
  Adds an attribute with \a name to the current element. The attribute will carry the QColor \a value.
  
  If the color contains no transparency component (i.e. alpha is 255), the color is saved in the
  format "#rrggbb". If it contains alpha, it is saved as "#rrggbbaa".
  
  <b>example output:</b>
  \code
  <tagName name="#83A0FF"/>
  \endcode
*/
//void QXmlPut::setAttributeColor(const QString &name, const QColor &value)
//{
//  mCurrentElement.setAttribute(name, colorToStr(value));
//}

/*!
  Returns the XML document as a string. Sub-elements are indented by \a spacesPerIndent spaces.
  
  \see QXmlGet::fromString
*/
QString QXmlPut::toString(int spacesPerIndent) const
{
  return mDocument.toString(spacesPerIndent);
}

/*!
  Saves the XML document to the file \a fileName. Sub-elements are indented by \a spacesPerIndent spaces.
  
  The encoding (typically UTF-8) specified in the XML declaration (QXmlPut constructor) is used.
  
  This function returns false if the file couldn't be created/written to.
  
  \see QXmlGet::load
*/
bool QXmlPut::save(const QString &fileName, int spacesPerIndent) const
{
  QFile file(fileName);
  if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
  {
    QTextStream stream(&file);
    mDocument.save(stream, spacesPerIndent, QDomNode::EncodingFromDocument);
    file.close();
    return true;
  } else
    qDebug() << FUNCNAME << "Couldn't open file for writing:" << fileName;
  
  return false;
}

QString QXmlPut::boolToStr(bool value, QXmlPutGet::BoolFormat format)
{
  switch (format)
  {
    case QXmlPutGet::bfYesNo: return value ? "yes" : "no";
    case QXmlPutGet::bfTrueFalse: return value ? "true" : "false";
    case QXmlPutGet::bf10: return value ? "1" : "0";
    default:
    {
      qDebug() << FUNCNAME << "Invalid QXmlPutGet::BoolFormat format, using bfYesNo" << (int)format;
      return value ? "yes" : "no";
    }
  }
}

//QString QXmlPut::colorToStr(const QColor &color)
//{
//  QString result = color.name();
//  if (color.alpha() < 255 && result.startsWith("#"))
//    result.append(QString::number(color.alpha(), 16));
//  return result;
//}

//QString QXmlPut::intVectorToStr(const QVector<int> &value)
//{
//  QString result;
//  for (int i=0; i<value.size(); ++i)
//  {
//    if (i>0)
//      result.append(";"+QString::number(value.at(i)));
//    else
//      result.append(QString::number(value.at(i)));
//  }
//  return result;
//}

//QString QXmlPut::doubleVectorToStr(const QVector<double> &value)
//{
//  QString result;
//  for (int i=0; i<value.size(); ++i)
//  {
//    if (i>0)
//      result.append(";"+QString::number(value.at(i)));
//    else
//      result.append(QString::number(value.at(i)));
//  }
//  return result;
//}

//QString QXmlPut::boolVectorToStr(const QVector<bool> &value, QXmlPutGet::BoolFormat format)
//{
//  QString result;
//  for (int i=0; i<value.size(); ++i)
//  {
//    if (i>0)
//      result.append(";"+boolToStr(value.at(i), format));
//    else
//      result.append(boolToStr(value.at(i), format));
//  }
//  return result;
//}


// ================================================================================
// =================== QXMLGet
// ================================================================================

/* start documentation of inline functions */

/*! \fn QDomElement QXmlGet::element() const
  
  Returns the current element as a QDomElement. This can be used as a jump target for \ref goTo, or
  some low-level manipulation of the node with the Qt-DOM-interface.
*/

/*! \fn QString QXmlGet::xmlDeclaration() const
  
  Returns the XML-Declaration (the tag name of the <tt>&lt;? .. ?&gt;</tt> header), which always is "xml".
  
  (Thus this function is rather redundant, but the DOM provides it, so it is provided here, too.)
*/

/*! \fn QString QXmlGet::xmlVersion() const
  
  Returns the xml version of the document. Typically this is "1.0" or "1.1".
  
  \b example:
  \code
  <?xml version="1.0" encoding="UTF-8"?>
  (...)
  \endcode
*/

/*! \fn QString QXmlGet::encoding() const
  
  Returns the encoding of the document, e.g. "UTF-8".
  
  \b example:
  \code
  <?xml version="1.0" encoding="UTF-8"?>
  (...)
  \endcode
*/

/*! \fn bool QXmlGet::standalone() const
  
  Returns the standalone-property of the document's xml declaration.
  
  \b example:
  \code
  <?xml version="1.0" encoding="UTF-8"?>
  \endcode
  has standalone false
  \code
  <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
  \endcode
  has standalone true.
*/

/*! \fn QDomDocument QXmlGet::document() const
  
  Returns the QDomDocument this QXmlGet instance is using.
*/

/* end documentation of inline functions */

/*!
  Creates a QXmlGet instance with an empty document. Call \ref fromString or \ref load to conveniently load XML content
  to the document. If you have a QDomDocument ready, use the \ref QXmlGet(QDomDocument document) constructor instead.
*/
QXmlGet::QXmlGet() :
  mStandalone(false)
{
}

/*! \overload
  
  Creates a QXmlGet instance from the passed QDomDocument. This instance can then read from the document.
*/
QXmlGet::QXmlGet(QDomDocument document) :
  mDocument(document),
  mXmlDeclaration(""),
  mXmlVersion(""),
  mEncoding(""),
  mStandalone(false)
{
  if (document.isNull())
  {
    qDebug() << FUNCNAME << "Attempt to open a QDomDocument that is null";
  } else
  {
    // parse xml header declaration:
    QDomNode node = mDocument.firstChild();
    if (node.isProcessingInstruction())
    {
      QDomProcessingInstruction procInst = node.toProcessingInstruction();
      mXmlDeclaration = procInst.target();
      QString pseudoParams = procInst.data();
      QRegExp regExp("(version|encoding|standalone) *= *(?:\"|')([^\"' ]*)(?:\"|')");
      int p = regExp.indexIn(pseudoParams);
      while (p > -1)
      {
        if (regExp.captureCount()==2 && regExp.cap(1)=="version")
          mXmlVersion = regExp.cap(2);
        else if (regExp.captureCount()==2 && regExp.cap(1)=="encoding")
          mEncoding = regExp.cap(2);
        else if (regExp.captureCount()==2 && regExp.cap(1)=="standalone")
          mStandalone = regExp.cap(2)=="yes";
        p = regExp.indexIn(pseudoParams, p+regExp.matchedLength());
      }
    }
  }
  // initialize traversal variables to root node:
  mCurrentParent = mDocument.documentElement();
  mCurrentElement = mCurrentParent;
  mBarrierNode = mCurrentParent;
}


/*! \overload
  
  Creates a QXmlGet instance from the QXmlPut instance \a xmlPut. This way you may now read from
  the document, to which the QXmlPut instance writes.
*/
QXmlGet::QXmlGet(const QXmlPut &xmlPut):
  mDocument(xmlPut.mDocument),
  mBarrierNode(xmlPut.mBarrierNode),
  mCurrentParent(xmlPut.mCurrentParent),
  mCurrentElement(xmlPut.mCurrentElement),
  mXmlDeclaration(xmlPut.mXmlDeclaration),
  mXmlVersion(xmlPut.mXmlVersion),
  mEncoding(xmlPut.mEncoding),
  mStandalone(xmlPut.mStandalone)
{
}

/*!
  Returns the doctype name.
  
  \b example:
  \code
  <?xml version="1.0" encoding="UTF-8"?>
  <!DOCTYPE myDocType PUBLIC 'myPublicId' 'mySystemId'>
  <root>
    (...)
  </root>
  \endcode
  has the doctype "myDocType".
*/
QString QXmlGet::docType() const
{
  return mDocument.doctype().name();
}

/*!
  Returns the public identifier of the external DTD subset or an empty string if there is no public identifier.
  
  \b example:
  \code
  <?xml version="1.0" encoding="UTF-8"?>
  <!DOCTYPE myDocType PUBLIC 'myPublicId' 'mySystemId'>
  <root>
    (...)
  </root>
  \endcode
  has the public identifier "myPublicId".
*/
QString QXmlGet::publicId() const
{
  return mDocument.doctype().publicId();
}

/*!
  Returns the system identifier of the external DTD subset or an empty string if there is no system identifier.
  
  \b example:
  \code
  <?xml version="1.0" encoding="UTF-8"?>
  <!DOCTYPE myDocType PUBLIC 'myPublicId' 'mySystemId'>
  <root>
    (...)
  </root>
  \endcode
  has the system identifier "mySystemId".
*/
QString QXmlGet::systemId() const
{
  return mDocument.doctype().systemId();
}

QXmlGet::~QXmlGet()
{
}

/*!
  Finds the first element named \a tagName in the current hierarchy level. If this returns true, an
  element was found. You then can access contents and attributes of this element via the get(...) functions.
  
  For example, if you expect the element to contain a string as in
  \verbatim<element>Hellothere!</element>\endverbatim call \ref getString to retrieve "Hello there!".
  
  If you expect the element to have further child elements and want to descend into their hierarchy
  level, call \ref descend or \ref descended.
  
  If you expect more than one element with the name \a tagName in the current hierarchy level,
  consider using \ref findNext instead, because \ref find would always just find the first element.
  
  \see findNext, findAndDescend, findNextAndDescend, findReset
*/
bool QXmlGet::find(const QString &tagName)
{
  QDomElement el = mCurrentParent.firstChildElement(tagName);
  if (!el.isNull())
  {
    mCurrentElement = el;
    return true;
  } else
    return false;
}

/*!
  Finds the first element with the specified \a tagName in the current hierarchy level and (if
  found) descends into it. If such an element is found, the function returns true. In that case you
  must make sure to call \ref rise after the work in the lower hierarchy level is done.
  
  If you expect more than one element with the name \a tagName in the current hierarchy level,
  consider using \ref findNextAndDescend instead, because \ref findAndDescend would always just
  find the first element.
  
  \see findNextAndDescend, find, descend, descended
*/
bool QXmlGet::findAndDescend(const QString &tagName)
{
  if (find(tagName))
  {
    descend();
    return true;
  } else
    return false;
}

/*!
  Resets the current element to the starting position (this means it will be pointing to the parent
  of the current hierarchy level). Any subsequent call to \ref findNext or \ref findNextAndDescend
  would start searching from the beginning. Calls to \ref find are not affected, since it always
  starts searching from the beginning.
  
  \see findNext, findNextAndDescend, find
*/
void QXmlGet::findReset()
{
  mCurrentElement = mCurrentParent;
}

/*!
  finds the next element after the current element with tag name \a tagName.
  
  Returns true when an element was found. This makes it possible to use it directly as a
  condition of a while-loop, looping over all elements with name \a tagName. If you want to descend
  into every such element, consider using \ref findNextAndDescend.
  
  Once no next element was found, the function returns false and the current element is reset. This
  means a subsequent call would restart the loop from the beginning, for example searching for a
  different \a tagName.
  
  \see findReset, findNextAndDescend, find
*/
bool QXmlGet::findNext(const QString &tagName)
{
  QDomElement el;
  if (mCurrentParent == mCurrentElement)
    el = mCurrentParent.firstChildElement(tagName);
  else
    el = mCurrentElement.nextSiblingElement(tagName);
  if (!el.isNull())
  {
    mCurrentElement = el;
    return true;
  } else
  {
    mCurrentElement = mCurrentParent;
    return false;
  }
}

/*!
  finds the next element after the current element with tag name \a tagName and descends into it.
  
  Returns true when an element was found. This makes it possible to use it directly as a
  condition of a while-loop, looping over (and descending into) all elements with name \a tagName.
  Don't forget to call \ref rise at the end of each iteration.
  
  Once no next element was found, the function returns false and the current element is reset. This
  means a subsequent call would restart the loop from the beginning, for example searching for a
  different \a tagName.
  
  \see findNext, findReset, find
*/
bool QXmlGet::findNextAndDescend(const QString &tagName)
{
  if (findNext(tagName))
  {
    descend();
    return true;
  } else
    return false;
}

/*!
  Returns the number of elements with the specified tag name inside the current element. If \a
  tagName is empty, all elements are counted.
  
  \see hasChildren
*/
int QXmlGet::childCount(const QString &tagName) const
{
  if (tagName.isEmpty())
  {
    int c = 0;
    QDomElement el = mCurrentElement.firstChildElement();
    while (!el.isNull())
    {
      ++c;
      el = el.nextSiblingElement();
    }
    return c;
  } else
    return mCurrentElement.elementsByTagName(tagName).count();
}

/*!
  Returns whether the current element has children (sub elements). If so, you can call \ref descend
  or \ref descended to change into their hierarchy level to access them.
  
  \see childCount
*/
bool QXmlGet::hasChildren() const
{
  return !mCurrentElement.firstChildElement().isNull();
}

/*!
  Returns the tag name of the current element.

  For example, the element <tt>\verbatim<rect width="4">\endverbatim</tt> has the tag name "rect".
*/
QString QXmlGet::tagName() const
{
  return mCurrentElement.tagName();
}

/*!
  Returns the string the current tag contains. If the current tag doesn't contain a string, \a
  defaultValue is returned.
  
  \see getStringList
*/
QString QXmlGet::getString(const QString &defaultValue) const
{
  QDomNode node = mCurrentElement.firstChild();
  if (node.isNull())
    return defaultValue;
  
  if (node.isCDATASection())
  {
    // CDATA sections may be opened and closed multiple times inside a single text tag, so we get all of them:
    QString result;
    result.append(node.toCDATASection().data());
    node = node.nextSibling();
    while (!node.isNull())
    {
      if (node.isCDATASection())
        result.append(node.toCDATASection().data());
      node = node.nextSibling();
    }
    return result;
  } else if (node.isText())
  {
    return node.toText().data();
  }
  
  return defaultValue;
}

/*!
  Returns the string list the current tag contains. If the current tag doesn't contain a string
  list, \a defaultValue is returned.
  
  \see getString
*/
QStringList QXmlGet::getStringList(const QStringList &defaultValue)
{
  if (mCurrentElement == mCurrentParent)
    return defaultValue;
  descend();
  QStringList result;
  while (findNext("li"))
    result.append(getString());
  rise();
  return result;
}

/*!
  Returns the integer the current tag contains. If the current tag doesn't contain an integer, \a
  defaultValue is returned.
  
  \see getIntVector
*/
int QXmlGet::getInt(int defaultValue) const
{
  QDomNode node = mCurrentElement.firstChild();
  if (node.isNull() || !node.isText())
    return defaultValue;
  QString value = node.toText().data();
  
  bool ok;
  int result = value.toInt(&ok);
  if (ok) return result;
  
  return defaultValue;
}

/*!
  Returns the integer vector the current tag contains. If the current tag doesn't contain an
  integer vector, \a defaultValue is returned.
  
  \see getInt
*/
//QVector<int> QXmlGet::getIntVector(const QVector<int> &defaultValue) const
//{
//  QDomNode node = mCurrentElement.firstChild();
//  if (node.isNull() || !node.isText())
//    return defaultValue;
//  bool ok;
//  QVector<int> result = strToIntVector(node.toText().data(), &ok);
//  if (ok)
//    return result;
//  return defaultValue;
//}

/*!
  Returns the double the current tag contains. If the current tag doesn't contain a double, \a
  defaultValue is returned.
  
  \see getDoubleVector
*/
double QXmlGet::getDouble(double defaultValue) const
{
  QDomNode node = mCurrentElement.firstChild();
  if (node.isNull() || !node.isText())
    return defaultValue;
  QString value = node.toText().data();
  
  bool ok;
  double result = value.toDouble(&ok);
  if (ok) return result;
  
  return defaultValue;
}

/*!
  Returns the double vector the current tag contains. If the current tag doesn't contain a double
  vector, \a defaultValue is returned.
  
  \see getDouble
*/
//QVector<double> QXmlGet::getDoubleVector(const QVector<double> &defaultValue) const
//{
//  QDomNode node = mCurrentElement.firstChild();
//  if (node.isNull() || !node.isText())
//    return defaultValue;
//  bool ok;
//  QVector<double> result = strToDoubleVector(node.toText().data(), &ok);
//  if (ok)
//    return result;
//  return defaultValue;
//}

/*!
  Returns the bool the current tag contains. If the current tag doesn't contain a bool, \a
  defaultValue is returned.
  
  by setting \a formats, you may specify which boolean formats this function accepts.
  
  \see getBoolVector, QXmlPutGet::BoolFormat
*/
bool QXmlGet::getBool(bool defaultValue, QXmlPutGet::BoolFormats formats) const
{
  QDomNode node = mCurrentElement.firstChild();
  if (node.isNull() || !node.isText())
    return defaultValue;
  QString value = node.toText().data();
  
  bool ok;
  bool result = strToBool(value, &ok, formats);
  if (ok) return result;
  
  return defaultValue;
}

/*!
  Returns the bool vector the current tag contains. If the current tag doesn't contain a bool
  vector, \a defaultValue is returned.
  
  by setting \a formats, you may specify which boolean formats this function accepts.
  
  \see getBool, QXmlPutGet::BoolFormat
*/
//QVector<bool> QXmlGet::getBoolVector(const QVector<bool> &defaultValue, QXmlPutGet::BoolFormats formats) const
//{
//  QDomNode node = mCurrentElement.firstChild();
//  if (node.isNull() || !node.isText())
//    return defaultValue;
//  QStringList valueList = node.toText().data().split(";");
//  QVector<bool> result;
//  bool ok;
//  for (int i=0; i<valueList.size(); ++i)
//  {
//    result.append(strToBool(valueList.at(i), &ok, formats));
//    if (!ok) return defaultValue;
//  }
//  return result;
//}

/*!
  Returns the QColor the current tag contains. If the current tag doesn't contain a QColor, \a
  defaultValue is returned.
*/
//QColor QXmlGet::getColor(const QColor &defaultValue) const
//{
//  QDomNode node = mCurrentElement.firstChild();
//  if (node.isNull() || !node.isText())
//    return defaultValue;
//  QString value = node.toText().data();
//  bool ok;
//  QColor result = strToColor(value, &ok);
//  if (ok)
//    return result;
//  return defaultValue;
//}

///*!
//  Returns the QSize the current tag contains. If the current tag doesn't contain a QSize, \a
//  defaultValue is returned.
  
//  \see getSizeF
//*/
//QSize QXmlGet::getSize(const QSize &defaultValue) const
//{
//  if (!hasAttribute("width") || !hasAttribute("height"))
//    return defaultValue;
//  QSize result;
//  result.setWidth(getAttributeInt("width"));
//  result.setHeight(getAttributeInt("height"));
//  return result;
//}

///*!
//  Returns the QSizeF the current tag contains. If the current tag doesn't contain a QSizeF, \a
//  defaultValue is returned.
  
//  \see getSize
//*/
//QSizeF QXmlGet::getSizeF(const QSizeF &defaultValue) const
//{
//  if (!hasAttribute("width") || !hasAttribute("height"))
//    return defaultValue;
//  QSizeF result;
//  result.setWidth(getAttributeDouble("width"));
//  result.setHeight(getAttributeDouble("height"));
//  return result;
//}

///*!
//  Returns the QPoint the current tag contains. If the current tag doesn't contain a QPoint, \a
//  defaultValue is returned.
  
//  \see getPointF
//*/
//QPoint QXmlGet::getPoint(const QPoint &defaultValue) const
//{
//  if (!hasAttribute("x") || !hasAttribute("y"))
//    return defaultValue;
//  QPoint result;
//  result.setX(getAttributeInt("x"));
//  result.setY(getAttributeInt("y"));
//  return result;
//}

///*!
//  Returns the QPointF the current tag contains. If the current tag doesn't contain a QPointF, \a
//  defaultValue is returned.
  
//  \see getPoint
//*/
//QPointF QXmlGet::getPointF(const QPointF &defaultValue) const
//{
//  if (!hasAttribute("x") || !hasAttribute("y"))
//    return defaultValue;
//  QPointF result;
//  result.setX(getAttributeDouble("x"));
//  result.setY(getAttributeDouble("y"));
//  return result;
//}

///*!
//  Returns the QRect the current tag contains. If the current tag doesn't contain a QRect, \a
//  defaultValue is returned.
  
//  \see getRectF
//*/
//QRect QXmlGet::getRect(const QRect &defaultValue) const
//{
//  if (!hasAttribute("left") || !hasAttribute("top") || !hasAttribute("width") || !hasAttribute("height"))
//    return defaultValue;
//  QRect result;
//  result.setLeft(getAttributeInt("left"));
//  result.setTop(getAttributeInt("top"));
//  result.setWidth(getAttributeInt("width"));
//  result.setHeight(getAttributeInt("height"));
//  return result;
//}

///*!
//  Returns the QRectF the current tag contains. If the current tag doesn't contain a QRectF, \a
//  defaultValue is returned.
  
//  \see getRect
//*/
//QRectF QXmlGet::getRectF(const QRectF &defaultValue) const
//{
//  if (!hasAttribute("left") || !hasAttribute("top") || !hasAttribute("width") || !hasAttribute("height"))
//    return defaultValue;
//  QRectF result;
//  result.setLeft(getAttributeDouble("left"));
//  result.setTop(getAttributeDouble("top"));
//  result.setWidth(getAttributeDouble("width"));
//  result.setHeight(getAttributeDouble("height"));
//  return result;
//}

///*!
//  Returns the QDate the current tag contains. If the current tag doesn't contain a QDate, \a
//  defaultValue is returned.
  
//  \see getDateTime, getTime
//*/
//QDate QXmlGet::getDate(const QDate &defaultValue) const
//{
//  QDomNode node = mCurrentElement.firstChild();
//  if (node.isNull() || !node.isText())
//    return defaultValue;
//  QString value = node.toText().data();
  
//  QDate result = QDate::fromString(value, Qt::ISODate);
//  if (result.isValid()) return result;
  
//  return defaultValue;
//}

///*!
//  Returns the QTime the current tag contains. If the current tag doesn't contain a QTime, \a
//  defaultValue is returned.
  
//  \see getDateTime, getDate
//*/
//QTime QXmlGet::getTime(const QTime &defaultValue) const
//{
//  QDomNode node = mCurrentElement.firstChild();
//  if (node.isNull() || !node.isText())
//    return defaultValue;
//  QString value = node.toText().data();
  
//  QTime result = QTime::fromString(value, Qt::ISODate);
//  if (result.isValid()) return result;
  
//  return defaultValue;
//}

///*!
//  Returns the QDateTime the current tag contains. If the current tag doesn't contain a QDateTime, \a
//  defaultValue is returned.
  
//  \see getTime, getDate
//*/
//QDateTime QXmlGet::getDateTime(const QDateTime &defaultValue) const
//{
//  QDomNode node = mCurrentElement.firstChild();
//  if (node.isNull() || !node.isText())
//    return defaultValue;
//  QString value = node.toText().data();
  
//  QDateTime result = QDateTime::fromString(value, Qt::ISODate);
//  if (result.isValid()) return result;
  
//  return defaultValue;
//}

///*!
//  Returns the QByteArray the current tag contains. If the current tag doesn't contain a QByteArray, \a
//  defaultValue is returned.
//*/
//QByteArray QXmlGet::getByteArray(const QByteArray &defaultValue) const
//{
//  QDomNode node = mCurrentElement.firstChild();
//  bool compressed = getAttributeInt("compression", 0) > 0;
//  if (!node.isNull() && node.isCDATASection())
//  {
//    QByteArray data(node.toCDATASection().data().toAscii());
//    QString newLine = "\n";
//    data.replace(newLine.toAscii(), "");
//    data = QByteArray::fromBase64(data);
//    if (compressed)
//      data = qUncompress(data);
//    return data;
//  }
//  return defaultValue;
//}

///*!
//  Returns the QImage the current tag contains. If the current tag doesn't contain a QImage, \a
//  defaultValue is returned.
//*/
//QImage QXmlGet::getImage(const QImage &defaultValue) const
//{
//  if (!hasAttribute("format"))
//    return defaultValue;
//  QString format = getAttributeString("format");
//  QByteArray data = getByteArray();
//  QBuffer buff(&data);
//  buff.open(QBuffer::ReadOnly);
//  QImage result;
//  if (!result.load(&buff, format.toAscii().constData()))
//    return defaultValue;
//  buff.close();
//  return result;
//}

///*!
//  Returns the QPen the current tag contains. If the current tag doesn't contain a QPen, \a
//  defaultValue is returned.
  
//  \see getBrush
//*/
//QPen QXmlGet::getPen(const QPen &defaultValue) const
//{
//  QPen result = defaultValue;
//  if (hasAttribute("color"))
//    result.setColor(getAttributeColor("color", result.color()));
//  if (hasAttribute("penstyle"))
//    result.setStyle((Qt::PenStyle)getAttributeInt("penstyle", result.style()));
//  if (hasAttribute("width"))
//    result.setWidthF(getAttributeDouble("width", result.widthF()));
  
//  if (result.style() == Qt::CustomDashLine)
//  {
//    QVector<double> dv = getAttributeDoubleVector("dashpattern");
//    if (!dv.isEmpty())
//    {
//      QVector<qreal> rv(dv.size());
//      qCopy(dv.constBegin(), dv.constEnd(), rv.begin());
//      result.setDashPattern(rv);
//      if (hasAttribute("dashoffset"))
//        result.setDashOffset(getAttributeDouble("dashoffset", result.dashOffset()));
//    } else
//      result.setStyle(defaultValue.style());
//  }
//  if (hasAttribute("capstyle"))
//    result.setCapStyle((Qt::PenCapStyle)getAttributeInt("capstyle", result.capStyle()));
//  if (hasAttribute("joinstyle"))
//    result.setJoinStyle((Qt::PenJoinStyle)getAttributeInt("joinstyle", result.joinStyle()));
//  if (hasAttribute("miterlimit"))
//    result.setMiterLimit(getAttributeDouble("miterlimit", result.miterLimit()));
//  return result;
//}

///*!
//  Returns the QBrush the current tag contains. If the current tag doesn't contain a QBrush, \a
//  defaultValue is returned.
  
//  \see getPen
//*/
//QBrush QXmlGet::getBrush(const QBrush &defaultValue) const
//{
//  QBrush result = defaultValue;
//  if (hasAttribute("color"))
//    result.setColor(getAttributeColor("color", result.color()));
//  if (hasAttribute("brushstyle"))
//    result.setStyle((Qt::BrushStyle)getAttributeInt("brushstyle", result.style()));
//  return result;
//}

///*!
//  Returns the QFont the current tag contains. If the current tag doesn't contain a QFont, \a
//  defaultValue is returned.
//*/
//QFont QXmlGet::getFont(const QFont &defaultValue) const
//{
//  QFont result = defaultValue;
//  if (hasAttribute("fontdescription"))
//    result.fromString(getAttributeString("fontdescription"));
//  return result;
//}

/*!
  Returns whether the current tag has an attribute with the name \a name.
*/
bool QXmlGet::hasAttribute(const QString &name) const
{
  return mCurrentElement.hasAttribute(name);
}

/*!
  Returns the string the current tag carries as attribute with the name \a name. If the current tag
  doesn't contain such an attribute,\a defaultValue is returned.
*/
QString QXmlGet::getAttributeString(const QString &name, const QString &defaultValue) const
{
  if (mCurrentElement.hasAttribute(name))
    return mCurrentElement.attribute(name);
  
  return defaultValue;
}

/*!
  Returns the integer the current tag carries as attribute with the name \a name. If the current tag
  doesn't contain such an attribute,\a defaultValue is returned.
  
  \see getAttributeIntVector
*/
int QXmlGet::getAttributeInt(const QString &name, int defaultValue) const
{
  if (mCurrentElement.hasAttribute(name))
  {
    QString value = mCurrentElement.attribute(name);
    bool ok;
    int result = value.toInt(&ok);
    if (ok) return result;
  }
  return defaultValue;
}

/*!
  Returns the integer vector the current tag carries as attribute with the name \a name. If the current tag
  doesn't contain such an attribute,\a defaultValue is returned.
  
  \see getAttributeInt
*/
//QVector<int> QXmlGet::getAttributeIntVector(const QString &name, const QVector<int> &defaultValue) const
//{
//  if (mCurrentElement.hasAttribute(name))
//  {
//    QString value = mCurrentElement.attribute(name);
//    bool ok;
//    QVector<int> result = strToIntVector(value, &ok);
//    if (ok) return result;
//  }
//  return defaultValue;
//}

/*!
  Returns the double the current tag carries as attribute with the name \a name. If the current tag
  doesn't contain such an attribute,\a defaultValue is returned.
  
  \see getAttributeDoubleVector
*/
double QXmlGet::getAttributeDouble(const QString &name, double defaultValue) const
{
  if (mCurrentElement.hasAttribute(name))
  {
    QString value = mCurrentElement.attribute(name);
    bool ok;
    double result = value.toDouble(&ok);
    if (ok) return result;
  }
  return defaultValue;
}

/*!
  Returns the double vector the current tag carries as attribute with the name \a name. If the current tag
  doesn't contain such an attribute,\a defaultValue is returned.
  
  \see getAttributeDouble
*/
//QVector<double> QXmlGet::getAttributeDoubleVector(const QString &name, const QVector<double> &defaultValue) const
//{
//  if (mCurrentElement.hasAttribute(name))
//  {
//    QString value = mCurrentElement.attribute(name);
//    bool ok;
//    QVector<double> result = strToDoubleVector(value, &ok);
//    if (ok) return result;
//  }
//  return defaultValue;
//}

/*!
  Returns the bool the current tag carries as attribute with the name \a name. If the current tag
  doesn't contain such an attribute,\a defaultValue is returned.
  
  by setting \a formats, you may specify which boolean formats this function accepts.
  
  \see getAttributeBoolVector
*/
bool QXmlGet::getAttributeBool(const QString &name, bool defaultValue, QXmlPutGet::BoolFormats formats) const
{
  if (mCurrentElement.hasAttribute(name))
  {
    QString value = mCurrentElement.attribute(name);
    bool ok;
    bool result = strToBool(value, &ok, formats);
    if (ok) return result;
  }
  return defaultValue;
}

/*!
  Returns the bool vector the current tag carries as attribute with the name \a name. If the current tag
  doesn't contain such an attribute,\a defaultValue is returned.
  
  by setting \a formats, you may specify which boolean formats this function accepts.
  
  \see getAttributeBool
*/
//QVector<bool> QXmlGet::getAttributeBoolVector(const QString &name, const QVector<bool> &defaultValue, QXmlPutGet::BoolFormats formats) const
//{
//  if (mCurrentElement.hasAttribute(name))
//  {
//    QString value = mCurrentElement.attribute(name);
//    bool ok;
//    QVector<bool> result = strToBoolVector(value, &ok, formats);
//    if (ok) return result;
//  }
//  return defaultValue;
//}

/*!
  Returns the QColor the current tag carries as attribute with the name \a name. If the current tag
  doesn't contain such an attribute,\a defaultValue is returned.
*/
//QColor QXmlGet::getAttributeColor(const QString &name, const QColor &defaultValue) const
//{
//  if (mCurrentElement.hasAttribute(name))
//  {
//    QString value = mCurrentElement.attribute(name);
//    bool ok;
//    QColor result = strToColor(value, &ok);
//    if (ok) return result;
//  }
//  return defaultValue;
//}

/*!
  Loads the XML content from the string \a str into this QXmlGet instance. Returns true on success.
  
  If the provided markup contains errors, this function returns false and outputs the error message
  in \a errorMessage and the location of the error in \a errorLine and \a errorColumn.
  
  \see QXmlPut::toString
*/
bool QXmlGet::fromString(const QString &str, QString *errorMessage, int *errorLine, int *errorColumn)
{
  QDomDocument newDoc;
  if (newDoc.setContent(str, errorMessage, errorLine, errorColumn))
  {
    *this = QXmlGet(newDoc);
    return true;
  }
  return false;
}

/*!
  Loads the XML content from the file \a fileName into this QXmlGet instance. Returns true on success.
  
  If the provided markup contains errors, this function returns false and outputs the error message
  in \a errorMessage and the location of the error in \a errorLine and \a errorColumn.
  
  If the provided file can't be opened (e.g. because it doesn't exist or the application doesn't
  have read permission), this function returns false and \a errorMessage, \a errorLine and \a
  errorColumn are left unchanged.
  
  \see QXmlPut::save
*/
bool QXmlGet::load(const QString &fileName, QString *errorMessage, int *errorLine, int *errorColumn)
{
  QFile file(fileName);
  if (file.open(QIODevice::ReadOnly))
  {
    QDomDocument newDoc;
    if (newDoc.setContent(&file, errorMessage, errorLine, errorColumn))
    {
      *this = QXmlGet(newDoc);
      return true;
    }
    file.close();
  } else
    qDebug() << FUNCNAME << "Couldn't read from file:" << fileName;
  
  return false;
}

/*!
  Returns a QXmlGet instance on the same Document and at the same position as this instance, but
  which is restricted to this hierarchy level (and the levels below). This means, the returned
  QXmlGet instance isn't allowed to \ref rise above the current hierarchy level.
  
  This is useful if you wish that subroutines can handle their own XML work without possibly
  interfering with the rest. By passing a restricted instance, it's guaranteed the subroutines
  don't accidentally write/read outside their designated XML element.
  
  If only the subroutine needs to write to/read from a specific element, consider using \ref
  descended.
  
  \b example:
  \code
  if (xmlGet.find("toptag")) // we might have better used findAndDescend() here.
  {
    xmlGet.descend();
    if (xmlGet.find("stringtag"))
      QString str = xmlGet.getString();
    readOtherContent(xmlGet.restricted()); // A subroutine that reads from the <toptag> level
    xmlGet.rise();
  }
  \endcode
  
  \see descended
*/
QXmlGet QXmlGet::restricted()
{
  QXmlGet result(*this);
  result.mBarrierNode = result.mCurrentParent;
  return result;
}

/*!
  Descends into the current element. Child elements can then be accessed normally via \ref find
  etc.
  
  Once the work in the lower hierarchy level is done, you can return to the previous position in
  the parent hierarchy level by calling \ref rise.
  
  If a subroutine needs to write to/read from a specific element, consider using \ref descended
  instead of a descend-rise-pair.
  
  \b example:
  \code
  if (xmlGet.find("firsttag")) // we might have better used findAndDescend() here.
  {
    xmlGet.descend(); // descend into the <firsttag> element, it is now the parent element.
    if (xmlGet.find("stringtag"))
      QString str = xmlGet.getString();
    xmlGet.rise(); // rise from the <firsttag> element. <firsttag> is now the current element again, and not the parent element anymore.
  }
  \endcode
  
  \see rise, descended, findAndDescend, findNextAndDescend
*/
void QXmlGet::descend()
{
  if (mCurrentParent == mCurrentElement)
    qDebug() << FUNCNAME << "Descended into own parent, possibly missing a rise()";
  mCurrentParent = mCurrentElement;
}

/*!
  Returns a QXmlGet instance that is descended into and restricted to the current element. Child
  elements can then be accessed with the returned instance normally via \ref find etc.
  
  Due to the restriction, the returned instance can't rise above its initial hierarchy level, i.e.
  into or above the hierarchy level of the instance this function is called on.
  
  When descending into elements like this, there is no need to call \ref rise (and thus no
  possibility to forget a \ref rise), because the current instance isn't influenced. Whatever
  descending/rising the subroutine does with the returned instance can't break the callers XML
  handling code.
  
  \b example:
  \code
  if (xmlGet.find("header"))
    readHeaderSubroutine(xmlGet.descended());
  if (xmlGet.find("body"))
    readBodySubroutine(xmlGet.descended());
  if (xmlGet.find("footer"))
    readFooterSubroutine(xmlGet.descended());
  \endcode
  
  \see restricted, descend
*/
QXmlGet QXmlGet::descended()
{
  QXmlGet result(*this);
  result.descend();
  result.mBarrierNode = result.mCurrentParent;
  return result;
}

/*!
  Rises to the previous position in the parent hierarchy level. This finishes the work in a lower
  hierarchy level that was started with \ref descend earlier.
  
  If a subroutine needs to write to/read from a specific element, consider using \ref descended
  instead of a descend-rise-pair.
  
  If this instance is restricted (see \ref descended and \ref restricted) and is already inside its
  highest allowed hierarchy level, a further attempt to \ref rise will return false without
  changing the current hierarchy level, and cause a corresponding qDebug output.
  
  \b example:
  \code
  if (xmlGet.find("firsttag")) // we might have better used findAndDescend() here.
  {
    xmlGet.descend(); // descend into the <firsttag> element, it is now the parent element.
    if (xmlGet.find("stringtag"))
      QString str = xmlGet.getString();
    xmlGet.rise(); // rise from the <firsttag> element. <firsttag> is now the current element again, and not the parent element anymore.
  }
  \endcode
  
  \see descend
*/
bool QXmlGet::rise()
{
  if (mCurrentParent == mBarrierNode)
  {
    qDebug() << FUNCNAME << "attept to rise beyond allowed node";
    return false;
  }
  
  QDomNode node = mCurrentParent.parentNode();
  if (!node.isNull())
  {
    if (node.isElement())
    {
      QDomElement newParent = node.toElement();
      mCurrentElement = mCurrentParent;
      mCurrentParent = newParent;
      if (mCurrentElement.isNull())
        mCurrentElement = newParent;
      return true;
    } else
      qDebug() << FUNCNAME << "Attempt to rise into non-element node";
  } else
    qDebug() << FUNCNAME << "Attempt to rise above document node";
  return false;
}

/*!
  Changes the current parent element to \a parentElement. It must be in the current QDomDocument
  already.
  
  If this instance is restricted (see \ref descended and \ref restricted) and \a parentElement is
  not inside the allowed hierarchy, this function will return false without changing the current
  position, and cause a corresponding qDebug output.
  
  You probably won't use this function very often, since normal, linear QXmlPutGet navigation
  should be done with \ref find, \ref descend, \ref rise, \ref descended, etc. However, it's useful
  for nonlinear XML navigation, where frequent jumps between different locations in the XML
  hierarchy need to be done.
  
  \see element
*/
bool QXmlGet::goTo(QDomElement parentElement)
{
  // check whether this instance is allowed to jump to the element:
  QDomElement el = parentElement;
  while (!el.isNull() && el != mBarrierNode)
    el = el.parentNode().toElement();
  if (el.isNull())
  {
    qDebug() << FUNCNAME << "attempt to jump outside of allowed tree";
    return false;
  }
         
  // perform jump:
  if (parentElement.ownerDocument() == mDocument)
  {
    mCurrentParent = parentElement;
    mCurrentElement = parentElement.lastChildElement();
    if (mCurrentElement.isNull())
      mCurrentElement = parentElement;
    return true;
  } else
    qDebug() << FUNCNAME << "Attempt to go to element not in document";
  return false;
}

bool QXmlGet::strToBool(QString value, bool *ok, QXmlPutGet::BoolFormats formats)
{
  value = value.toLower();
  if (formats.testFlag(QXmlPutGet::bfYesNo))
  {
    if (value == "yes")
    {
      if (ok) *ok = true;
      return true;
    } else if (value == "no")
    {
      if (ok) *ok = true;
      return false;
    }
  }
  if (formats.testFlag(QXmlPutGet::bfTrueFalse))
  {
    if (value == "true")
    {
      if (ok) *ok = true;
      return true;
    } else if (value == "false")
    {
      if (ok) *ok = true;
      return false;
    }
  }
  if (formats.testFlag(QXmlPutGet::bf10))
  {
    if (value == "1")
    {
      if (ok) *ok = true;
      return true;
    } else if (value == "0")
    {
      if (ok) *ok = true;
      return false;
    }
  }
  if (ok) *ok = false;
  return false;
}

//QColor QXmlGet::strToColor(QString value, bool *ok)
//{
//  int alpha = 255;
//  bool subOk;
//  if (value.startsWith("#"))
//  {
//    // separate off alpha value, because QColor(QString) constructor doesn't parse alpha
//    if (value.length() == 1+4) // #RGBA
//    {
//      alpha = value.right(1).toInt(&subOk, 16)/15.0*255;
//      if (!subOk)
//      {
//        if (ok) *ok = false;
//        return QColor();
//      }
//      value.chop(1);
//    } else if (value.length() == 1+8) // #RRGGBBAA
//    {
//      alpha = value.right(2).toInt(&subOk, 16);
//      if (!subOk)
//      {
//        if (ok) *ok = false;
//        return QColor();
//      }
//      value.chop(2);
//    } else if (value.length() == 1+16) // #RRRGGGBBBAAA
//    {
//      alpha = value.right(3).toInt(&subOk, 16)/4095.0*255;
//      if (!subOk)
//      {
//        if (ok) *ok = false;
//        return QColor();
//      }
//      value.chop(3);
//    }
//  }
//  QColor result(value);
//  if (result.isValid())
//  {
//    if (alpha < 255)
//      result.setAlpha(alpha);
//    if (ok) *ok = true;
//    return result;
//  }
//  if (ok) *ok = false;
//  return QColor();
//}

//QVector<int> QXmlGet::strToIntVector(QString value, bool *ok)
//{
//  QStringList valueList = value.split(";");
//  QVector<int> result;
//  bool subOk;
//  for (int i=0; i<valueList.size(); ++i)
//  {
//    result.append(valueList.at(i).toInt(&subOk));
//    if (!subOk)
//    {
//      if (ok) *ok = false;
//      return QVector<int>();
//    }
//  }
//  if (ok) *ok = true;
//  return result;
//}

//QVector<double> QXmlGet::strToDoubleVector(QString value, bool *ok)
//{
//  QStringList valueList = value.split(";");
//  QVector<double> result;
//  bool subOk;
//  for (int i=0; i<valueList.size(); ++i)
//  {
//    result.append(valueList.at(i).toDouble(&subOk));
//    if (!subOk)
//    {
//      if (ok) *ok = false;
//      return QVector<double>();
//    }
//  }
//  if (ok) *ok = true;
//  return result;
//}

//QVector<bool> QXmlGet::strToBoolVector(QString value, bool *ok, QXmlPutGet::BoolFormats formats)
//{
//  QStringList valueList = value.split(";");
//  QVector<bool> result;
//  bool subOk;
//  for (int i=0; i<valueList.size(); ++i)
//  {
//    result.append(strToBool(valueList.at(i), &subOk, formats));
//    if (!subOk)
//    {
//      if (ok) *ok = false;
//      return QVector<bool>();
//    }
//  }
//  if (ok) *ok = true;
//  return result;
//}




