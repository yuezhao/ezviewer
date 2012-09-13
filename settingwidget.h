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

#ifndef SETTINGWIDGET_H
#define SETTINGWIDGET_H

#include <QWidget>

namespace Ui {
    class SettingWidget;
}

class QAbstractButton;
class SettingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingWidget(QWidget *parent = 0);
    ~SettingWidget();

signals:
    void changeBgColor(const QColor &color);
    void changeAntialiasMode(int mode);
    void changeTimerInterval(int val);
    void clickClose();

protected slots:
    void initUIvalue();
    void showDialogChange(int state);
    void antialiasModeChange(int index);
    void bgColorEnable(int state);
    void setColor();
    void timerIntervalChange(int val);

    void restoreDefaults();

private:
    Ui::SettingWidget *ui;
    QColor bgColor;
};

#endif // SETTINGWIDGET_H
