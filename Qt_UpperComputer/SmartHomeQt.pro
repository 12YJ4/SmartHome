QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    AlarmDialog.cpp \
    DataBase.cpp \
    FaceThread.cpp \
    MainWindow.cpp \
    UdpService.cpp \
    main.cpp

HEADERS += \
    AlarmDialog.h \
    DataBase.h \
    FaceThread.h \
    MainWindow.h \
    UdpService.h

# OpenCV 3.4.5 (MinGW) —— 按本机实际安装路径修改
INCLUDEPATH += D:\opencv3.4.5_install\include
INCLUDEPATH += D:\opencv3.4.5_install\include\opencv
INCLUDEPATH += D:\opencv3.4.5_install\include\opencv2
LIBS += D:\opencv3.4.5_install\x64\mingw\lib\libopencv_*.a

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# 编译时自动把级联分类器复制到构建目录（exe 运行目录）
xmlcopy.files = $$PWD/haarcascade_frontalface_alt2.xml
xmlcopy.path = $$OUT_PWD
COPIES += xmlcopy

debugXml.files = $$PWD/haarcascade_frontalface_alt2.xml
debugXml.path = $$OUT_PWD/debug
COPIES += debugXml

releaseXml.files = $$PWD/haarcascade_frontalface_alt2.xml
releaseXml.path = $$OUT_PWD/release
COPIES += releaseXml
