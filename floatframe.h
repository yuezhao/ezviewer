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

#ifndef FLOATFRAME_H
#define FLOATFRAME_H

#include <QFrame>
#include <QList>


class FloatFrame : public QFrame
{
    Q_OBJECT
public:
    explicit FloatFrame(QWidget *parent = 0);

signals:
    void showContextMenu(const QPoint &pos);
    void mouseDoubleClick();
    void siteChange(const QPoint &change);

public slots:
    void addWidget(QWidget *w);
    void cancelWidget(QWidget *w);

protected slots:
    void contextMenuEvent ( QContextMenuEvent * event );
    void enterEvent( QEvent * event );
    void leaveEvent( QEvent * event );
    void mouseDoubleClickEvent ( QMouseEvent * event );
    void mousePressEvent ( QMouseEvent * event );
    void mouseMoveEvent ( QMouseEvent * event );
    void mouseReleaseEvent ( QMouseEvent * event );

    void myTimerEvent();

private:
    QTimer *hideTimer;
    int hideInterval;

    QList<QWidget *> list;

    QPoint startPos;
};

#endif // FLOATFRAME_H
