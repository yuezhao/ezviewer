/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <qlabel.h>
#include <qpointer.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qstylepainter.h>
#include <qtimer.h>
#include <qtextdocument.h>
#include <qdebug.h>

#include "tooltip.h"


class TipLabel : public QLabel
{
    Q_OBJECT
public:
    TipLabel(const QString &text, QWidget *w, int minMsec = 10000);
    ~TipLabel();
    static TipLabel *instance;

    bool eventFilter(QObject *, QEvent *);

    QBasicTimer hideTimer, expireTimer;

    bool fadingOut;

    void reuseTip(const QString &text, int minMsec = 10000);
    void hideTip();
    void hideTipImmediately();
    void setTipRect(QWidget *w, const QRect &r);
    void restartExpireTimer(int minMsec = 10000);
    bool tipChanged(const QPoint &pos, const QString &text, QObject *o);

    /**
     * @param alignCenter default is false, which means align with left top.
     */
    void placeTip(const QPoint &pos, QWidget *w, bool alignCenter = false);

    static int getTipScreen(const QPoint &pos, QWidget *w);
protected:
    void timerEvent(QTimerEvent *e);
    void paintEvent(QPaintEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);

private:
    QWidget *widget;
    QRect rect;
};

TipLabel *TipLabel::instance = 0;

TipLabel::TipLabel(const QString &text, QWidget *w, int minMsec)
    : QLabel(w, Qt::ToolTip | Qt::BypassGraphicsProxyWidget), widget(0)
{
    delete instance;
    instance = this;
    setForegroundRole(QPalette::ToolTipText);
    setBackgroundRole(QPalette::ToolTipBase);
//    setPalette(ToolTip::palette());
    ensurePolished();
    setMargin(10 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0, this));
    setFrameStyle(QFrame::NoFrame);
    setAlignment(Qt::AlignLeft);
    setIndent(1);
    qApp->installEventFilter(this);
    setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0, this) / 255.0);
    setMouseTracking(true);
    fadingOut = false;
    reuseTip(text, minMsec);

    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void TipLabel::restartExpireTimer(int minMsec)
{
    int time = minMsec + 40 * qMax(0, text().length()-100);
    expireTimer.start(time, this);
    hideTimer.stop();
}

void TipLabel::reuseTip(const QString &text, int minMsec)
{
    setWordWrap(Qt::mightBeRichText(text));
    setText(text);
    QFontMetrics fm(font());
    QSize extra(1, 0);
    // Make it look good with the default ToolTip font on Mac, which has a small descent.
    if (fm.descent() == 2 && fm.ascent() >= 11)
        ++extra.rheight();
    resize(sizeHint() + extra);
    restartExpireTimer(minMsec);
}

void TipLabel::paintEvent(QPaintEvent *ev)
{
    QStylePainter p(this);
    QStyleOptionFrame opt;
    opt.init(this);
    p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
    p.end();

    QLabel::paintEvent(ev);
}

void TipLabel::resizeEvent(QResizeEvent *e)
{
    QStyleHintReturnMask frameMask;
    QStyleOption option;
    option.init(this);
    if (style()->styleHint(QStyle::SH_ToolTip_Mask, &option, this, &frameMask))
        setMask(frameMask.region);

    QLabel::resizeEvent(e);
}

void TipLabel::mouseMoveEvent(QMouseEvent *e)
{
    if (rect.isNull())
        return;
    QPoint pos = e->globalPos();
    if (widget)
        pos = widget->mapFromGlobal(pos);
    if (!rect.contains(pos))
        hideTip();
    QLabel::mouseMoveEvent(e);
}

TipLabel::~TipLabel()
{
    instance = 0;
}

void TipLabel::hideTip()
{
    if (!hideTimer.isActive())
        hideTimer.start(300, this);
}

void TipLabel::hideTipImmediately()
{
    close(); // to trigger QEvent::Close which stops the animation
    deleteLater();
}

void TipLabel::setTipRect(QWidget *w, const QRect &r)
{
    if (!rect.isNull() && !w)
        qWarning("QToolTip::setTipRect: Cannot pass null widget if rect is set");
    else{
        widget = w;
        rect = r;
    }
}

void TipLabel::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == hideTimer.timerId()
        || e->timerId() == expireTimer.timerId()){
        hideTimer.stop();
        expireTimer.stop();

        hideTipImmediately();
    }
}

bool TipLabel::eventFilter(QObject *o, QEvent *e)
{
    switch (e->type()) {
    case QEvent::KeyPress:
    case QEvent::Leave:
        hideTip();
        break;
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::FocusIn:
    case QEvent::FocusOut:
//    case QEvent::Wheel:
        hideTipImmediately();
        break;

    case QEvent::MouseMove:
        if (o == widget && !rect.isNull() && !rect.contains(static_cast<QMouseEvent*>(e)->pos()))
            hideTip();
    default:
        break;
    }
    return false;
}

int TipLabel::getTipScreen(const QPoint &pos, QWidget *w)
{
    if (QApplication::desktop()->isVirtualDesktop())
        return QApplication::desktop()->screenNumber(pos);
    else
        return QApplication::desktop()->screenNumber(w);
}

void TipLabel::placeTip(const QPoint &pos, QWidget *w, bool alignCenter)
{
    QRect screen = QApplication::desktop()->screenGeometry(getTipScreen(pos, w));

    QPoint p = pos;
    if (alignCenter){    // Make sure place in the widget center.
        QSize tipSize = sizeHint();
        p -= QPoint(tipSize.width(), tipSize.height()) / 2;
    }

    if (p.x() + this->width() > screen.x() + screen.width())
        p.rx() -= 4 + this->width();
    if (p.y() + this->height() > screen.y() + screen.height())
        p.ry() -= 24 + this->height();
    if (p.y() < screen.y())
        p.setY(screen.y());
    if (p.x() + this->width() > screen.x() + screen.width())
        p.setX(screen.x() + screen.width() - this->width());
    if (p.x() < screen.x())
        p.setX(screen.x());
    if (p.y() + this->height() > screen.y() + screen.height())
        p.setY(screen.y() + screen.height() - this->height());
    this->move(p);
}

bool TipLabel::tipChanged(const QPoint &pos, const QString &text, QObject *o)
{
    if (TipLabel::instance->text() != text)
        return true;

    if (o != widget)
        return true;

    if (!rect.isNull())
        return !rect.contains(pos);
    else
       return false;
}

void ToolTip::showText(const QPoint &pos, const QString &text, QWidget *w, const QRect &rect, bool alignCenter, qreal opacity, int minMsec)
{
    if (TipLabel::instance && TipLabel::instance->isVisible()){ // a tip does already exist
        if (text.isEmpty()){ // empty text means hide current tip
            TipLabel::instance->hideTip();
            return;
        }
        else if (!TipLabel::instance->fadingOut){
            // If the tip has changed, reuse the one
            // that is showing (removes flickering)
            QPoint localPos = pos;
            if (w)
                localPos = w->mapFromGlobal(pos);
            if (TipLabel::instance->tipChanged(localPos, text, w)){
                TipLabel::instance->reuseTip(text, minMsec);
                if(opacity >= 0)
                    TipLabel::instance->setWindowOpacity(opacity);
                TipLabel::instance->setTipRect(w, rect);
                TipLabel::instance->placeTip(pos, w, alignCenter);
            }
            return;
        }
    }

    if (!text.isEmpty()){ // no tip can be reused, create new tip:
#ifndef Q_WS_WIN
        new TipLabel(text, w, minMsec); // sets QTipLabel::instance to itself
#else
        // On windows, we can't use the widget as parent otherwise the window will be
        // raised when the tooltip will be shown
        new TipLabel(text, QApplication::desktop()->screen(TipLabel::getTipScreen(pos, w)), minMsec);
#endif
        if(opacity >= 0)
            TipLabel::instance->setWindowOpacity(opacity);
        TipLabel::instance->setTipRect(w, rect);
        TipLabel::instance->placeTip(pos, w, alignCenter);
        TipLabel::instance->setObjectName(QLatin1String("qtooltip_label"));

        TipLabel::instance->show();
    }
}

/*!
    \overload

    This is analogous to calling QToolTip::showText(\a pos, \a text, \a w, QRect())
*/

void ToolTip::showText(const QPoint &pos, const QString &text, QWidget *w)
{
    ToolTip::showText(pos, text, w, QRect());
}

void ToolTip::showText(const QPoint &pos, const QString &text, bool alignCenter, qreal opacity, int minMsec)
{
    ToolTip::showText(pos, text, 0, QRect(), alignCenter, opacity, minMsec);
}

/*!
  \since 4.4

  Returns true if this tooltip is currently shown.

  \sa showText()
 */
bool ToolTip::isVisible()
{
    return (TipLabel::instance != 0 && TipLabel::instance->isVisible());
}

/*!
  \since 4.4

  Returns the tooltip text, if a tooltip is visible, or an
  empty string if a tooltip is not visible.
 */
QString ToolTip::text()
{
    if (TipLabel::instance)
        return TipLabel::instance->text();
    return QString();
}


/*!
    \since 4.2

    Returns the font used to render tooltips.
*/
QFont ToolTip::font()
{
    return QApplication::font("QTipLabel");
}


/*!
    \since 4.2

    Sets the \a font used to render tooltips.
*/
void ToolTip::setFont(const QFont &font)
{
    QApplication::setFont(font, "QTipLabel");
}

#include "tooltip.moc"
