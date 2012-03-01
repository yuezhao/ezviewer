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

#include "settingwidget.h"
#include "ui_settingwidget.h"

#include "global.h"
#include <QtGui>

#include <QDebug>


SettingWidget::SettingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingWidget)
{
    ui->setupUi(this);

    initUIvalue();

    connect(ui->sizeModeCombo, SIGNAL(currentIndexChanged(int)),
            SLOT(sizeModeChange(int)));
    connect(ui->showDialogCheckBox, SIGNAL(stateChanged(int)),
            SLOT(showDialogChange(int)));
    connect(ui->antialiasModeCombo, SIGNAL(currentIndexChanged(int)),
            SLOT(antialiasModeChange(int)));
    connect(ui->colorButton, SIGNAL(clicked()), SLOT(setColor()));
    connect(ui->colorCheckBox, SIGNAL(stateChanged(int)),
            SLOT(bgColorEnable(int)));
    connect(ui->timerSpinBox, SIGNAL(valueChanged(int)),
            SLOT(timerIntervalChange(int)));

    QPushButton *button = ui->buttonBox->addButton(QDialogButtonBox::Close);
    button->setDefault(true);
    connect(button, SIGNAL(clicked()), SIGNAL(clickClose()));
    button = ui->buttonBox->addButton(QDialogButtonBox::RestoreDefaults);
    connect(button, SIGNAL(clicked()), SLOT(restoreDefaults()));
}

SettingWidget::~SettingWidget()
{
    delete ui;
}

void SettingWidget::initUIvalue()
{
    QSettings *settings = new QSettings(INI_FILE_PATH, QSettings::IniFormat);
    int sizeMode = settings->value(SizeModeKey, 0).toInt();
    bool showDialog = settings->value(DialogKey, true).toBool();
    int antialiasMode = settings->value(AntialiasModeKey, 0).toInt();
    bool enableBgColor = settings->value(EnableBgColorKey, true).toBool();
    QString colorStr = settings->value(BgColorKey, BG_GREEN).toString();
    int timerInterval = settings->value(TimerIntervalKey, 4).toInt();

    if(sizeMode < 0 || sizeMode > 3)
        sizeMode = 0;
    if(antialiasMode < 0 || antialiasMode > 2)
        antialiasMode = 0;
    bgColor.setNamedColor(colorStr);
    if(!bgColor.isValid())
        bgColor.setNamedColor(BG_GREEN);
    if(timerInterval < 1 || timerInterval > 1000)
        timerInterval = 4;

    ui->sizeModeCombo->setCurrentIndex(sizeMode);
    ui->showDialogCheckBox->setChecked(showDialog);
    ui->antialiasModeCombo->setCurrentIndex(antialiasMode);
    ui->timerSpinBox->setValue(timerInterval);

    QPixmap pix(25, 25);
    pix.fill(bgColor);
    ui->colorButton->setIcon(QIcon(pix));
    ui->colorEdit->setText(bgColor.name());
    ui->colorCheckBox->setChecked(enableBgColor);
    ui->colorLabel->setEnabled(enableBgColor);
    ui->colorButton->setEnabled(enableBgColor);
    ui->colorEdit->setEnabled(enableBgColor);
}

void SettingWidget::sizeModeChange(int index)
{
    if(index == -1) return;

    QSettings *settings = new QSettings(INI_FILE_PATH, QSettings::IniFormat);
    settings->setValue(SizeModeKey, index);
    emit enableSelfAdaptive(index == 3);
}

void SettingWidget::showDialogChange(int state)
{
    QSettings *settings = new QSettings(INI_FILE_PATH, QSettings::IniFormat);
    switch(state){
    case Qt::Checked:
        settings->setValue(DialogKey, true);
        break;
    case Qt::Unchecked:
        settings->setValue(DialogKey, false);
        break;
    }
}

void SettingWidget::antialiasModeChange(int index)
{
    if(index == -1) return;

    QSettings *settings = new QSettings(INI_FILE_PATH, QSettings::IniFormat);
    settings->setValue(AntialiasModeKey, index);
    emit changeAntialiasMode(index);
}

void SettingWidget::bgColorEnable(int state)
{
    QSettings *settings = new QSettings(INI_FILE_PATH, QSettings::IniFormat);
    bool enableBgColor = true;
    switch(state){
    case Qt::Checked:
        settings->setValue(EnableBgColorKey, true);
        enableBgColor = true;
        break;
    case Qt::Unchecked:
        settings->setValue(EnableBgColorKey, false);
        enableBgColor = false;
        break;
    }

    ui->colorLabel->setEnabled(enableBgColor);
    ui->colorButton->setEnabled(enableBgColor);
    ui->colorEdit->setEnabled(enableBgColor);
    if(enableBgColor)
        emit changeBgColor(bgColor);
    else
        emit changeBgColor(QColor());
}

void SettingWidget::setColor()
{
    QColor color = QColorDialog::getColor(bgColor, this);//! Native Dialog
    //QColorDialog::getColor(bgColor, this, tr("Select Color"), QColorDialog::DontUseNativeDialog);

    if (color.isValid()) {
        bgColor = color;
        QPixmap pix(25, 25);
        pix.fill(bgColor);
        ui->colorButton->setIcon(QIcon(pix));
        ui->colorEdit->setText(bgColor.name());
        QSettings *settings = new QSettings(INI_FILE_PATH, QSettings::IniFormat);
        settings->setValue(BgColorKey, bgColor.name());
        emit changeBgColor(bgColor);
    }
}

void SettingWidget::timerIntervalChange(int val)
{
//    qDebug() << "timer value change to " << val;
    QSettings *settings = new QSettings(INI_FILE_PATH, QSettings::IniFormat);
    settings->setValue(TimerIntervalKey, val);
    emit changeTimerInterval(val);
}

void SettingWidget::restoreDefaults()
{
    QFile::remove(INI_FILE_PATH);
    initUIvalue();
    emit changeAntialiasMode(ui->antialiasModeCombo->currentIndex());
    emit changeBgColor(bgColor);
}
