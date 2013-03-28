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

#include "settingwidget.h"
#include "ui_settingwidget.h"

#include "config.h"
#include "global.h"
#include "osrelated.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QColorDialog>
#include <QGridLayout>
#include <QTabWidget>
#include <QtConcurrentRun>


SettingWidget::SettingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingWidget)
{
    ui->setupUi(this);
    ui->ui_groupBox->hide();

    showDialogCheckBox = ui->showDialogCheckBox;
    antialiasModeCombo = ui->antialiasModeCombo;
    timerSpinBox = ui->timerSpinBox;
    colorButton = ui->colorButton;
    colorEdit = ui->colorEdit;
    colorCheckBox = ui->colorCheckBox;
    colorLabel = ui->colorLabel;
    preReadingCheckBox = ui->preReadingCheckBox;
    buttonBox = ui->buttonBox;
    cacheValueLabel = ui->cacheValueLabel;
    cacheValueSlider = ui->cacheValueSlider;

    initUIvalue();

    connect(showDialogCheckBox, SIGNAL(stateChanged(int)),
            SLOT(showDialogChange(int)));
    connect(antialiasModeCombo, SIGNAL(currentIndexChanged(int)),
            SLOT(antialiasModeChange(int)));
    connect(colorButton, SIGNAL(clicked()), SLOT(setColor()));
    connect(colorCheckBox, SIGNAL(stateChanged(int)),
            SLOT(bgColorEnable(int)));
    connect(timerSpinBox, SIGNAL(valueChanged(int)),
            SLOT(timerIntervalChange(int)));
    connect(preReadingCheckBox, SIGNAL(stateChanged(int)),
            SLOT(preReadingChanged(int)));
    connect(cacheValueSlider, SIGNAL(valueChanged(int)),
            SLOT(cacheValueChanged(int)));

    QPushButton *button = buttonBox->addButton(QDialogButtonBox::Close);
    button->setDefault(true);
    connect(button, SIGNAL(clicked()), SIGNAL(clickClose()));
    button = buttonBox->addButton(QDialogButtonBox::RestoreDefaults);
    connect(button, SIGNAL(clicked()), SLOT(restoreDefaults()));
}

SettingWidget::~SettingWidget()
{
    delete ui;
}

void SettingWidget::initUIvalue()
{
    showDialogCheckBox->setChecked(Config::showDialog());
    antialiasModeCombo->setCurrentIndex(Config::antialiasMode());
    timerSpinBox->setValue(Config::timerInterval());
    preReadingCheckBox->setChecked(Config::enablePreReading());
    cacheValueLabel->setText(QString::number(Config::cacheValue()));
    cacheValueSlider->setValue(Config::cacheValue());

    QPixmap pix(25, 25);
    pix.fill(Config::bgColor());
    colorButton->setIcon(QIcon(pix));
    colorEdit->setText(Config::bgColor().name());
    bool enabledBgColor = Config::enableBgColor();
    colorCheckBox->setChecked(enabledBgColor);
    colorLabel->setEnabled(enabledBgColor);
    colorButton->setEnabled(enabledBgColor);
    colorEdit->setEnabled(enabledBgColor);
}

void SettingWidget::showDialogChange(int state)
{
    Config::setShowDialog(state == Qt::Checked);
}

void SettingWidget::antialiasModeChange(int index)
{
    if(index == -1) return;
    Config::setAntialiasMode(index);
}

void SettingWidget::bgColorEnable(int state)
{
    bool enableBgColor = (state == Qt::Checked);
    Config::setEnableBgColor(enableBgColor);

    colorLabel->setEnabled(enableBgColor);
    colorButton->setEnabled(enableBgColor);
    colorEdit->setEnabled(enableBgColor);
}

void SettingWidget::setColor()
{
    QColor color = QColorDialog::getColor(Config::bgColor(), this);
    if (color.isValid()) {
        QPixmap pix(25, 25);
        pix.fill(color);
        colorButton->setIcon(QIcon(pix));
        colorEdit->setText(color.name());
        Config::setBgColor(color);
    }
}

void SettingWidget::timerIntervalChange(int val)
{
    Config::setTimerInterval(val);
}

void SettingWidget::preReadingChanged(int state)
{
    Config::setEnablePreReading(state == Qt::Checked);
}

void SettingWidget::cacheValueChanged(int val)
{
    Config::setCacheValue(val);
    cacheValueLabel->setText(QString::number(val));
}

void SettingWidget::restoreDefaults()
{
    Config::clearConfig();
    qApp->processEvents();  /// make sure the value of Config refreshed.
    initUIvalue();
}


/**********************************************************************
 *
 *  class SettingsDialog
 *
 **********************************************************************/

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint),
      settingWidget(this)
{
    QTabWidget *tab = new QTabWidget(this);
    tab->addTab(&settingWidget, tr("Common"));

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

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tab);
    setLayout(layout);

    setWindowTitle(Global::ProjectName());

    connect(&settingWidget, SIGNAL(clickClose()), SLOT(close()));
}

void SettingsDialog::checkFileAssociation()
{
    int size = gridLayout->count();
    QCheckBox *cb;
    for(int i = 0; i < size; ++i){
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
    if (OSRelated::isSupportAssociation() && !future.isFinished())
        future.waitForFinished();

    QDialog::done(r);
}
