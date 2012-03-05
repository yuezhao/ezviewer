#include "contralbar.h"
#include "ui_contralbar.h"

ContralBar::ContralBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContralBar)
{
    ui->setupUi(this);

    settingButton = ui->settingButton;
    openButton = ui->openButton;
    preButton = ui->preButton;
    playButton = ui->playButton;
    nextButton = ui->nextButton;
    rotateLeftButton = ui->rotateLeftButton;
    rotateRightButton = ui->rotateRightButton;
    deleteButton = ui->deleteButton;
}

ContralBar::~ContralBar()
{
    delete ui;
}
