Ez看图
=======

EZ看图是一个简单纯粹的图片浏览器，使用Qt4编写，开源跨平台，国际化支持。   
默认支持jpg, bmp, gif(包括动画), png, jpeg, ico, svg(包括动画),  mng(包括动画), webp, psd(部分支持), psb(部分支持),pbm, pgm, ppm, tif, tiff, xbm, xpm等多种图片格式。     

***下载可执行文件:*** [v3.0正式版，2013/3/31](http://goo.gl/n8hvO)

作者
--------------------
    三生石 (huangezhao@gmail.com)	
    
特点：
--------------------
    1. 采用统一绘图机制，可以旋转、镜像、缩放、拖动图片，包括动态图片(gif, mng, svg);
    2. 图片缩放显示时使用抗锯齿功能;
    3. 图片缩放大于窗体时，鼠标左键快速拖动图片，释放鼠标后会依惯性滑动;
    4. 预读下一张图片，以提高图片切换时的载入速度;
    5. 缓存浏览过的图片，以加快反向浏览时的速度;
    6. 图片属性中支持查看Exif信息;
    7. 根据Exif中的方向信息自动旋转图片;
    8. 对于含多个帧的图片(gif, mng, ico, mng, svg等)，按F键可以循环切换各个帧;
    9. 鼠标停在窗口右上角一定时间后显示图片属性(适合全屏观看时查看属性);
    10. 可直接从资源管理器拖动一个或多个图片到看图器;
    11. 自动放映/暂停功能;
    12. 支持复制图片内容到剪贴板;
    13. 丰富的自定义按键功能。

编译
--------------------
1. 使用 `git clone` 代码库到你的电脑上。
2. 运行 `qmake`
3. 运行 `make`

注意: 

1. 如果想支持webp格式，可以使用 [qt_webp](http://github.com/cor3ntin/qt_webp).
2. 如果想支持psd格式，可以使用 [libqpsd](http://github.com/Code-ReaQtor/libqpsd).
    
    
EzViewer is a simple image viewer, programmed by Qt4.   
It's open-source and cross-platform software, with full international support.   
It supports jpg, bmp, gif, png, jpeg, ico, svg, webp, psd(partly), pbm, pgm, ppm, tif, tiff, xbm, xpm, and many other image formats.

***Get the binary:*** [v3.0，2013/3/31](http://goo.gl/n8hvO)

Author
--------------------
	Yuezhao Huang (huangezhao@gmail.com)	
    
Features:
--------------------
	1. Unified display mechanism, support rotate, mirror, zoom in/out, drag the image, including animation image.
	2. Use anti-aliasing when zoom in/out the pictures.
	3. When image scaling larger than the form, the picture will sliding inertially when the left mouse button quickly drag a picture and release then.
	4. Pre-reading next picture, for improving file change performance.
	5. Cache image has viewed, to improve the performance of browser back.
	6. Support Exif informance in picture property.
        7. Auto rotate image according EXIF orientation tag.
        8. Frame step by press F if the image has more than one frame.
        9. If the cursor stay on the top-right, will show the image property.
        10. Support dragging pictures directly from Windows Explorer to this viewer.
        11. Auto play/stop play.
        12. Support to copy the image content to the clipboard.
        13. Support for shortcut keys.

Build
--------------------
1. Clone the project
2. qmake
3. make

NOTE: 

1. To get webp format support, use [qt_webp](http://github.com/cor3ntin/qt_webp).
2. To get psd format support, use [libqpsd](http://github.com/Code-ReaQtor/libqpsd).
