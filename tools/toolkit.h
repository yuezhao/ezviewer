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

#ifndef TOOLKIT_H
#define TOOLKIT_H

#include <QString>
#include <QFileInfo>

#define SafeDelete(arg) \
{ \
if(arg){ \
        ::delete arg; \
        arg = NULL; \
    } \
}


namespace ToolKit{

QString fileSize2Str(qint64 size);

inline QString filename(const QString &filePath)
{ return QFileInfo(filePath).fileName(); }

// return a stringlist that only include file exist.
QStringList getFilesExist(const QStringList &list);

uint getFileHash(const QString &filePath);


template<typename T>
void swap(T &a, T &b);

template<typename T>
T gcd(T a, T b); // get the greatest common divisor 求最大公约数
}


namespace ToolKit{
template<typename T>
inline void swap(T &a, T &b)
{
    T temp = b;
    b = a;
    a = temp;
}

template<typename T>
T gcd(T a, T b)
{
    if (a == 0) return b;
    if (b == 0) return a;

    if(b < a) swap(a, b);

    T tmp;
    while(a != 0)
    {
        tmp = b % a;
        b = a;
        a = tmp;
    }
    return b;
}
}

#endif // TOOLKIT_H
