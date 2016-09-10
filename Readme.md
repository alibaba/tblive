
千牛主播（tblive）
====

tblive开源项目对应的产品是千牛主播，是一个独立的PC端主播工具，基于开源软件OBS Studio(https://github.com/jp9000/obs-studio)来修改定制，增强可用性，同时引入统一登陆sdk（UnifiedAuthSDK.dll），可以支持淘宝账号的验证以及免登url的拼接，能更好的支持淘系的二方webview页面。


## Windows下开发环境

### 安装VS2013 + Update4

### 安装 Qt5.6_vs2013
http://download.qt.io/official_releases/qt/5.6/5.6.0/qt-opensource-windows-x86-msvc2013-5.6.0.exe

并设置环境变量 QTDIR: `C:\Qt\Qt5.6.0\5.6\msvc2013`

### 安装qt-vs-addin
http://mirrors.ustc.edu.cn/qtproject/archive/vsaddin/qt-vs-addin-1.2.5.exe

VS2013中，QT5 - [Qt-option]，设置qt版本

### 安装cmake
https://cmake.org/files/v3.5/cmake-3.5.0-rc3-win32-x86.msi

并将cmake的bin路径添加到PATH

### 使用cmake-gui生成工程编译

设置code source：比如 `F:/github/tblive`

设置build目录：`F:/github/tblive/build`

点击'Configure'，然后选中COPY_DEPENDENCIES选项，再点击'Configure'，最后点击'Generate'

再用vs2013打开build目录下的obs-studio.sln，然后生成；生成的可执行程序在rundir目录下。


### 使用批处理编译

obs_win_build.bat



## Mac OS X 下开发环境
TODO

