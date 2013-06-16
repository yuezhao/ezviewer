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

#ifndef FLOATFRAME_H
#define FLOATFRAME_H

#include <QWidget>
#include <QList>
#include <QBasicTimer>
#include <QTime>


class FloatFrame : public QWidget
{
    Q_OBJECT
public:
    explicit FloatFrame(QWidget *parent = 0);

    void setFillBackground(bool enabled);

signals:
    void mouseClicked();

public slots:
    void addWidget(QWidget *w);
    void cancelWidget(QWidget *w);
    void setHideInterval(int msec) { hideInterval = msec; }
    void setExpireInterval(int msec) { expireInterval = msec; }
    void set_enabled(bool enabled);

protected slots:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void timerEvent(QTimerEvent *e);

    void hideAll();
    void showAll();

private:
    QBasicTimer hideTimer, expireTimer;
    int hideInterval;
    int expireInterval;

    QTime timeStamp;
    QPoint pressPos;

    bool fillBackground;
    bool _enabled;
    QList<QWidget *> list;
};

#endif // FLOATFRAME_H
