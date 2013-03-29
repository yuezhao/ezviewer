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

#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <QMap>

#include "action.h"


class ActionManager
{
public:
    template <typename PT, typename T, typename ReturnType>
    static void registerFunction(const QString &description, PT *obj,
                          ReturnType (T::*function)(), const QString &script);

    static void unregisterAllFunction();

    static bool bindShortcut(const QString &keySequence, const QString &actionScript);

    // return: action script text.
    static QString getMatchAction(const QString &keySequence);

    static bool run(const QString &keySequence);

private:
    static QMap<QString, Action*> actionMap;   // actionScript - action
    static QMap<QString, QString> shortcutMap; // keySequence  - actionScript
};



template <typename PT, typename T, typename ReturnType>
inline void ActionManager::registerFunction(const QString &description, PT *obj,
                      ReturnType (T::*function)(), const QString &script)
{
    actionMap.insert(script,
                     new ActionImpl<T, ReturnType>(description, obj, function) );
}

inline QString ActionManager::getMatchAction(const QString &keySequence)
{
    return shortcutMap.contains(keySequence) ? shortcutMap.value(keySequence)
                                             : QString::null;
}

#endif // ACTIONMANAGER_H
