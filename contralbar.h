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

#ifndef CONTRALBAR_H
#define CONTRALBAR_H

#include <QWidget>

namespace Ui {
    class ContralBar;
}

class QPushButton;
class ContralBar : public QWidget
{
    Q_OBJECT

public:
    explicit ContralBar(QWidget *parent = 0);
    ~ContralBar();

    QPushButton *settingButton;
    QPushButton *openButton;
    QPushButton *preButton;
    QPushButton *playButton;
    QPushButton *nextButton;
    QPushButton *rotateLeftButton;
    QPushButton *rotateRightButton;
    QPushButton *deleteButton;

private:
    Ui::ContralBar *ui;
};

#endif // CONTRALBAR_H
