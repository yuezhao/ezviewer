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
#include <QStringList>

#include "action.h"


class ActionManager
{
public:
    template <typename PT, typename T, typename ReturnType>
    static void registerFunction(const QString &description, PT *obj,
                          ReturnType (T::*function)(), const QString &tag);

    template <typename PT, typename T, typename ReturnType, typename ArgumentType>
    static void registerFunction(const QString &description,
                                 PT *obj,
                                 ReturnType (T::*function)(ArgumentType),
                                 const QString &tag,
                                 ArgumentType arg);

    template <typename PT, typename T, typename ReturnType, typename ArgumentType>
    static void registerFunction(const QString &description,
                                 PT *obj,
                                 ReturnType (T::*function)(const ArgumentType &),
                                 const QString &tag,
                                 ArgumentType arg);

    template <typename PT, typename T, typename ReturnType, typename ArgumentType>
    static void registerFunction(const QString &description,
                                 PT *obj,
                                 ReturnType (T::*function)(ArgumentType, ArgumentType),
                                 const QString &tag,
                                 ArgumentType arg, ArgumentType);

    static bool bindShortcut(const QString &keySequence, const QString &actionTag);
    static void bindShortcut(const QStringList &keySequences, const QString &actionTag);
    static bool unbindShortcut(const QString &keySequence);
    static void unbindShortcut(const QStringList &keySequences);

    static void unbindAllShortcut() { shortcutMap.clear(); }

    // return: action description list, and if actionScript is not NULL, the action tag list will be set.
    static QStringList getAllActions(QStringList *actionTag = NULL);
    // return: action tag text that this key sequence bind with.
    static QString getMatchAction(const QString &keySequence);
    // return: all key sequence list that bind with the action tag.
    static QStringList getBindShortcuts(const QString &actionTag);
    // return: all key sequence and action tag pair.
    static QMap<QString, QString> getAllShortcut() { return shortcutMap; }

    // run the function that bind with the key sequence.
    static bool run(const QString &keySequence);

private:
    ~ActionManager();

    static QMap<QString, Action*> actionMap;   // actionTag - action
    static QMap<QString, QString> shortcutMap; // keySequence  - actionTag

    static ActionManager ensure_destructor_invoke_while_quit;
};



template <typename PT, typename T, typename ReturnType>
inline void ActionManager::registerFunction(const QString &description, PT *obj,
                      ReturnType (T::*function)(), const QString &tag)
{
    actionMap.insert(tag,
                     new ActionImpl<T, ReturnType>(description, obj, function) );
}

template <typename PT, typename T, typename ReturnType, typename ArgumentType>
inline void ActionManager::registerFunction(const QString &description,
                                            PT *obj,
                                            ReturnType (T::*function)(ArgumentType),
                                            const QString &tag,
                                            ArgumentType arg)
{
    actionMap.insert(tag,
                     new ActionImplWithArgument<T, ReturnType, ArgumentType>
                     (description, obj, function, arg) );
}

template <typename PT, typename T, typename ReturnType, typename ArgumentType>
inline void ActionManager::registerFunction(const QString &description,
                             PT *obj,
                             ReturnType (T::*function)(const ArgumentType &),
                             const QString &tag,
                             ArgumentType arg)
{
    actionMap.insert(tag,
                     new ActionImplWithArgument<T, ReturnType, const ArgumentType &>
                     (description, obj, function, arg) );
}

template <typename PT, typename T, typename ReturnType, typename ArgumentType>
inline void ActionManager::registerFunction(const QString &description,
                                            PT *obj,
                                            ReturnType (T::*function)(ArgumentType, ArgumentType),
                                            const QString &tag,
                                            ArgumentType arg1, ArgumentType arg2)
{
    actionMap.insert(tag,
                     new ActionImplWithArgument2<T, ReturnType, ArgumentType>
                     (description, obj, function, arg1, arg2) );
}

inline QString ActionManager::getMatchAction(const QString &keySequence)
{
    return shortcutMap.contains(keySequence) ? shortcutMap.value(keySequence)
                                             : QString::null;
}


#endif // ACTIONMANAGER_H
