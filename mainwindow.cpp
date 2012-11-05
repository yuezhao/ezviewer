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

#include "mainwindow.h"


#include "contralbar.h"
#include "floatframe.h"
#include "settingwidget.h"
#include "global.h"
#include "toolkit.h"

#include <QtGui>


const int SWITCH_FRAME_WIDTH = 90;
const int BUTTOM_FRAME_HEIGHT = 60;

MainWindow::MainWindow(const QStringList &fileList, QWidget *parent) :
    QMainWindow(parent), cfgWatcher(new QFileSystemWatcher(this))
{
    WasMaximized = false;
    slideInterval = 4000;
    slideTimer = new QTimer(this);
    slideTimer->setInterval(slideInterval);
    //! if file numbers is 0 or 1, stop timer???......................
    connect(slideTimer, SIGNAL(timeout()), SLOT(nextPic()));

    viewer = new PicManager(this);
    setCentralWidget(viewer);

//    setMinimumSize(150, 100);

    connect(viewer, SIGNAL(fileNameChange(QString)), SLOT(setMyWindowTitle(QString)));
    connect(viewer, SIGNAL(mouseDoubleClick()), SLOT(changeFullScreen()));
    connect(viewer, SIGNAL(showContextMenu(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(viewer, SIGNAL(siteChange(QPoint)), SLOT(moveWindow(QPoint)));

    initContextMenu();
    initButtomBar();
    initSwitchFrame();

    resize(FIT_SIZE);
    readSettings();
    watchConfigFile();

    setMyWindowTitle();
    setWindowIcon(QIcon(":/res/twitter.png"));
    setAcceptDrops(true);   //! !!

    QSettings settings(INI_FILE_PATH, QSettings::IniFormat);

    if(!fileList.empty())
        viewer->openFiles(fileList);
    else if(settings.value(DialogKey, true).toBool()) // show dialog while launch.
        QTimer::singleShot(0, this, SLOT(openFile()));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    cfgWatcher->removePaths(cfgWatcher->files());
    SafeDelete(cfgWatcher);
    writeSettings();
    event->accept();
}

//void MainWindow::changeUseTitleBar(bool enable)
//{
//    bool visible = isVisible();
//    if(enable){
//        setWindowFlags(Qt::Window);
//        titleFrame->hide();
//    }else{
//        setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
//        titleFrame->show();
//    }

//    if(visible)
//        show(); ///
//}

void MainWindow::initButtomBar()
{
    buttomFrame = new FloatFrame(this);
    buttomFrame->resize(width(), BUTTOM_FRAME_HEIGHT);
    buttomFrame->move(0, height() - BUTTOM_FRAME_HEIGHT);
    connect(buttomFrame, SIGNAL(showContextMenu(QPoint)),
            SLOT(showContextMenu(QPoint)));
    connect(buttomFrame, SIGNAL(mouseDoubleClick()),
            SLOT(changeFullScreen()));
    connect(buttomFrame, SIGNAL(siteChange(QPoint)),
            SLOT(moveWindow(QPoint)));
    ///set all the button's focous policy to Qt::NoFocous in 'ui' file.
//    connect(buttomFrame, SIGNAL(mouseLeave()), SLOT(setFocus()));

    contralBar = new ContralBar(buttomFrame);
    buttomFrame->addWidget(contralBar);

    QHBoxLayout *hlayout = new QHBoxLayout(buttomFrame);
    hlayout->setContentsMargins(0,0,0,0);   ///qframe's layout margis default is not 0.
    hlayout->setAlignment(Qt::AlignCenter);
    buttomFrame->setLayout(hlayout);

    hlayout->addWidget(contralBar);

    settingButton = contralBar->settingButton;
    openButton = contralBar->openButton;
    preButton = contralBar->preButton;
    playButton = contralBar->playButton;
    nextButton = contralBar->nextButton;
    rotateLeftButton = contralBar->rotateLeftButton;
    rotateRightButton = contralBar->rotateRightButton;
    deleteButton = contralBar->deleteButton;

    connect(settingButton, SIGNAL(clicked()), SLOT(setting()));
    connect(openButton, SIGNAL(clicked()), SLOT(openFile()));
    connect(preButton, SIGNAL(clicked()), SLOT(prePic()));
    connect(playButton, SIGNAL(clicked()), SLOT(switchSlideShow()));
    connect(nextButton, SIGNAL(clicked()), SLOT(nextPic()));
    connect(rotateLeftButton, SIGNAL(clicked()), SLOT(rotateLeft()));
    connect(rotateRightButton, SIGNAL(clicked()), SLOT(rotateRight()));
    connect(deleteButton, SIGNAL(clicked()), SLOT(deleteFileAsk()));
}

void MainWindow::initSwitchFrame()
{
    leftFrame = new FloatFrame(this);
    leftFrame->resize(SWITCH_FRAME_WIDTH, height() - 2*BUTTOM_FRAME_HEIGHT);
    leftFrame->move(0, BUTTOM_FRAME_HEIGHT);
    leftFrame->setHideInterval(200);
    connect(leftFrame, SIGNAL(showContextMenu(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(leftFrame, SIGNAL(mouseClicked()), SLOT(prePic()));

    QLabel *lb = new QLabel(leftFrame);
    lb->setPixmap(QPixmap(":/res/Left2.png"));
    leftFrame->addWidget(lb);

    QHBoxLayout *hlayout = new QHBoxLayout(leftFrame);
    hlayout->setAlignment(Qt::AlignCenter);
    hlayout->addWidget(lb);
    leftFrame->setLayout(hlayout);


    rightFrame = new FloatFrame(this);
    rightFrame->resize(SWITCH_FRAME_WIDTH, height() - 2*BUTTOM_FRAME_HEIGHT);
    rightFrame->move(width() - SWITCH_FRAME_WIDTH, BUTTOM_FRAME_HEIGHT);
    rightFrame->setHideInterval(200);
    connect(rightFrame, SIGNAL(showContextMenu(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(rightFrame, SIGNAL(mouseClicked()), SLOT(nextPic()));

    lb = new QLabel(rightFrame);
    lb->setPixmap(QPixmap(":/res/Right2.png"));
    rightFrame->addWidget(lb);

    hlayout = new QHBoxLayout(rightFrame);
    hlayout->setAlignment(Qt::AlignCenter);
    hlayout->addWidget(lb);
    rightFrame->setLayout(hlayout);
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    buttomFrame->resize(width(), BUTTOM_FRAME_HEIGHT);
    buttomFrame->move(0, height() - BUTTOM_FRAME_HEIGHT);
    leftFrame->resize(SWITCH_FRAME_WIDTH, height() - 2*BUTTOM_FRAME_HEIGHT);
    leftFrame->move(0, BUTTOM_FRAME_HEIGHT);
    rightFrame->resize(SWITCH_FRAME_WIDTH, height() - 2*BUTTOM_FRAME_HEIGHT);
    rightFrame->move(width() - SWITCH_FRAME_WIDTH, BUTTOM_FRAME_HEIGHT);
    QWidget::resizeEvent(e);
}

void MainWindow::about()
{
    QMessageBox::about(this,
                       tr("About %1").arg(GlobalStr::PROJECT_NAME()),
                       GlobalStr::ABOUT_TEXT());
}

void MainWindow::setting()
{
    SettingsDialog dlg(this);
    dlg.exec();
}

void MainWindow::setMyWindowTitle(const QString &title)
{
    bool hasFile = !title.isEmpty();
    bool hasPicture = viewer->hasPicture();

    if(hasFile){
        setWindowTitle(title);
    }else{
        setWindowTitle(GlobalStr::PROJECT_NAME());
    }

    preButton->setEnabled(hasFile);
    playButton->setEnabled(hasFile);
    nextButton->setEnabled(hasFile);
    leftFrame->set_enabled(hasFile);    ///
    rightFrame->set_enabled(hasFile);   ///
    rotateLeftButton->setEnabled(hasPicture);
    rotateRightButton->setEnabled(hasPicture);
    deleteButton->setEnabled(!slideTimer->isActive() && hasFile);   ////

    if(!hasFile && slideTimer->isActive())
        switchSlideShow();    ///
}

void MainWindow::openFile()
{
    QString currentFile(viewer->filePath());
    QString defaultDir(currentFile.isEmpty()
                       ? QDesktopServices::storageLocation(QDesktopServices::PicturesLocation)
                       : QFileInfo(currentFile).absolutePath());
    QString fileName =
            QFileDialog::getOpenFileName(
                this, tr("Open File"), defaultDir,
                tr("Images (%1);;All Files (*)").arg(SUPPORT_FORMAT));
    if (!fileName.isEmpty())
        viewer->openFile(fileName);
}

void MainWindow::watchConfigFile()
{
    if(!QFile::exists(INI_FILE_PATH))   // create config file
        QFile(INI_FILE_PATH).open(QIODevice::WriteOnly);

    cfgWatcher->addPath(INI_FILE_PATH);
    connect(cfgWatcher, SIGNAL(fileChanged(QString)), SLOT(applyConfig()));
}

void MainWindow::readSettings()
{
    applyConfig();    ///
    QSettings settings(INI_FILE_PATH, QSettings::IniFormat);
    restoreGeometry(settings.value("geometry").toByteArray());
}

void MainWindow::applyConfig()
{
    QSettings settings(INI_FILE_PATH, QSettings::IniFormat);
    int antialiasMode = settings.value(AntialiasModeKey, 0).toInt();
    bool enableBgColor = settings.value(EnableBgColorKey, true).toBool();
    QString colorStr = settings.value(BgColorKey, BG_GREEN).toString();
    int timerInterval = settings.value(TimerIntervalKey, 4).toInt();

    if(antialiasMode < 0 || antialiasMode > 2) // 3 modes
        antialiasMode = 0;
    QColor bgColor(colorStr);
    if(!bgColor.isValid())
        bgColor.setNamedColor(BG_GREEN);
    if(timerInterval < 1 || timerInterval > 1000)
        timerInterval = 4;

    viewer->changeAntialiasMode(antialiasMode);
    if(enableBgColor)
        viewer->changeBgColor(bgColor);
    else
        viewer->changeBgColor(QColor());
    changeTimerInterval(timerInterval);
}

void MainWindow::writeSettings()
{
    QSettings settings(INI_FILE_PATH, QSettings::IniFormat);
    settings.setValue("geometry", saveGeometry());
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
//    if (event->mimeData()->hasFormat("text/uri-list")) {
//        event->acceptProposedAction();
//    }

    const QMimeData *mimeData = event->mimeData();
    if (event->mimeData()->hasUrls()){
        QList<QUrl> urlList(mimeData->urls());
        QFileInfo fileInfo;
        for (int i = 0; i < urlList.size(); ++i) {
            fileInfo.setFile(urlList.at(i).toLocalFile());
            if(fileInfo.isFile()){
//                    && FORMAT_LIST.contains(fileInfo.suffix().toLower())){
                event->acceptProposedAction();
                break;
            }
        }
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList(mimeData->urls());
        QStringList fileList;
        for(int size = urlList.size(), i=0; i < size; ++i)
            fileList.append(urlList.at(i).toLocalFile());
        fileList = ToolKit::getFilesExist(fileList);   ///
        if(!fileList.empty())
            viewer->openFiles(fileList);
    }

    event->acceptProposedAction();
}

void MainWindow::changeFullScreen()
{
//    setWindowState( windowState() ^ Qt::WindowFullScreen );
    if(isFullScreen()){
//        statusBar()->show();
        if(WasMaximized)
            showMaximized();
        else
            showNormal();

//        updateGeometry();
//        move(pos());
    }else{
        WasMaximized = isMaximized();    //! only for windows?
//        statusBar()->hide();
        showFullScreen();
    }
}

void MainWindow::showAttribute()
{
    if(viewer->hasPicture() || viewer->hasFile()){
#ifdef Q_WS_WIN
        QWhatsThis::showText(QCursor::pos(), viewer->attribute(), this);
#else//(Q_WS_X11)
        QMessageBox::information(this, tr("Property"), viewer->attribute());
#endif
        //    setToolTip(viewer->attribute());
        //    QToolTip::showText(QCursor::pos(), viewer->attribute(), this);
    }
}

void MainWindow::switchSlideShow()// if other commend when slide show??
{
    if(!viewer->hasFile() && !slideTimer->isActive())  ////for setMyWindowTitle() call
        return;

    if(slideTimer->isActive()){
        slideTimer->stop();
        slideAction->setIcon(QIcon(":/res/Play.png"));
        slideAction->setText(tr("Auto Play"));
        playButton->setIcon(QIcon(":/res/Play.png"));
        playButton->setToolTip(tr("Auto Play"));
        openButton->setEnabled(true);
        deleteButton->setEnabled(viewer->hasFile());    ///
    }else{
        slideTimer->start();
        slideAction->setIcon(QIcon(":/res/Stop.png"));
        slideAction->setText(tr("Stop Play"));
        playButton->setIcon(QIcon(":/res/Stop.png"));
        playButton->setToolTip(tr("Stop Play"));
        openButton->setEnabled(false);
        deleteButton->setEnabled(false);
    }
}

void MainWindow::initContextMenu()
{
//    QStyle *st = style();

    QAction *settingAction = new QAction(QIcon(":/res/Setting.png"),
                                         tr("&Setting"), this);
    connect(settingAction, SIGNAL(triggered()), SLOT(setting()));

    QAction *aboutAction = new QAction(tr("&About"), this);
    connect(aboutAction, SIGNAL(triggered()), SLOT(about()));

    QAction *closeAction = new QAction(tr("Close"), this);
    connect(closeAction, SIGNAL(triggered()), SLOT(close()));

    openAction = new QAction(QIcon(":/res/Open.png"),
//                st->standardIcon(QStyle::SP_DialogOpenButton),
                tr("&Open"), this);
    connect(openAction, SIGNAL(triggered()), SLOT(openFile()));

    slideAction = new QAction(QIcon(":/res/Play.png"),
                              tr("Auto Play"), this);
    connect(slideAction, SIGNAL(triggered()), SLOT(switchSlideShow()));

    rotateLeftAction = new QAction(QIcon(":/res/Undo.png"),
                                   tr("Rotate &Left"), this);
    connect(rotateLeftAction, SIGNAL(triggered()), SLOT(rotateLeft()));

    rotateRightAction = new QAction(QIcon(":/res/Redo.png"),
                                    tr("Rotate &Right"), this);
    connect(rotateRightAction, SIGNAL(triggered()), SLOT(rotateRight()));

    mirrorHAction = new QAction(tr("Mirrored &Horizontal"), this);
    connect(mirrorHAction, SIGNAL(triggered()), SLOT(mirrorHorizontal()));

    mirrorVAction = new QAction(tr("Mirrored &Vertical"), this);
    connect(mirrorVAction, SIGNAL(triggered()), SLOT(mirrorVertical()));

    copyAction = new QAction(tr("&Copy to clipboard"), this);
    connect(copyAction, SIGNAL(triggered()), SLOT(copyToClipboard()));

    attributeAction = new QAction(QIcon(":/res/Info.png"),
                                  tr("&Property"), this);
    connect(attributeAction, SIGNAL(triggered()), SLOT(showAttribute()));

    deleteAction = new QAction(QIcon(":/res/Delete.png"),
//                st->standardIcon(QStyle::SP_DialogCloseButton),
                tr("&Delete"), this);
    connect(deleteAction, SIGNAL(triggered()), SLOT(deleteFileAsk()));

    contextMenu = new QMenu(this);
    contextMenu->addAction(openAction);
    contextMenu->addAction(slideAction);
    contextMenu->addSeparator();
    contextMenu->addAction(rotateLeftAction);
    contextMenu->addAction(rotateRightAction);
    contextMenu->addAction(mirrorHAction);
    contextMenu->addAction(mirrorVAction);
    contextMenu->addSeparator();
    contextMenu->addAction(copyAction);
    contextMenu->addAction(deleteAction);
    contextMenu->addAction(attributeAction);
    contextMenu->addSeparator();
    contextMenu->addAction(settingAction);
    contextMenu->addAction(aboutAction);
    contextMenu->addSeparator();
    contextMenu->addAction(closeAction);
}

void MainWindow::showContextMenu(const QPoint &pos)
{
    bool hasPixmap = viewer->hasPicture();
    bool has_file = viewer->hasFile();
    //! QMenu is top-level window, no matter hidden or shown.
    bool notSliding = (!slideTimer->isActive());

    openAction->setEnabled(notSliding);
    slideAction->setEnabled(has_file);
    rotateLeftAction->setEnabled(hasPixmap);
    rotateRightAction->setEnabled(hasPixmap);
    mirrorHAction->setEnabled(hasPixmap);
    mirrorVAction->setEnabled(hasPixmap);
    copyAction->setEnabled(hasPixmap);
    deleteAction->setEnabled(has_file && notSliding);
    attributeAction->setEnabled(hasPixmap || has_file); //

    contextMenu->popup(pos);
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
//    const int CommandCount = 18;
//    QStringList CommandList;
//    CommandList << "openFile" << "changeFullScreen" << "switchSlideShow"
//                << "showAttribute" << "setting" << "about"
//                << "nextPic" << "prePic" << "closeWindow"
//                << "rotateLeft" << "rotateRight" << "mirrorHorizontal"
//                << "mirrorVertical" << "switchGifPause" << "nextGifFrame"
//                << "copyToClipboard" << "deleteFile" << "closeAllWindows";

//    void (MainWindow::*Func[CommandCount])() = {   //函数指针数组
//                &MainWindow::openFile,       &MainWindow::changeFullScreen,
//                &MainWindow::switchSlideShow,      &MainWindow::showAttribute,
//                &MainWindow::setting,        &MainWindow::about,
//                &MainWindow::nextPic,        &MainWindow::prePic,
//                &MainWindow::closeWindow,          &MainWindow::rotateLeft,
//                &MainWindow::rotateRight,    &MainWindow::mirrorHorizontal,
//                &MainWindow::mirrorVertical, &MainWindow::switchGifPause,
//                &MainWindow::nextGifFrame,   &MainWindow::copyToClipboard,
//                &MainWindow::deleteFile,     &MainWindow::closeAllWindows
//    };

    //(this->*Func[i])(end);  //执行相应指令对应的函数


//    qDebug() << "key code : " << QString::number(e->key(), 16);
//    qDebug() << "key text: " << e->text();
//    if(e->modifiers() & Qt::NoModifier)
//        qDebug() << "Qt::NoModifier";
//    if(e->modifiers() & Qt::ShiftModifier)
//        qDebug() << "Qt::ShiftModifier";
//    if(e->modifiers() & Qt::ControlModifier)
//        qDebug() << "Qt::ControlModifier";
//    if(e->modifiers() & Qt::AltModifier)
//        qDebug() << "Qt::AltModifier";
//    if(e->modifiers() & Qt::MetaModifier)
//        qDebug() << "Qt::MetaModifier";

    switch(e->key()){
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_PageDown:
    case Qt::Key_J:
        nextPic();
        e->accept();
        break;
    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_PageUp:
    case Qt::Key_K:
        prePic();
        e->accept();
        break;
    case Qt::Key_O:
    case Qt::Key_N:
        if(!slideTimer->isActive())
            openFile();
        e->accept();
        break;
    case Qt::Key_Escape:
    case Qt::Key_Q:
        close();        //! !
        e->accept();
        break;
    case Qt::Key_Equal:
    case Qt::Key_Plus:
        switch(e->modifiers()){
        case Qt::ShiftModifier:
            viewer->zoomIn(0.05);
            break;
        case Qt::ControlModifier:
            viewer->zoomIn(0.2);
            break;
        default:
            viewer->zoomIn(0.1);
            break;
        }
        e->accept();
        break;
    case Qt::Key_Minus:
    case Qt::Key_Underscore:
        switch(e->modifiers()){
        case Qt::ShiftModifier:
            viewer->zoomIn(-0.05);
            break;
        case Qt::ControlModifier:
            viewer->zoomIn(-0.2);
            break;
        default:
            viewer->zoomIn(-0.1);
            break;
        }
        e->accept();
        break;
    case Qt::Key_Return:
        changeFullScreen();
        e->accept();
        break;
    case Qt::Key_L:
        rotateLeft();
        e->accept();
        break;
    case Qt::Key_R:
        rotateRight();
        e->accept();
        break;
    case Qt::Key_H:
        mirrorHorizontal();
        e->accept();
        break;
    case Qt::Key_V:
        mirrorVertical();
        e->accept();
        break;
    case Qt::Key_I:
        showAttribute();
        e->accept();
        break;
    case Qt::Key_Delete:
    case Qt::Key_D:
        if(slideTimer->isActive())
            break;
        if(e->modifiers() == Qt::ControlModifier)
            deleteFileNoAsk();
        else
            deleteFileAsk();
        e->accept();
        break;
    case Qt::Key_C:
        if(e->modifiers() == Qt::ControlModifier){
            copyToClipboard();
            e->accept();
        }
        break;
    case Qt::Key_S:
        setting();
        e->accept();
        break;
    case Qt::Key_Space:
    case Qt::Key_Pause:
        switchGifPause();
        e->accept();
        break;
    case Qt::Key_F:
        nextGifFrame();
        e->accept();
        break;
    case Qt::Key_P:
        switchSlideShow();
        e->accept();
        break;
    default:
        QWidget::keyPressEvent(e);
        break;
    }

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents); //add:20121006
}
