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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "imageviewer.h"

class FloatFrame;
class QMenu;
class QAction;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    static   MainWindow *creatMainWindow(int sizeMode, int antialiasMode,
                    bool enableBgColor, const QColor &bgColor,
                    int timerInterval);

public slots:
    void openFile();
    static void openFile(const QStringList &fileList); //! static method
    void enableSelfAdaptive(bool enable) { viewer->enableSelfAdaptive(enable); }
    void changeAntialiasMode(int mode) { viewer->changeAntialiasMode(mode); }
    void changeBgColor(const QColor &color) { viewer->changeBgColor(color); }
    void changeTimerInterval(int sec) { slideTimer->setInterval(sec * 1000); }

protected slots:
    void keyPressEvent(QKeyEvent *e);
    void resizeEvent ( QResizeEvent * event );

    void moveWindow(const QPoint &change) { move(pos() + change); }
    void resizeWindow(const QSize &size)  { resize(size); }

    void setMyWindowTitle(const QString &title = QString::null);
    void showContextMenu(const QPoint &pos);

    void changeFullScreen();
    void showAttribute();
    void slideShow();
    void setting();
    void about();

    void nextPic()          { viewer->nextPic(); }
    void prePic()           { viewer->prePic(); }
    void rotateLeft()       { viewer->rotateLeft(); }
    void rotateRight()      { viewer->rotateRight(); }
    void mirrorHorizontal() { viewer->mirrorHorizontal(); }
    void mirrorVertical()   { viewer->mirrorVertical(); }
    void switchGifPause()   { viewer->switchGifPause(); }
    void nextGifFrame()     { viewer->nextGifFrame(); }
    void copyToClipboard()  { viewer->copyToClipboard(); }
    void deleteFileAsk()    { viewer->deleteFileAsk(); }
    void deleteFileNoAsk()  { viewer->deleteFileNoAsk(); }
//    void closeWindow()      { close(); }

private:
    void initContextMenu();
    void initTitleButton();
    void openFile(const QString &file) { viewer->openFile(file); }

private:
    ImageViewer *viewer;
    bool WasMaximized;
    QTimer *slideTimer;
    int slideInterval;//msec

    FloatFrame *titleButtonFF;

    QMenu *contextMenu;
    QAction *openAction;
    QAction *slideAction;
    QAction *rotateLeftAction;
    QAction *rotateRightAction;
    QAction *mirrorHAction;
    QAction *mirrorVAction;
    QAction *copyAction;
    QAction *deleteAction;
    QAction *attributeAction;
    QAction *exitAct;
};


#endif // MAINWINDOW_H
