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

#include <QCheckBox>
#include <QGridLayout>
#include <QTabWidget>
#include <QtConcurrentRun>


SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint),
      commonSetting(new CommonSetting(this)),
      shortcutSetting(new ShortcutSetting(this)),
      willExit(false)
{
    QTabWidget *tab = new QTabWidget(this);
    tab->addTab(commonSetting, tr("Common"));

    if(OSRelated::isSupportAssociation()){
        gridLayout = new QGridLayout;

        const int CountOfColumn = 3;
        QStringList formatList = Config::formatsList();
        QCheckBox *cb;
        for(int i = 0, size = formatList.size(); i < size; ++i){
            cb = new QCheckBox(formatList.at(i));
            gridLayout->addWidget(cb, i / CountOfColumn, i % CountOfColumn);
        }

        QWidget *assocWidget = new QWidget(this);
        assocWidget->setLayout(gridLayout);
        tab->addTab(assocWidget, tr("File Association"));

        future = QtConcurrent::run(this, &SettingsDialog::checkFileAssociation);
    }

    tab->addTab(shortcutSetting, tr("Shortcut"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tab);
    setLayout(layout);

    setWindowTitle(Global::ProjectName());

    connect(commonSetting, SIGNAL(clickClose()), SLOT(close()));
}

void SettingsDialog::checkFileAssociation()
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
}

void SettingsDialog::changeAssociation(bool enabled)
{
    QCheckBox *cb = dynamic_cast<QCheckBox*>(sender());
    if(cb == NULL) return;

    QString ext(cb->text());
    if(OSRelated::isSupportAssociation()){
        if(enabled){
            OSRelated::setAssociation(ext);
        }else{
            OSRelated::clearAssociation(ext);
        }
    }
}

void SettingsDialog::done(int r)
{
    willExit = true;
    if (OSRelated::isSupportAssociation() && !future.isFinished())
        future.waitForFinished();

    QDialog::done(r);
}
