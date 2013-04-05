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

#ifndef FILEASSOC_H
#define FILEASSOC_H

#include <QString>

namespace FileAssoc{

bool isSupportAssociation();
bool checkAssociation(const QString &extension);
bool setAssociation(const QString &extension,
                    const QString &typeDescription = QString::null,
                    const QString &friendlyAppName = QString::null);
bool clearAssociation(const QString &extension);
// Must call this after change association setting, like set or clear association.
void refreshExplorer();
}

#endif // FILEASSOC_H
