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

#ifndef SHORTCUTSETTING_H
#define SHORTCUTSETTING_H

#include <QWidget>

namespace Ui {
    class ShortcutSetting;
}

class QTableWidget;
class QLabel;
class QLineEdit;
class QPushButton;
class ShortcutSetting : public QWidget
{
    Q_OBJECT

public:
    explicit ShortcutSetting(QWidget *parent = 0);
    ~ShortcutSetting();

signals:
    void clickClose();

private slots:
    void currentItemChanged(int currentRow, int currentColumn,
                            int previousRow, int previousColumn);

    void removeShortcut();

    bool eventFilter(QObject *obj, QEvent *event);

private:
    void setupViews();
    void setupData();

    void addShortcut(const QString &keySequence);

    Ui::ShortcutSetting *ui;

    QTableWidget *table;
    QLabel *label;
    QLineEdit *lineEdit;
    QPushButton *button;

    QStringList actionScripts;
};

#endif // SHORTCUTSETTING_H
