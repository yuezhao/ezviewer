/****************************************************************************
 * EZ Viewer
 * Copyright (C) 2013 huangezhao. CHINA.
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

#include "actionmanager.h"


QMap<QString, Action*> ActionManager::actionMap;
QMap<QString, QString> ActionManager::shortcutMap;


void ActionManager::unregisterAllFunction()
{
    QMapIterator<QString, Action*> i(actionMap);
    while (i.hasNext()) {
        i.next();
        delete i.value();
    }
}

bool ActionManager::bindShortcut(const QString &keySequence,
                                 const QString &actionScript)
{
    if (shortcutMap.contains(keySequence))
        return false;
    shortcutMap.insert(keySequence, actionScript);
    /////////////////////
    return true;
}

void ActionManager::bindShortcut(const QStringList &keySequences, const QString &actionScript)
{
    foreach (QString key, keySequences)
        bindShortcut(key, actionScript);
}

bool ActionManager::unbindShortcut(const QString &keySequence)
{
    if (shortcutMap.contains(keySequence)) {
        shortcutMap.remove(keySequence);
        //////////////////////
        return true;
    }
    return false;
}

void ActionManager::unbindShortcut(const QStringList &keySequences)
{
    foreach (QString key, keySequences)
        unbindShortcut(key);
}

QStringList ActionManager::getAllActions(QStringList *actionScript)
{
    QStringList descriptions;
    if (actionScript)
        actionScript->clear();
    QMap<QString, Action*>::const_iterator it = actionMap.constBegin();
    while (it != actionMap.constEnd()) {
        descriptions.append(it.value()->description());
        if (actionScript)
            actionScript->append(it.key());
        ++it;
    }
    return descriptions;
}

QStringList ActionManager::getBindShortcuts(const QString &actionScript)
{
    QStringList list;
    QMap<QString, QString>::const_iterator it = shortcutMap.constBegin();
    while (it != shortcutMap.constEnd()) {
        if (it.value() == actionScript)
            list.append(it.key());
        ++it;
    }
    return list;
}

bool ActionManager::run(const QString &keySequence)
{
    if (shortcutMap.contains(keySequence)) {
        QString script = shortcutMap.value(keySequence);
        if (actionMap.contains(script))
            return actionMap.value(script)->run();
    }
    return false;
}

