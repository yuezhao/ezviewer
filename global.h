/****************************************************************************
 * EZ Viewer
 * Copyright (C) 2012 huangezhao. CHINA.
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

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QObject>


namespace Global {

const QString Version = "3.0";
const QString CopyRightYear = "2012/2-2012/3";
const QString BuildTime = __TIME__ ", " __DATE__;
const QString WeiboSite = "www.weibo.com/huangezhao";
const QString ProjectSite = "code.google.com/p/ezviewer";
const QString AuthorBlog = "yuezhaoblog.blogspot.com";
const QString AuthorEmail = "huangezhao@gmail.com";


static inline QString ProjectName()
{ return QObject::tr("EzViewer"); }

static inline QString ProjectAuthor()
{ return QObject::tr("huangezhao"); }

static inline QString AboutInfo()
{
    return QObject::tr("<p align='center'>%1 - A Simple Image Viewer</p>" \
                       "<p>Version: v%2"
                       "<br>Build Time: %3"  \
                       "<br>CopyRight &#169; %4 by %5</p>" \
                       "<p>Contact Author: <a href='http://%6'>%6</a>" \
                       "<br>Project Home: <a href='http://%7'>%7</a>" \
                       "<br>Author's Blog: <a href='http://%8'>%8</a>" \
                       "<br>Send Email: <a href='mailto:%9'>%9</a></p>" )
            .arg(ProjectName()).arg(Version).arg(BuildTime).arg(CopyRightYear)
            .arg(ProjectAuthor()).arg(WeiboSite).arg(ProjectSite)
            .arg(AuthorBlog).arg(AuthorEmail);
}

static inline QString LoadFileErrorInfo()
{
    return QObject::tr("Cannot load picture:'%3'" \
                       "\nYou can help to improve %1 by email the picture to %2")
            .arg(ProjectName()).arg(AuthorEmail);
}

}

#endif // GLOBAL_H
