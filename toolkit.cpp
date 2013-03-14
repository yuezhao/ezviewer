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

#include "toolkit.h"
#include "config.h"

#include <QByteArray>


namespace ToolKit{

QString fileSize2Str(qint64 size)
{
    const qreal OneK = 1024;
    const qreal OneM = 1024 * 1024;
    const qreal OneG = 1024 * 1024 * 1024;

    if(size <= OneK)
        return QString("%1 B").arg(size);
    else if ( size <= OneM )
        return QString("%1 KB").arg(size / OneK, 0, 'f', Config::FileSizePrecision);
    else if ( size <= OneG )
        return QString("%1 MB").arg(size / OneM, 0, 'f', Config::FileSizePrecision);
    else
        return QString("%1 GB").arg(size / OneG, 0, 'f', Config::FileSizePrecision);
}

QStringList getFilesExist(const QStringList &list)
{
    QStringList fileList;
    QFileInfo fileInfo;
    QString fileName;
    for (int size = list.size(), i = 0; i < size; ++i) {
        fileName = list.at(i);
        fileInfo.setFile(fileName);
        if(fileInfo.isFile())//no directory
            fileList.append(fileName);  //! fileInfo.absolutePath()??
    }
    return fileList;
}

}
