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

#ifndef COMMONSETTING_H
#define COMMONSETTING_H

#include <QWidget>


namespace Ui {
    class CommonSetting;
}

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QLabel;
class QSpinBox;
class QSlider;
class QPushButton;
class CommonSetting : public QWidget
{
    Q_OBJECT

public:
    explicit CommonSetting(QWidget *parent = 0);
    ~CommonSetting();

signals:
    void clickClose();

protected slots:
    void initUIvalue();
    void restoreDefaults();

    void showDialogChange(int state);
    void scaleModeChange(int index);
    void alignModeChange(int id);
    void antialiasModeChange(int index);
    void bgColorEnable(int state);
    void setColor();
    void timerIntervalChange(int val);
    void preReadingChanged(int state);
    void cacheValueChanged(int val);

private:
    Ui::CommonSetting *ui;

    QCheckBox *showDialogCheckBox;
    QComboBox *scaleModeCombo;
    QButtonGroup *alignButtonGroup;
    QComboBox *antialiasModeCombo;
    QSpinBox  *timerSpinBox;
    QPushButton *colorButton;
    QLineEdit *colorEdit;
    QCheckBox *colorCheckBox;
    QLabel    *colorLabel;
    QCheckBox *preReadingCheckBox;
    QLabel    *cacheValueLabel;
    QSlider   *cacheValueSlider;
};


#endif // COMMONSETTING_H
