Ez看图
作者：三生石

EZ看图是一个简单纯粹的图片浏览器，使用Qt4编写，开源跨平台，国际化支持。
默认支持jpg, bmp, gif(包括动画), png, jpeg, ico, svg(包括动画),  mng(包括动画),
webp, psd(部分支持), pbm, pgm, ppm, tif, tiff, xbm, xpm等多种图片格式。

特点有：
1、采用统一绘图机制，可以旋转、镜像、缩放、拖动图片，包括动态图片(gif, mng, svg);
2、图片缩放显示时使用抗锯齿功能;
3、图片缩放大于窗体时，鼠标左键快速拖动图片，释放鼠标后会依惯性滑动;
4、预读下一张图片，以提高图片切换时的载入速度;
5、缓存浏览过的图片，以加快反向浏览时的速度;
6、图片属性中支持查看Exif信息;
7、对于含多个帧的图片(gif, mng, ico, mng, svg等)，按F键可以循环切换各个帧;
8、鼠标停在窗口右上角一定时间后显示图片属性(适合全屏观看时查看属性);
9、可直接从资源管理器拖动一个或多个图片到看图器;
10、自动放映/暂停功能;
11、支持复制图片内容到剪贴板;
12、支持自定义快捷键。

默认快捷键如下：
    导航键上下左右、PageUp/PageDown以及J/K：切换图片；
    O：打开图片对话框；
    Q或Esc：关闭窗口；
    +/-：缩放图片；注意按住Ctrl时可以快速缩放，按住Shift时可以慢速缩放。
    回车键：全屏显示；
    I：显示图片信息；
    D或Del：删除图片对话框；注意按住Ctrl时不会出现对话框，而是直接删除图片；
    Ctrl+C：复制图片内容到剪贴板；
    L/R：向左/右旋转90度；
    H/V：水平/竖直镜像图片；
    S：设置；
    空格键/Pause：gif动画暂停/继续。
    F：gif动画暂停/步进。
    P：自动播放/暂停功能。

用法：
Windows XP/7:运行 EzViewer.exe
Linux用户需下载源码自行编译。



EzViewer is a simple image viewer, programmed by Qt4.
It's open-source and cross-platform software, with full international support.
It supports jpg, bmp, gif, png, jpeg, ico, svg, webp, psd(partly),
pbm, pgm, ppm, tif, tiff, xbm, xpm, and many other image formats.

Features:
1, Unified display mechanism, support rotate, mirror, zoom in/out, drag the image, including animation image.
2, Use anti-aliasing when zoom in/out the pictures.
3, When image scaling larger than the form, the picture will sliding inertially when
the left mouse button quickly drag a picture and release then.
4, Pre-reading next picture, for improving file change performance.
5, Cache image has viewed, to improve the performance of browser back.
6, Support Exif informance in picture property.
7, Frame step by press F if the image has more than one frame.
8, If the cursor stay on the top-right, will show the image property.
9, Support dragging pictures directly from Windows Explorer to this viewer.
10, Auto play/stop play.
11, Support to copy the image content to the clipboard.
12, Support for shortcut keys:
    Navigation keys Up,Down,Left,Right, PageUp / PageDown and J / K: Switch pictures;
    O: Open select picture dialog;
    Q or Esc: close the window;
    + / -: zoom in/out the picture; Note that you can quickly zoom in/out when hold down the Ctrl, or slowly zoom when hold down the Shift.
    Enter: full-screen;
    I: show the picture information;
    D or Del: show the delete picture dialog; Note that no dialog appears when you hold down the Ctrl, it will delete a picture directly ;
    Ctrl+C: Copy the image content to the clipboard;
    L / R: rotation the picture to left or right by 90 degrees;
    H / V:  mirror the images by horizontal / vertical;
    S: show setting dialog;
    Space / Pause:  pause / continue when display animation.
    F:  frame step when display gif animation.
    P：auto play/stop play.

Usage:
Windows XP / 7: run EzViewer.exe
Linux: download source code and compile it
