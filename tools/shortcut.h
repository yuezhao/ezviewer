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

#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QKeySequence>
#include <QString>


class Shortcut
{
public:
    Shortcut(QString description) : text(description) {}

    QString toString() const { return text; }

    virtual void run() = 0;

protected:
    QString text;
};


template <typename T, typename ReturnType = void>
class ShortcutImpl : public Shortcut
{
public:
    typedef ReturnType (T::*FuncType)();

    ShortcutImpl(QString description, T *obj, FuncType f)
        : Shortcut(description), object(obj), function(f) {}

    virtual void run() {
        if (object)
            (object->*function)();
    }
private:
    T *object;
    FuncType function;
};


#endif // SHORTCUT_H
