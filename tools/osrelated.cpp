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
#include <QThread>

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

bool preReadingSuggested()
{
    return QThread::idealThreadCount() > 1;
}

void moveFile2Trash(const QString &filePath)
{
#ifdef Q_WS_WIN

    SHFILEOPSTRUCT FileOp;
    FileOp.hwnd = 0;
    FileOp.wFunc = FO_DELETE;

    TCHAR buf[_MAX_PATH + 1];
    _tcscpy(buf, QStringToTCHAR(QDir::toNativeSeparators(filePath)));  // Copy file path, ends with '\0'.
    buf[_tcslen(buf)+1] = 0;  //! must end with 2 '\0'.

    FileOp.pFrom = buf; // source file path
    FileOp.pTo = NULL; // dist file path
    FileOp.fFlags |= FOF_ALLOWUNDO; // delete file to trash
//    FileOp.fFlags &= ~FOF_ALLOWUNDO;    // directly delete, don't move to trash

//    if(!needAsk)
        FileOp.fFlags |= FOF_NOCONFIRMATION;    //! delete without notification dialog

    // IF file is being used, the return value is 32 or 1223.
    if(SHFileOperation(&FileOp)) // Actually delete file here
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
