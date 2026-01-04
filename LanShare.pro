# ==============================================================================
# 项目基础配置 - Qt 项目构建文件 (.pro)
# 适用框架：Qt 5/Qt 6
# 功能：配置项目依赖、编译选项、文件列表、输出目录等核心构建参数
# ==============================================================================
QT       += core gui network widgets concurrent sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    dbmanager.cpp \
    main.cpp \
    mainwindow.cpp \
    peermodel.cpp \
    transferthread.cpp
    RESOURCES += res.qrc

HEADERS += \
    dbmanager.h \
    mainwindow.h \
    peermodel.h \
    transferthread.h

FORMS += \
    mainwindow.ui

# 编译输出目录配置
DESTDIR = bin
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui
OBJECTS_DIR = build/obj

