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

#include "shortcutsetting.h"
#include "ui_shortcutsetting.h"

#include "actionmanager.h"
#include "config.h"

#include <QKeyEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>


const char *SplitFlag = ", ";

ShortcutSetting::ShortcutSetting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShortcutSetting)
{
    ui->setupUi(this);

    setupViews();
    setupData();
}

ShortcutSetting::~ShortcutSetting()
{
    delete ui;
}

void ShortcutSetting::setupViews()
{
    table = ui->tableWidget;
    table->setColumnCount(2);

    QStringList headers;
    headers << tr("Function") << tr("Key Sequence");
    table->setHorizontalHeaderLabels(headers);

    label = ui->label;

    lineEdit = ui->lineEdit;
    lineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    QSize size = QSize(lineEdit->sizeHint().height(),
                       lineEdit->sizeHint().height());

    button = new QPushButton(this);
    button->setIcon(QIcon(":/Delete2"));
    button->setFixedSize(size);
    button->setFocusPolicy(Qt::NoFocus);    // Otherwise if get focus, it will show a rect round button.
    button->setFlat(true);
    button->setCursor(QCursor(Qt::PointingHandCursor));
    connect(button, SIGNAL(clicked()), SLOT(removeShortcut()));

    const int RightMargin = 8;
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addStretch();
    buttonLayout->addWidget(button);
    buttonLayout->addSpacing(RightMargin);
    lineEdit->setLayout(buttonLayout);

    // Setting the input rect, protects the text from covered by the button.
    lineEdit->setTextMargins(0, 1, size.width() + RightMargin, 1);
    lineEdit->installEventFilter(this);


    label->setEnabled(false);
    lineEdit->setEnabled(false);
    button->hide();


    QDialogButtonBox *buttonBox = ui->buttonBox;
    QPushButton *button = buttonBox->addButton(QDialogButtonBox::Close);
    connect(button, SIGNAL(clicked()), SIGNAL(clickClose()));
}

void ShortcutSetting::setupData()
{
    QStringList descriptions = ActionManager::getAllActions(&actionScripts);
    table->setRowCount(descriptions.size());

    for (int i = 0; i < descriptions.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(descriptions.at(i)));
        QStringList shortcuts = ActionManager::getBindShortcuts(actionScripts.at(i));
        table->setItem(i, 1, new QTableWidgetItem(shortcuts.join(SplitFlag)));
    }

    table->resizeColumnsToContents();   // Auto resize by the content of columns.
//    table->horizontalHeader()->setStretchLastSection(true); // Fill the rest by the last column.
//    table->horizontalHeader()->setResizeMode(QHeaderView::Stretch); // Make every column stretched and divide all the room equally.

    connect(table, SIGNAL(currentCellChanged(int,int,int,int)),
            SLOT(currentItemChanged(int,int,int,int)));
}

void ShortcutSetting::currentItemChanged(int currentRow, int currentColumn,
                                         int previousRow, int previousColumn)
{
    Q_UNUSED(currentColumn)
    Q_UNUSED(previousRow)
    Q_UNUSED(previousColumn)

    if (currentRow >= 0) {
        QString keySequence = table->item(currentRow, 1)->text();
        label->setEnabled(true);
        lineEdit->setEnabled(true);
        lineEdit->setText(keySequence);
        button->setVisible(!keySequence.isEmpty());
    }
}

void ShortcutSetting::removeShortcut()
{
    int currentRow = table->currentRow();
    if (currentRow >= 0) {
        QTableWidgetItem *item = table->item(currentRow, 1);
        Config::removeShortcut(item->text().split(SplitFlag));
        item->setText(QString::null);
    }

    lineEdit->setText(QString::null);
    button->setVisible(false);
}

void ShortcutSetting::addShortcut(const QString &keySequence)
{
    if (ActionManager::getMatchAction(keySequence).isEmpty()) {
        QString text = lineEdit->text();
        if (!text.isEmpty())
            text.append(SplitFlag);
        text.append(keySequence);

        int currentRow = table->currentRow();
        if (currentRow >= 0) {
            QTableWidgetItem *item = table->item(currentRow, 1);
            Config::addShortcut(keySequence, actionScripts.at(currentRow));
            item->setText(text);
        }

        lineEdit->setText(text);
        button->setVisible(true);
    } // else ... // TODO: show which function bind with the key sequence, use tooltip.
}

bool ShortcutSetting::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != lineEdit || event->type() != QEvent::KeyPress)
        return false;

    QKeyEvent *e = static_cast<QKeyEvent*>(event);
    int keyCode = e->key();
    if (keyCode == Qt::Key_Control || keyCode == Qt::Key_Shift
            || keyCode == Qt::Key_Meta || keyCode == Qt::Key_Alt )
        return true;

    addShortcut(QKeySequence(e->modifiers() + keyCode).toString());
    return true;
}
