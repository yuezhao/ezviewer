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
}

#endif // FILEASSOC_H
