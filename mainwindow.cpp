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

#include "mainwindow.h"

#include "config.h"
#include "global.h"
#include "contralbar.h"
#include "floatframe.h"
#include "toolkit.h"
#include "tooltip.h"
#include "shortcut.h"
#include "settingdialog.h"

#include <QtGui>


const int SWITCH_FRAME_WIDTH = 90;
const int BUTTOM_FRAME_HEIGHT = 60;
const int ATTRIBUTE_RECT_WIDTH = 100;
const int ATTRIBUTE_RECT_HEIGHT = 100;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    WasMaximized = false;
    slideInterval = 4000;
    slideTimer = new QTimer(this);
    slideTimer->setInterval(slideInterval);
    //! if file numbers is 0 or 1, stop timer???......................
    connect(slideTimer, SIGNAL(timeout()), SLOT(nextPic()));

    viewer = new PicManager(this);
    setCentralWidget(viewer);
    viewer->installEventFilter(this);

    connect(viewer, SIGNAL(imageChanged(QString)), SLOT(imageChanged(QString)));
    connect(viewer, SIGNAL(siteChange(QPoint)), SLOT(moveWindow(QPoint)));

    initContextMenu();
    initButtomBar();
    initSwitchFrame();
    initShortCutTable();

    resize(Config::WindowFitSize);
    readSettings();
    Config::insertConfigWatcher(this, SLOT(applyConfig()));

    imageChanged();
    setWindowIcon(QIcon(":/appIcon"));
    setAcceptDrops(true);   //! !!
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Config::cancelConfigWatcher(this);
    if(!isFullScreen())
        writeSettings();
    event->accept();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About %1").arg(Global::ProjectName()),
                       Global::AboutInfo());
}

void MainWindow::setting()
{
    SettingsDialog dlg(this);
    dlg.exec();
}

void MainWindow::imageChanged(const QString &title)
{
    bool hasFile = !title.isEmpty();
    bool hasPicture = viewer->hasPicture();

    setWindowTitle(hasFile ?
                       QString("%1 - %2").arg(title).arg(Global::ProjectName())
                     : Global::ProjectName());
    ToolTip::hideText();
    QWhatsThis::hideText();

    preButton->setEnabled(hasFile);
    playButton->setEnabled(hasFile);
    nextButton->setEnabled(hasFile);
    leftFrame->set_enabled(hasFile);    ///
    rightFrame->set_enabled(hasFile);   ///
    rotateLeftButton->setEnabled(hasPicture);
    rotateRightButton->setEnabled(hasPicture);
    deleteButton->setEnabled(!slideTimer->isActive() && hasFile);   ////

    if(!hasFile && slideTimer->isActive())  // TODO: if only one file, stop slide show.
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
                tr("Images (%1);;All Files (*)").arg(Config::supportFormats()));
    if (!fileName.isEmpty())
        viewer->openFile(fileName);
}

void MainWindow::readSettings()
{
    applyConfig();    ///
    restoreGeometry(Config::lastGeometry());
}

void MainWindow::applyConfig()
{
    viewer->changeAntialiasMode(Config::antialiasMode());
    if(Config::enableBgColor())
        viewer->changeBgColor(Config::bgColor());
    else
        viewer->changeBgColor(QColor());
    changeTimerInterval(Config::timerInterval());
    ImageFactory::setCacheNumber(Config::cacheValue());
    ImageFactory::setPreReadingEnabled(Config::enablePreReading());
}

void MainWindow::writeSettings()
{
    Config::setLastGeometry(saveGeometry());
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
        if(WasMaximized)
            showMaximized();
        else
            showNormal();
    }else{
        WasMaximized = isMaximized();    //! only for windows?
        showFullScreen();
    }
}

void MainWindow::showAttribute()
{
    if(viewer->hasPicture() || viewer->hasFile()){
#ifndef QT_NO_WHATSTHIS
        QWhatsThis::showText(QCursor::pos(), viewer->attribute(), this);
#else
        QMessageBox::information(this, tr("Property"), viewer->attribute());
#endif
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == viewer) {
        switch(event->type()) {
        case QEvent::ToolTip:
        {
            QHelpEvent* e = static_cast<QHelpEvent*>(event);
            if (attributeRect.contains(e->pos())){
                QString attribute = viewer->attribute();
                if(!attribute.isEmpty())
                    ToolTip::showText(e->globalPos(),
                                      attribute.prepend("<b>").append("</b>"),
                                      false, 0.8);
            }
            return true;
        }
        case QEvent::MouseButtonDblClick:
        {
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            if(e->button() & Qt::LeftButton)
                changeFullScreen();
            return true;
        }
        case QEvent::ContextMenu:
        {
            QContextMenuEvent *e = static_cast<QContextMenuEvent*>(event);
            showContextMenu(e->globalPos());
            return true;
        }
        default:
            break;
        }
    }
    return false;
}

void MainWindow::switchSlideShow()// if other commend when slide show??
{
    if(!viewer->hasFile() && !slideTimer->isActive())  ////for setMyWindowTitle() call
        return;

    if(slideTimer->isActive()){
        slideTimer->stop();
        slideAction->setIcon(QIcon(":/Play.png"));
        slideAction->setText(tr("Auto Play"));
        playButton->setIcon(QIcon(":/Play.png"));
        playButton->setToolTip(tr("Auto Play"));
        openButton->setEnabled(true);
        deleteButton->setEnabled(viewer->hasFile());    ///
    }else{
        slideTimer->start();
        slideAction->setIcon(QIcon(":/Stop.png"));
        slideAction->setText(tr("Stop Play"));
        playButton->setIcon(QIcon(":/Stop.png"));
        playButton->setToolTip(tr("Stop Play"));
        openButton->setEnabled(false);
        deleteButton->setEnabled(false);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    buttomFrame->resize(width(), BUTTOM_FRAME_HEIGHT);
    buttomFrame->move(0, height() - BUTTOM_FRAME_HEIGHT);
    leftFrame->resize(SWITCH_FRAME_WIDTH, height() - BUTTOM_FRAME_HEIGHT - ATTRIBUTE_RECT_HEIGHT);
    leftFrame->move(0, ATTRIBUTE_RECT_HEIGHT);
    rightFrame->resize(SWITCH_FRAME_WIDTH, height() - BUTTOM_FRAME_HEIGHT - ATTRIBUTE_RECT_HEIGHT);
    rightFrame->move(width() - SWITCH_FRAME_WIDTH, ATTRIBUTE_RECT_HEIGHT);

    attributeRect = QRect(width() - ATTRIBUTE_RECT_WIDTH, 0, ATTRIBUTE_RECT_WIDTH, ATTRIBUTE_RECT_HEIGHT);

    QWidget::resizeEvent(event);
}

void MainWindow::initButtomBar()
{
    buttomFrame = new FloatFrame(this);
    connect(buttomFrame, SIGNAL(showContextMenu(QPoint)),
            SLOT(showContextMenu(QPoint)));
    connect(buttomFrame, SIGNAL(mouseDoubleClick()),
            SLOT(changeFullScreen()));
    ///set all the button's focous policy to Qt::NoFocous in 'ui' file.

    contralBar = new ContralBar(buttomFrame);
    buttomFrame->addWidget(contralBar);

    QHBoxLayout *hlayout = new QHBoxLayout(buttomFrame);
    hlayout->setContentsMargins(0,0,0,0);   ///qframe's layout margis default is not 0.
    hlayout->setAlignment(Qt::AlignCenter);
    hlayout->addWidget(contralBar);
    buttomFrame->setLayout(hlayout);

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
    leftFrame->setFillBackground(false);
    leftFrame->setHideInterval(400);
    connect(leftFrame, SIGNAL(showContextMenu(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(leftFrame, SIGNAL(mouseClicked()), SLOT(prePic()));

    QLabel *lb = new QLabel(leftFrame);
    lb->setPixmap(QPixmap(":/Left2.png"));
    leftFrame->addWidget(lb);

    QHBoxLayout *hlayout = new QHBoxLayout(leftFrame);
    hlayout->setAlignment(Qt::AlignCenter);
    hlayout->addWidget(lb);
    leftFrame->setLayout(hlayout);


    rightFrame = new FloatFrame(this);
    rightFrame->setFillBackground(false);
    rightFrame->setHideInterval(400);
    connect(rightFrame, SIGNAL(showContextMenu(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(rightFrame, SIGNAL(mouseClicked()), SLOT(nextPic()));

    lb = new QLabel(rightFrame);
    lb->setPixmap(QPixmap(":/Right2.png"));
    rightFrame->addWidget(lb);

    hlayout = new QHBoxLayout(rightFrame);
    hlayout->setAlignment(Qt::AlignCenter);
    hlayout->addWidget(lb);
    rightFrame->setLayout(hlayout);
}

void MainWindow::initContextMenu()
{
//    QStyle *st = style();

    QAction *settingAction = new QAction(QIcon(":/Setting.png"),
                                         tr("&Setting"), this);
    connect(settingAction, SIGNAL(triggered()), SLOT(setting()));

    QAction *aboutAction = new QAction(tr("&About"), this);
    connect(aboutAction, SIGNAL(triggered()), SLOT(about()));

    QAction *closeAction = new QAction(tr("Close"), this);
    connect(closeAction, SIGNAL(triggered()), SLOT(close()));

    openAction = new QAction(QIcon(":/Open.png"),
//                st->standardIcon(QStyle::SP_DialogOpenButton),
                tr("&Open"), this);
    connect(openAction, SIGNAL(triggered()), SLOT(openFile()));

    slideAction = new QAction(QIcon(":/Play.png"),
                              tr("Auto Play"), this);
    connect(slideAction, SIGNAL(triggered()), SLOT(switchSlideShow()));

    rotateLeftAction = new QAction(QIcon(":/Undo.png"),
                                   tr("Rotate &Left"), this);
    connect(rotateLeftAction, SIGNAL(triggered()), SLOT(rotateLeft()));

    rotateRightAction = new QAction(QIcon(":/Redo.png"),
                                    tr("Rotate &Right"), this);
    connect(rotateRightAction, SIGNAL(triggered()), SLOT(rotateRight()));

    mirrorHAction = new QAction(tr("Mirrored &Horizontal"), this);
    connect(mirrorHAction, SIGNAL(triggered()), SLOT(mirrorHorizontal()));

    mirrorVAction = new QAction(tr("Mirrored &Vertical"), this);
    connect(mirrorVAction, SIGNAL(triggered()), SLOT(mirrorVertical()));

    copyAction = new QAction(tr("&Copy to clipboard"), this);
    connect(copyAction, SIGNAL(triggered()), SLOT(copyToClipboard()));

    attributeAction = new QAction(QIcon(":/Info.png"),
                                  tr("&Property"), this);
    connect(attributeAction, SIGNAL(triggered()), SLOT(showAttribute()));

    deleteAction = new QAction(QIcon(":/Delete.png"),
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


void MainWindow::parseCmd(QStringList args)
{
    args.removeFirst(); // remove name of executable
    if(!args.empty()){
        viewer->openFiles(args);
    }else{
        if(Config::showDialog()) // show dialog while launch.
            openFile();
    }
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

void MainWindow::initShortCutTable()
{
    shortCutTable.append(new ShortcutImpl<MainWindow>(tr("openFile"),
                     this, &MainWindow::openFile));
    shortCutTable.append(new ShortcutImpl<MainWindow>(tr("changeFullScreen"),
                     this, &MainWindow::changeFullScreen));
    shortCutTable.append(new ShortcutImpl<MainWindow>(tr("switchSlideShow"),
                     this, &MainWindow::switchSlideShow));
    shortCutTable.append(new ShortcutImpl<MainWindow>(tr("showAttribute"),
                     this, &MainWindow::showAttribute));
    shortCutTable.append(new ShortcutImpl<MainWindow>(tr("setting"),
                     this, &MainWindow::setting));
    shortCutTable.append(new ShortcutImpl<MainWindow>(tr("about"),
                     this, &MainWindow::about));
    shortCutTable.append(new ShortcutImpl<MainWindow>(tr("nextPic"),
                     this, &MainWindow::nextPic));
    shortCutTable.append(new ShortcutImpl<MainWindow>(tr("prePic"),
                     this, &MainWindow::prePic));
    shortCutTable.append(new ShortcutImpl<MainWindow, bool>(tr("close"),
                     this, &MainWindow::close));
    shortCutTable.append(new ShortcutImpl<MainWindow>(tr("rotateLeft"),
                     this, &MainWindow::rotateLeft));
    shortCutTable.append(new ShortcutImpl<MainWindow>(tr("rotateRight"),
                     this, &MainWindow::rotateRight));
    shortCutTable.append(new ShortcutImpl<MainWindow>(tr("mirrorHorizontal"),
                     this, &MainWindow::mirrorHorizontal));
    shortCutTable.append(new ShortcutImpl<MainWindow>(tr("mirrorVertical"),
                     this, &MainWindow::mirrorVertical));
    shortCutTable.append(new ShortcutImpl<MainWindow>(tr("switchAnimationState"),
                     this, &MainWindow::switchAnimationState));
    shortCutTable.append(new ShortcutImpl<MainWindow>(tr("nextAnimationFrame"),
                     this, &MainWindow::nextAnimationFrame));
    shortCutTable.append(new ShortcutImpl<MainWindow>(tr("copyToClipboard"),
                     this, &MainWindow::copyToClipboard));
    shortCutTable.append(new ShortcutImpl<MainWindow>(tr("deleteFileAsk"),
                     this, &MainWindow::deleteFileAsk));

    shortCutMap.insert(QKeySequence(QKeySequence::Open).toString(), shortCutTable.at(0));
    shortCutMap.insert("J", shortCutTable.at(6));
    shortCutMap.insert("K", shortCutTable.at(7));
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    QKeySequence keys(e->modifiers() + e->key());
    qDebug("key press: %s", qPrintable(keys.toString()));
    if (shortCutMap.contains(keys.toString()))
        shortCutMap.value(keys.toString())->run();

//    switch(e->key()){
//    case Qt::Key_Right:
//    case Qt::Key_Down:
//    case Qt::Key_PageDown:
//    case Qt::Key_J:
//        nextPic();
//        e->accept();
//        break;
//    case Qt::Key_Left:
//    case Qt::Key_Up:
//    case Qt::Key_PageUp:
//    case Qt::Key_K:
//        prePic();
//        e->accept();
//        break;
//    case Qt::Key_O:
//    case Qt::Key_N:
//        if(!slideTimer->isActive())
//            openFile();
//        e->accept();
//        break;
//    case Qt::Key_Escape:
//    case Qt::Key_Q:
//        close();        //! !
//        e->accept();
//        break;
//    case Qt::Key_Equal:
//    case Qt::Key_Plus:
//        switch(e->modifiers()){
//        case Qt::ShiftModifier:
//            viewer->zoomIn(0.05);
//            break;
//        case Qt::ControlModifier:
//            viewer->zoomIn(0.2);
//            break;
//        default:
//            viewer->zoomIn(0.1);
//            break;
//        }
//        e->accept();
//        break;
//    case Qt::Key_Minus:
//    case Qt::Key_Underscore:
//        switch(e->modifiers()){
//        case Qt::ShiftModifier:
//            viewer->zoomIn(-0.05);
//            break;
//        case Qt::ControlModifier:
//            viewer->zoomIn(-0.2);
//            break;
//        default:
//            viewer->zoomIn(-0.1);
//            break;
//        }
//        e->accept();
//        break;
//    case Qt::Key_Return:
//        changeFullScreen();
//        e->accept();
//        break;
//    case Qt::Key_L:
//        rotateLeft();
//        e->accept();
//        break;
//    case Qt::Key_R:
//        rotateRight();
//        e->accept();
//        break;
//    case Qt::Key_H:
//        mirrorHorizontal();
//        e->accept();
//        break;
//    case Qt::Key_V:
//        mirrorVertical();
//        e->accept();
//        break;
//    case Qt::Key_I:
//        showAttribute();
//        e->accept();
//        break;
//    case Qt::Key_Delete:
//    case Qt::Key_D:
//        if(slideTimer->isActive())
//            break;
//        if(e->modifiers() == Qt::ControlModifier)
//            deleteFileNoAsk();
//        else
//            deleteFileAsk();
//        e->accept();
//        break;
//    case Qt::Key_C:
//        if(e->modifiers() == Qt::ControlModifier){
//            copyToClipboard();
//            e->accept();
//        }
//        break;
//    case Qt::Key_S:
//        setting();
//        e->accept();
//        break;
//    case Qt::Key_Space:
//    case Qt::Key_Pause:
//        switchAnimationState();
//        e->accept();
//        break;
//    case Qt::Key_F:
//        nextAnimationFrame();
//        e->accept();
//        break;
//    case Qt::Key_P:
//        switchSlideShow();
//        e->accept();
//        break;
//    default:
//        QWidget::keyPressEvent(e);
//        break;
//    }

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents); //add:20121006
}
