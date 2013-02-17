/****************************************************************************
 * EZ Viewer
 * Copyright (C) 2012 huangezhao. CHINA.
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

#include "floatframe.h"

#include <QTimerEvent>
#include <QContextMenuEvent>
#include <QMouseEvent>


FloatFrame::FloatFrame(QWidget *parent) :
    QWidget(parent), fillBackground(true), _enabled(true)
{
    hideInterval = 1000;
    expireInterval = 300;

    QPalette pal(palette());
    pal.setBrush(QPalette::Window, QBrush(QColor(0, 0, 0, 50)));
    setPalette(pal);

    setFillBackground(true);
}

void FloatFrame::addWidget(QWidget *w)
{
    if(w == NULL || list.contains(w)) return;

    list.append(w);

    if(!underMouse())
        w->hide();
}

void FloatFrame::cancelWidget(QWidget *w)
{
    if(w == NULL || !list.contains(w)) return;
    list.removeOne(w);
}

void FloatFrame::setFillBackground(bool enabled)
{
    fillBackground = enabled;
    if(underMouse())  ///
        setAutoFillBackground(fillBackground);
}

void FloatFrame::set_enabled(bool enabled)
{
    _enabled = enabled;
    if(!_enabled)
        hideAll();
}

void FloatFrame::hideAll()
{
    setAutoFillBackground(false);
    for (int i = 0; i < list.size(); ++i)
        list.at(i)->hide();
}

void FloatFrame::showAll()
{
    if(fillBackground)
        setAutoFillBackground(true);
    for (int i = 0; i < list.size(); ++i)
        list.at(i)->show();
}

void FloatFrame::enterEvent(QEvent * /*event*/)
{
    if(!_enabled) return;

    hideTimer.stop();
    if (!expireTimer.isActive())
        expireTimer.start(expireInterval, this);
}

void FloatFrame::leaveEvent(QEvent * /*event*/)
{
    expireTimer.stop();
    if (!hideTimer.isActive())
        hideTimer.start(hideInterval, this);
}

void FloatFrame::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == hideTimer.timerId())
        hideAll();
    else if (e->timerId() == expireTimer.timerId())
        showAll();

    hideTimer.stop();
    expireTimer.stop();
}

void FloatFrame::contextMenuEvent(QContextMenuEvent *event)
{
    emit showContextMenu(event->globalPos());
}

void FloatFrame::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() & Qt::LeftButton)
        emit mouseDoubleClick();
}

void FloatFrame::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() & Qt::LeftButton)
        emit mouseClicked();
}
