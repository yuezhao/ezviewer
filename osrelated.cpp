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

#include "osrelated.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

#ifdef Q_WS_WIN
#define _WIN32_WINNT  0x0500    //! for memory size
#include <windows.h>
#endif // Q_WS_WIN

#ifdef Q_WS_WIN
#include <windows.h>
#include <ShellAPI.h>
#ifdef UNICODE
#   define _UNICODE
#endif // UNICODE
#include <tchar.h>  // need _UNICODE
#ifdef UNICODE
#   define QStringToTCHAR(x) (wchar_t*) x.utf16()   // need UNICODE
#else
#   define QStringToTCHAR(x) x.local8Bit().constData()
#endif // UNICODE
#endif // Q_WS_WIN


namespace OSRelated
{

int cacheSizeSuggested()
{
    int totalMemory = 0;

#ifdef Q_WS_WIN
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    GlobalMemoryStatusEx (&statex);
    totalMemory = statex.ullTotalPhys / (1024*1024); // convert bytes to MB
#endif // Q_WS_WIN

    qDebug("memory is %d", totalMemory);
    if(totalMemory < 1000) // memory less than 1G
        return 0;
    else if(totalMemory < 1500)
        return 1;
    else
        return 2;
}

void moveFile2Trash(const QString &filePath)
{
#ifdef Q_WS_WIN

    SHFILEOPSTRUCT FileOp;//定义SHFILEOPSTRUCT结构对象;
    FileOp.hwnd = 0;
    FileOp.wFunc = FO_DELETE; //执行文件删除操作;

    TCHAR buf[_MAX_PATH + 1];
    _tcscpy(buf, QStringToTCHAR(QDir::toNativeSeparators(filePath)));  // 复制路径,end with '\0'.
    buf[_tcslen(buf)+1] = 0;  //! must end with 2 '\0'.

    FileOp.pFrom = buf; // source file path
    FileOp.pTo = NULL; //目标文件路径
    FileOp.fFlags |= FOF_ALLOWUNDO;//此标志使删除文件备份到Windows回收站
//    FileOp.fFlags &= ~FOF_ALLOWUNDO;    //直接删除，不进入回收站

//    if(!needAsk)
        FileOp.fFlags |= FOF_NOCONFIRMATION;    //! 直接删除，不进行确认

    // 如果文件被占用，返回值是32或1223
    if(SHFileOperation(&FileOp)) //这里开始删除文件
        QMessageBox::warning(0, QObject::tr("Delete Failed"),
                             QObject::tr("Delete file '%1' failed!")
                             .arg(QFileInfo(filePath).fileName()));

#else

    if(!QFile::remove(filePath))
        QMessageBox::warning(0, QObject::tr("Delete Failed"),
                             QObject::tr("Delete file '%1' failed!")
                             .arg(QFileInfo(filePath).fileName()));

#endif // Q_WS_WIN
}

}
