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
#include "osrelated.h"
#include <QtGui>


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
    QSettings settings(INI_FILE_PATH, QSettings::IniFormat);
    bool showDialog = settings.value(DialogKey, true).toBool();
    int antialiasMode = settings.value(AntialiasModeKey, 0).toInt();
    bool enableBgColor = settings.value(EnableBgColorKey, true).toBool();
    QString colorStr = settings.value(BgColorKey, BG_GREEN).toString();
    int timerInterval = settings.value(TimerIntervalKey, 4).toInt();
    bool enablePreReading = settings.value(EnablePreReadingKey, true).toBool();
    int  cacheValue = settings.value(CacheValueKey, -1).toInt();

    if(antialiasMode < 0 || antialiasMode > 2)
        antialiasMode = 0;
    bgColor.setNamedColor(colorStr);
    if(!bgColor.isValid())
        bgColor.setNamedColor(BG_GREEN);
    if(timerInterval < 1 || timerInterval > 1000)
        timerInterval = 4;
    if(cacheValue < 0 || cacheValue > 5)
        cacheValue = OSRelated::cacheSizeSuggested();

    showDialogCheckBox->setChecked(showDialog);
    antialiasModeCombo->setCurrentIndex(antialiasMode);
    timerSpinBox->setValue(timerInterval);
    preReadingCheckBox->setChecked(enablePreReading);
    cacheValueLabel->setText(QString::number(cacheValue));
    cacheValueSlider->setValue(cacheValue);

    QPixmap pix(25, 25);
    pix.fill(bgColor);
    colorButton->setIcon(QIcon(pix));
    colorEdit->setText(bgColor.name());
    colorCheckBox->setChecked(enableBgColor);
    colorLabel->setEnabled(enableBgColor);
    colorButton->setEnabled(enableBgColor);
    colorEdit->setEnabled(enableBgColor);
}

void SettingWidget::showDialogChange(int state)
{
    QSettings settings(INI_FILE_PATH, QSettings::IniFormat);
    switch(state){
    case Qt::Checked:
        settings.setValue(DialogKey, true);
        break;
    case Qt::Unchecked:
        settings.setValue(DialogKey, false);
        break;
    }
}

void SettingWidget::antialiasModeChange(int index)
{
    if(index == -1) return;

    QSettings settings(INI_FILE_PATH, QSettings::IniFormat);
    settings.setValue(AntialiasModeKey, index);
}

void SettingWidget::bgColorEnable(int state)
{
    QSettings settings(INI_FILE_PATH, QSettings::IniFormat);
    bool enableBgColor = true;
    switch(state){
    case Qt::Checked:
        settings.setValue(EnableBgColorKey, true);
        enableBgColor = true;
        break;
    case Qt::Unchecked:
        settings.setValue(EnableBgColorKey, false);
        enableBgColor = false;
        break;
    }

    colorLabel->setEnabled(enableBgColor);
    colorButton->setEnabled(enableBgColor);
    colorEdit->setEnabled(enableBgColor);
}

void SettingWidget::setColor()
{
    QColor color = QColorDialog::getColor(bgColor, this);//! Native Dialog
    //QColorDialog::getColor(bgColor, this, tr("Select Color"), QColorDialog::DontUseNativeDialog);

    if (color.isValid()) {
        bgColor = color;
        QPixmap pix(25, 25);
        pix.fill(bgColor);
        colorButton->setIcon(QIcon(pix));
        colorEdit->setText(bgColor.name());
        QSettings settings(INI_FILE_PATH, QSettings::IniFormat);
        settings.setValue(BgColorKey, bgColor.name());
    }
}

void SettingWidget::timerIntervalChange(int val)
{
    QSettings settings(INI_FILE_PATH, QSettings::IniFormat);
    settings.setValue(TimerIntervalKey, val);
}

void SettingWidget::preReadingChanged(int state)
{
    QSettings settings(INI_FILE_PATH, QSettings::IniFormat);
    switch(state){
    case Qt::Checked:
        settings.setValue(EnablePreReadingKey, true);
        break;
    case Qt::Unchecked:
        settings.setValue(EnablePreReadingKey, false);
        break;
    }
}

void SettingWidget::cacheValueChanged(int val)
{
    QSettings settings(INI_FILE_PATH, QSettings::IniFormat);
    settings.setValue(CacheValueKey, val);

    cacheValueLabel->setText(QString::number(val));
}

void SettingWidget::restoreDefaults()
{
    QSettings(INI_FILE_PATH, QSettings::IniFormat).clear(); ///
    initUIvalue();
}


/**********************************************************************
 *
 *  class SettingsDialog
 *
 **********************************************************************/

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint),
      sw(this)
{
    connect(&sw, SIGNAL(clickClose()), SLOT(close()));

    QTabWidget *tab = new QTabWidget(this);
    tab->addTab(&sw, tr("Common"));

    if(OSRelated::isSupportAssociation()){
        QGridLayout *gl = new QGridLayout;

        const int CountOfColumn = 3;
        QStringList formatList = QString(SUPPORT_FORMAT).remove("*.").split(' ');
        QCheckBox *cb;
        for(int i = 0, size = formatList.size(); i < size; ++i){
            cb = new QCheckBox(formatList.at(i));
            //! before connect(). otherwise it will launch the function changeAssociation(bool).
            cb->setChecked(OSRelated::checkAssociation(formatList.at(i)));
            connect(cb, SIGNAL(toggled(bool)),
                    SLOT(changeAssociation(bool)));

            gl->addWidget(cb, i / CountOfColumn, i % CountOfColumn);
        }

        QWidget *assocWidget = new QWidget(this);
        assocWidget->setLayout(gl);
        tab->addTab(assocWidget, tr("File Association"));
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tab);
    setLayout(layout);

    setWindowTitle(GlobalStr::PROJECT_NAME());
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
