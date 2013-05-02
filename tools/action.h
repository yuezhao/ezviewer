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

#ifndef ACTION_H
#define ACTION_H

#include <QKeySequence>
#include <QString>


class Action
{
public:
    Action(const QString &description) : mDescription(description) {}

    QString description() const { return mDescription; }

    virtual bool run() = 0;

protected:
    QString mDescription;   // for use reading
};


template <typename T, typename ReturnType = void>
class ActionImpl : public Action
{
public:
    typedef ReturnType (T::*FuncType)();

    ActionImpl(const QString &description, T *obj, FuncType f)
        : Action(description), object(obj), function(f)
    {}

    virtual bool run() {
        if (object && function) {
            (object->*function)();
            return true;
        }
        return false;
    }
private:
    T *object;
    FuncType function;
};


template <typename T, typename ReturnType = void, typename ArgumentType = int>
class ActionImplWithArgument : public Action
{
public:
    typedef ReturnType (T::*FuncType)(ArgumentType);

    ActionImplWithArgument(const QString &description,
                           T *obj, FuncType f, ArgumentType arg)
        : Action(description), object(obj), function(f), param(arg)
    {}

    virtual bool run() {
        if (object && function) {
            (object->*function)(param);
            return true;
        }
        return false;
    }
private:
    T *object;
    FuncType function;
    ArgumentType param;
};


template <typename T, typename ReturnType = void, typename ArgumentType = int>
class ActionImplWithArgument2 : public Action
{
public:
    typedef ReturnType (T::*FuncType)(ArgumentType, ArgumentType);

    ActionImplWithArgument2(const QString &description,
                           T *obj, FuncType f,
                           ArgumentType arg1, ArgumentType arg2)
        : Action(description), object(obj), function(f), param1(arg1), param2(arg2)
    {}

    virtual bool run() {
        if (object && function) {
            (object->*function)(param1, param2);
            return true;
        }
        return false;
    }
private:
    T *object;
    FuncType function;
    ArgumentType param1;
    ArgumentType param2;
};

#endif // ACTION_H
