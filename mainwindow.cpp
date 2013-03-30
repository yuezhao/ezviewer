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
#include "actionmanager.h"
#include "settingdialog.h"

#include <QtGui>

#define GET_SCRIPT(x) #x
#define SPLIT(x) &x, #x

const int SWITCH_FRAME_WIDTH = 90;
const int BUTTOM_FRAME_HEIGHT = 60;
const int ATTRIBUTE_RECT_WIDTH = 100;
const int ATTRIBUTE_RECT_HEIGHT = 100;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    viewer = new PicManager(this);
    setCentralWidget(viewer);
    viewer->installEventFilter(this);

    wasMaximized = false;
    slideInterval = 4000;
    slideTimer = new QTimer(this);
    slideTimer->setInterval(slideInterval);
    //! if file numbers is 0 or 1, stop timer???......................
    connect(slideTimer, SIGNAL(timeout()), viewer, SLOT(nextPic()));

    connect(viewer, SIGNAL(imageChanged(QString)), SLOT(imageChanged(QString)));
    connect(viewer, SIGNAL(siteChange(QPoint)), SLOT(moveWindow(QPoint)));

    initContextMenu();
    initButtomBar();
    initSwitchFrame();

    registerAllFunction();

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

void MainWindow::imageChanged(const QString &fileName)
{
    bool hasFile = !fileName.isEmpty();
    bool hasPicture = viewer->hasPicture();

    setWindowTitle(hasFile
                   ? QString("%1 - %2").arg(fileName).arg(Global::ProjectName())
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
    viewer->setCacheNumber(Config::cacheValue());
    viewer->setPreReadingEnabled(Config::enablePreReading());
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
        if(wasMaximized)
            showMaximized();
        else
            showNormal();
    }else{
        wasMaximized = isMaximized();    //! only for windows?
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
    connect(preButton, SIGNAL(clicked()), viewer, SLOT(prePic()));
    connect(playButton, SIGNAL(clicked()), SLOT(switchSlideShow()));
    connect(nextButton, SIGNAL(clicked()), viewer, SLOT(nextPic()));
    connect(rotateLeftButton, SIGNAL(clicked()), viewer, SLOT(rotateLeft()));
    connect(rotateRightButton, SIGNAL(clicked()), viewer, SLOT(rotateRight()));
    connect(deleteButton, SIGNAL(clicked()), viewer, SLOT(deleteFileAsk()));
}

void MainWindow::initSwitchFrame()
{
    leftFrame = new FloatFrame(this);
    leftFrame->setFillBackground(false);
    leftFrame->setHideInterval(400);
    connect(leftFrame, SIGNAL(showContextMenu(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(leftFrame, SIGNAL(mouseClicked()), viewer, SLOT(prePic()));

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
    connect(rightFrame, SIGNAL(mouseClicked()), viewer, SLOT(nextPic()));

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
    QAction *settingAction = new QAction(QIcon(":/Setting.png"),
                                         tr("&Setting"), this);
    connect(settingAction, SIGNAL(triggered()), SLOT(setting()));

    QAction *aboutAction = new QAction(tr("&About"), this);
    connect(aboutAction, SIGNAL(triggered()), SLOT(about()));

    QAction *closeAction = new QAction(tr("&Quit"), this);
    connect(closeAction, SIGNAL(triggered()), SLOT(close()));

    openAction = new QAction(QIcon(":/Open.png"),
//                style()->standardIcon(QStyle::SP_DialogOpenButton),
                tr("&Open"), this);
    connect(openAction, SIGNAL(triggered()), SLOT(openFile()));

    slideAction = new QAction(QIcon(":/Play.png"), tr("Auto Play"), this);
    connect(slideAction, SIGNAL(triggered()), SLOT(switchSlideShow()));

    rotateLeftAction = new QAction(QIcon(":/Undo.png"), tr("Rotate &Left"), this);
    connect(rotateLeftAction, SIGNAL(triggered()), viewer, SLOT(rotateLeft()));

    rotateRightAction = new QAction(QIcon(":/Redo.png"),
                                    tr("Rotate &Right"), this);
    connect(rotateRightAction, SIGNAL(triggered()), viewer, SLOT(rotateRight()));

    mirrorHAction = new QAction(tr("Mirrored &Horizontal"), this);
    connect(mirrorHAction, SIGNAL(triggered()), viewer, SLOT(mirrorHorizontal()));

    mirrorVAction = new QAction(tr("Mirrored &Vertical"), this);
    connect(mirrorVAction, SIGNAL(triggered()), viewer, SLOT(mirrorVertical()));

    copyAction = new QAction(tr("&Copy to clipboard"), this);
    connect(copyAction, SIGNAL(triggered()), viewer, SLOT(copyToClipboard()));

    attributeAction = new QAction(QIcon(":/Info.png"), tr("&Property"), this);
    connect(attributeAction, SIGNAL(triggered()), SLOT(showAttribute()));

    deleteAction = new QAction(QIcon(":/Delete.png"),
//                style()->standardIcon(QStyle::SP_DialogCloseButton),
                tr("&Delete"), this);
    connect(deleteAction, SIGNAL(triggered()), viewer, SLOT(deleteFileAsk()));

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

void MainWindow::registerAllFunction()
{
    ActionManager::registerFunction(tr("Open"),
                                    this, SPLIT(MainWindow::openFile));
    ActionManager::registerFunction(tr("Full Screen"),
                                    this, SPLIT(MainWindow::changeFullScreen));
    ActionManager::registerFunction(tr("Auto Play/Stop"),
                                    this, SPLIT(MainWindow::switchSlideShow));
    ActionManager::registerFunction(tr("Property"),
                                    this, SPLIT(MainWindow::showAttribute));
    ActionManager::registerFunction(tr("Setting"),
                                    this, SPLIT(MainWindow::setting));
    ActionManager::registerFunction(tr("About"),
                                    this, SPLIT(MainWindow::about));
    ActionManager::registerFunction(tr("Quit"),
                                    this, SPLIT(MainWindow::close));

    ActionManager::registerFunction(tr("Next Picture"),
                                    viewer, SPLIT(PicManager::nextPic));
    ActionManager::registerFunction(tr("Previous Picture"),
                                    viewer, SPLIT(PicManager::prePic));
    ActionManager::registerFunction(tr("Rotate Left"),
                                    viewer, SPLIT(PicManager::rotateLeft));
    ActionManager::registerFunction(tr("Rotate Right"),
                                    viewer, SPLIT(PicManager::rotateRight));
    ActionManager::registerFunction(tr("Mirrored Horizontal"),
                                    viewer, SPLIT(PicManager::mirrorHorizontal));
    ActionManager::registerFunction(tr("Mirrored Vertical"),
                                    viewer, SPLIT(PicManager::mirrorVertical));
    ActionManager::registerFunction(tr("Animation Play/Pause"),
                                    viewer, SPLIT(PicManager::switchAnimationState));
    ActionManager::registerFunction(tr("Frame Step"),
                                    viewer, SPLIT(PicManager::nextAnimationFrame));
    ActionManager::registerFunction(tr("Copy to clipboard"),
                                    viewer, SPLIT(PicManager::copyToClipboard));
    ActionManager::registerFunction(tr("Delete"),
                                    viewer, SPLIT(PicManager::deleteFileAsk));
    ActionManager::registerFunction(tr("Delete Without Notification"),
                                    viewer, SPLIT(PicManager::deleteFileNoAsk));

    ActionManager::registerFunction(tr("Zoom In"),
                                    viewer, SPLIT(PicManager::zoomIn), 0.1);
    ActionManager::registerFunction(tr("Zoom In (Slow)"),
                                    viewer, &PicManager::zoomIn,
                                    "PicManager::zoomIn0.05", 0.05);
    ActionManager::registerFunction(tr("Zoom in (Fast)"),
                                    viewer, &PicManager::zoomIn,
                                    "PicManager::zoomIn0.2", 0.2);
    ActionManager::registerFunction(tr("Zoom Out"),
                                    viewer, &PicManager::zoomIn,
                                    "PicManager::zoomIn-0.1", -0.1);
    ActionManager::registerFunction(tr("Zoom Out (Slow)"),
                                    viewer, &PicManager::zoomIn,
                                    "PicManager::zoomIn-0.05", -0.05);
    ActionManager::registerFunction(tr("Zoom Out (Fast)"),
                                    viewer, &PicManager::zoomIn,
                                    "PicManager::zoomIn-0.2", -0.2);
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    QKeySequence keys(e->modifiers() + e->key());
    qDebug("key press: %s", qPrintable(keys.toString()));

    if (slideTimer->isActive()) {
        QString action = ActionManager::getMatchAction(keys.toString());
        if (action == GET_SCRIPT(MainWindow::openFile)
                || action == GET_SCRIPT(PicManager::deleteFileNoAsk)
                || action == GET_SCRIPT(PicManager::deleteFileAsk) )
            return;
    }

    if(ActionManager::run(keys.toString())) {
        e->accept();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        return;
    }

    QWidget::keyPressEvent(e);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents); //add:20121006
}
