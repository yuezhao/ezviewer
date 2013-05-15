/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "velocitytracker.h"

#include <QtCore>
#include <QtGui>


class VelocityTrackerPrivate
{
public:
    typedef enum {
        Steady,
        Pressed,
        ManualScroll,
        AutoScroll,
        Stop
    } State;

    State state;
    int threshold;
    QPoint pressPos;
    QPoint delta;
    QPoint speed;
    QTime timeStamp;
    int interval;
};

VelocityTracker::VelocityTracker(QObject *parent)
    : QObject(parent)
{
    d = new VelocityTrackerPrivate;
    d->state = VelocityTrackerPrivate::Steady;
    d->threshold = 10;
    d->timeStamp = QTime::currentTime();
    d->interval = 20;
}

VelocityTracker::~VelocityTracker()
{
    delete d;
}

void VelocityTracker::setTickInterval(int interval)
{
    d->interval = interval;
}

void VelocityTracker::setThreshold(int th)
{
    if (th >= 0)
        d->threshold = th;
}

int VelocityTracker::threshold() const
{
    return d->threshold;
}

static QPoint deaccelerate(const QPoint &speed, int a = 1, int max = 64)
{
    int x = qBound(-max, speed.x(), max);
    int y = qBound(-max, speed.y(), max);
    x = (x == 0) ? x : (x > 0) ? qMax(0, x - a) : qMin(0, x + a);
    y = (y == 0) ? y : (y > 0) ? qMax(0, y - a) : qMin(0, y + a);
    return QPoint(x, y);
}

void VelocityTracker::handleMousePress(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    switch (d->state) {

    case VelocityTrackerPrivate::Steady:
        d->state = VelocityTrackerPrivate::Pressed;
        d->pressPos = event->pos();
        break;

    case VelocityTrackerPrivate::AutoScroll:
        d->state = VelocityTrackerPrivate::Stop;
        d->speed = QPoint(0, 0);
        d->pressPos = event->pos();
        stop();
        break;

    default:
        break;
    }
}

void VelocityTracker::handleMouseRelease(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    QPoint delta;

    switch (d->state) {

    case VelocityTrackerPrivate::Pressed:
        d->state = VelocityTrackerPrivate::Steady;
        break;

    case VelocityTrackerPrivate::ManualScroll:
        delta = event->pos() - d->pressPos;
        emit changedDelta(delta);
        if (d->timeStamp.elapsed() > 100) {
            d->timeStamp = QTime::currentTime();
            d->speed = delta - d->delta;
            d->delta = delta;
        }
        d->pressPos = event->pos();
        if (d->speed == QPoint(0, 0)) {
            d->state = VelocityTrackerPrivate::Steady;
        } else {
//            d->speed /= 4;
            d->state = VelocityTrackerPrivate::AutoScroll;
            start(d->interval);
        }
        break;

    case VelocityTrackerPrivate::Stop:
        d->state = VelocityTrackerPrivate::Steady;
        break;

    default:
        break;
    }
}

void VelocityTracker::handleMouseMove(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;

    QPoint delta;

    switch (d->state) {

    case VelocityTrackerPrivate::Pressed:
    case VelocityTrackerPrivate::Stop:
        delta = event->pos() - d->pressPos;
        if (delta.x() > d->threshold || delta.x() < -d->threshold ||
                delta.y() > d->threshold || delta.y() < -d->threshold) {
            d->timeStamp = QTime::currentTime();
            d->state = VelocityTrackerPrivate::ManualScroll;
            d->delta = QPoint(0, 0);
            d->pressPos = event->pos();
            emit changedDelta(delta);
        }
        break;

    case VelocityTrackerPrivate::ManualScroll:
        delta = event->pos() - d->pressPos;
        emit changedDelta(delta);
        d->pressPos = event->pos();
        if (d->timeStamp.elapsed() > 100) {
            d->timeStamp = QTime::currentTime();
            d->speed = delta - d->delta;
            d->delta = delta;
        }
        break;

    default:
        break;
    }
}

void VelocityTracker::tick()
{
    if (d->state == VelocityTrackerPrivate:: AutoScroll) {
        d->speed = deaccelerate(d->speed, 1);
        emit changedDelta(d->speed);
        if (d->speed == QPoint(0, 0)) {
            d->state = VelocityTrackerPrivate::Steady;
            stop();
        }
    } else {
        stop();
    }
}

bool VelocityTracker::isScrolling() const
{
    return d->state == VelocityTrackerPrivate::AutoScroll ||
            d->state == VelocityTrackerPrivate::ManualScroll;
}

void VelocityTracker::stopAutoScrolling()
{
    d->state = VelocityTrackerPrivate::Steady;
    d->speed = QPoint(0, 0);
    stop();
}
