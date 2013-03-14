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

const QString Version = "3.0 beta";
const QString BuildTime = "2012/2-2012/2";
const QString WeiboSite = "www.weibo.com/huangezhao";
const QString ProjectSite = "code.google.com/p/ezviewer/";
const QString AuthorBlog = "yuezhaoblog.blogspot.com";

static inline QString ProjectName()
{ return QObject::tr("EzViewer"); }

static inline QString ProjectAuthor()
{ return QObject::tr("huangezhao"); }

static inline QString AboutInfo()
{
    return QObject::tr("<p align='center'>%1 - A Simple Image Viewer</p>" \
                       "<p>Version: &nbsp;&nbsp; v%2"
                       "<br>Build Time: &nbsp;&nbsp; %3"  \
                       "<br>CopyRight &#169; 2012 by %4</p>" \
                       "<p>Contact Author: &nbsp;&nbsp; <a href='http://%5'>%5</a>" \
                       "<br>Project Home: &nbsp;&nbsp; <a href='http://%6'>%6</a>" \
                       "<br>Author's Blog: &nbsp;&nbsp; <a href='http://%7'>%7</a></p>")
            .arg(ProjectName()).arg(Version).arg(BuildTime)
            .arg(ProjectAuthor()).arg(WeiboSite).arg(ProjectSite)
            .arg(AuthorBlog);
}
}

#endif // GLOBAL_H
