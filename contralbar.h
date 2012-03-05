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
