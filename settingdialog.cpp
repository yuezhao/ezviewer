/****************************************************************************
 * EZ Viewer
 * Copyright (C) 2013 huangezhao. CHINA.
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

#include "settingdialog.h"
#include "config.h"
#include "global.h"
#include "commonsetting.h"
#include "osrelated.h"
#include "shortcutsetting.h"

#include <QCoreApplication>
#include <QCheckBox>
#include <QGridLayout>
#include <QPushButton>
#include <QTabWidget>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtConcurrent/QtConcurrentRun>
#else
#include <QtConcurrentRun>
#endif // QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)


SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint
              | Qt::WindowCloseButtonHint),
      commonSetting(new CommonSetting(this)),
      shortcutSetting(new ShortcutSetting(this)),
      willExit(false)
{
    QTabWidget *tab = new QTabWidget(this);
    tab->addTab(commonSetting, tr("Common"));

    QWidget *assocWidget = creatAssociationWidget();
    if (assocWidget)
        tab->addTab(assocWidget, tr("File Association"));

    tab->addTab(shortcutSetting, tr("Keyboard"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tab);
    setLayout(layout);

    setWindowTitle(Global::ProjectName());

    connect(commonSetting, SIGNAL(clickClose()), SLOT(close()));
    connect(shortcutSetting, SIGNAL(clickClose()), SLOT(close()));
}

QWidget *SettingsDialog::creatAssociationWidget()
{
    QWidget *assocWidget = NULL;
    if(OSRelated::isSupportAssociation()){
        QGridLayout *mainLayout = new QGridLayout(this);

        QHBoxLayout *horizontalLayout = new QHBoxLayout();
        QSpacerItem *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        horizontalLayout->addItem(verticalSpacer);
        gridLayout = new QGridLayout();
        horizontalLayout->addLayout(gridLayout);

        mainLayout->addLayout(horizontalLayout, 0, 0, 1, 1);


        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setContentsMargins(-1, 20, -1, -1);
        QSpacerItem *horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        horizontalLayout->addItem(horizontalSpacer);

        checkAllButton = new QPushButton(tr("Check All"), this);
        checkAllButton->setFocusPolicy(Qt::NoFocus);
        connect(checkAllButton, SIGNAL(clicked()), SLOT(switchAllAssociation()));
        horizontalLayout->addWidget(checkAllButton);

        uncheckAllButton = new QPushButton(tr("Uncheck All"), this);
        uncheckAllButton->setFocusPolicy(Qt::NoFocus);
        connect(uncheckAllButton, SIGNAL(clicked()), SLOT(switchAllAssociation()));
        horizontalLayout->addWidget(uncheckAllButton);

        QPushButton *closeButton = new QPushButton(tr("Close"), this);
        connect(closeButton, SIGNAL(clicked()), SLOT(close()));
        horizontalLayout->addWidget(closeButton);

        mainLayout->addLayout(horizontalLayout, 1, 0, 1, 1);



        const int CountOfColumn = 3;
        QStringList formatList = Config::formatsList();
        QCheckBox *cb;
        for(int i = 0, size = formatList.size(); i < size; ++i){
            cb = new QCheckBox(formatList.at(i));
            gridLayout->addWidget(cb, i / CountOfColumn, i % CountOfColumn);
        }

        assocWidget = new QWidget(this);
        assocWidget->setLayout(mainLayout);

        future = QtConcurrent::run(this, &SettingsDialog::reviewFileAssociation);
    }

    return assocWidget;
}

void SettingsDialog::reviewFileAssociation()
{
    int size = gridLayout->count();
    QCheckBox *cb;
    for(int i = 0; i < size; ++i){
        if (willExit)
            break;

        cb = dynamic_cast<QCheckBox*>(gridLayout->itemAt(i)->widget());
        if (cb) {
            //! before connect(). otherwise it will launch the function changeAssociation(bool).
            cb->setChecked(OSRelated::checkAssociation(cb->text()));
            connect(cb, SIGNAL(toggled(bool)), SLOT(changeAssociation(bool)));
        }
    }

    reviewCheckAllButtonState();
}

void SettingsDialog::reviewCheckAllButtonState()
{
    int size = gridLayout->count();
    QCheckBox *cb;
    bool isAllChecked = true;
    bool isAllUnchecked = true;
    for(int i = 0; i < size; ++i){
        cb = dynamic_cast<QCheckBox*>(gridLayout->itemAt(i)->widget());
        if (cb) {
            if (cb->isChecked())
                isAllUnchecked = false;
            else
                isAllChecked = false;
        }
    }

    checkAllButton->setEnabled(!isAllChecked);
    uncheckAllButton->setEnabled(!isAllUnchecked);
}

void SettingsDialog::changeAssociation(bool enabled)
{
    QCheckBox *cb = dynamic_cast<QCheckBox*>(sender());
    if(cb == NULL) return;

    QString ext(cb->text());
    if(enabled){
        OSRelated::setAssociation(ext);
    }else{
        OSRelated::clearAssociation(ext);
    }
    OSRelated::refreshExplorer();

    reviewCheckAllButtonState();
}

void SettingsDialog::switchAllAssociation()
{
    QPushButton *button = dynamic_cast<QPushButton*>(sender());
    if (button == checkAllButton) {
        future = QtConcurrent::run(this, &SettingsDialog::changeAllAssociation, true);
    } else if (button == uncheckAllButton) {
        future = QtConcurrent::run(this, &SettingsDialog::changeAllAssociation, false);
    }

    while (!future.isFinished())
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void SettingsDialog::changeAllAssociation(bool checked)
{
    int size = gridLayout->count();
    QCheckBox *cb;
    for(int i = 0; i < size; ++i){
        cb = dynamic_cast<QCheckBox*>(gridLayout->itemAt(i)->widget());
        if (cb) {
            if (cb->isChecked() == checked)
                continue;

            QString ext(cb->text());
            if(checked){
                OSRelated::setAssociation(ext);
            }else{
                OSRelated::clearAssociation(ext);
            }
            cb->setChecked(checked);
        }
    }

    OSRelated::refreshExplorer();

    reviewCheckAllButtonState();
}

void SettingsDialog::done(int r)
{
    willExit = true;
    if (OSRelated::isSupportAssociation() && !future.isFinished())
        future.waitForFinished();

    QDialog::done(r);
}
