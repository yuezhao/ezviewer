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

#include <QDebug>
#include <QtGui>

FloatFrame::FloatFrame(QWidget *parent) :
    QFrame(parent)
{
//    setFrameStyle(StyledPanel | Plain);
//        setFrameStyle(QFrame::Panel);//NoFrame
    startPos = QPoint(-1, -1);

    hideInterval = 1000;
    hideTimer = new QTimer(this);
    hideTimer->setInterval(hideInterval);
    connect(hideTimer, SIGNAL(timeout()), SLOT(myTimerEvent()));

    QPalette pal(palette());
    pal.setBrush(QPalette::Window, QBrush(QColor(0, 0, 0, 50)));
    setPalette(pal);

    if(underMouse())
        setAutoFillBackground(true);
}

void FloatFrame::addWidget(QWidget *w)
{
    if(w == NULL) return;
    if(list.contains(w)) return;

    list.append(w);
//    connect(hideTimer, SIGNAL(timeout()), w, SLOT(hide()));

    if(!underMouse())
        w->hide();
}

void FloatFrame::cancelWidget(QWidget *w)
{
    if(w == NULL) return;
    if(!list.contains(w)) return;

    list.removeOne(w);
//    disconnect(w);
}

void FloatFrame::enterEvent( QEvent * event )
{
    setAutoFillBackground(true);
    hideTimer->stop();
    for (int i = 0; i < list.size(); ++i)
        list.at(i)->show();
    emit mouseEnter();
}

void FloatFrame::leaveEvent( QEvent * event )
{
    hideTimer->start();
    emit mouseLeave();
}

void FloatFrame::myTimerEvent()
{
    setAutoFillBackground(false);
    for (int i = 0; i < list.size(); ++i)
        list.at(i)->hide();
}

void FloatFrame::contextMenuEvent( QContextMenuEvent * event )
{
    emit showContextMenu(event->globalPos());
}


void FloatFrame::mouseDoubleClickEvent ( QMouseEvent * event )
{
    if(event->button() & Qt::LeftButton)
        emit mouseDoubleClick();
}

void FloatFrame::mousePressEvent ( QMouseEvent * event )
{
    if(event->button() & Qt::LeftButton)
        startPos = event->globalPos();
}

void FloatFrame::mouseMoveEvent ( QMouseEvent * event )
{
    //! For mouse move events, this is all buttons that are pressed down.
    if(event->buttons() & Qt::LeftButton){
        if(startPos.x() >= 0)
            emit siteChange(event->globalPos() - startPos);
        startPos = event->globalPos();    //
    }
}

void FloatFrame::mouseReleaseEvent ( QMouseEvent * event )
{
    if(event->button() & Qt::LeftButton){
//        emit siteChange(event->globalPos() - startPos);
        startPos = QPoint(-1, -1);
    }
}
