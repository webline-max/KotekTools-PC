QT += core gui widgets network concurrent opengl openglwidgets
TARGET = KoTeK_Tools
CONFIG += debug_and_release no_batch
QMAKE_CXXFLAGS += /utf-8

# Пути к vcpkg (если нужен)
INCLUDEPATH += C:/vcpkg/installed/x64-windows/include
LIBS += -LC:/vcpkg/installed/x64-windows/lib



SOURCES += main.cpp \
    KoTeKTools.cpp \
    KoTeKTools_ui_setup.cpp \
    KoTeKTools_copy.cpp \
    KoTeKTools_convert.cpp \
    KoTeKTools_atmosphere.cpp \
    KoTeKTools_painting.cpp \
    KoTeKTools_extra.cpp \
    KoTeKTools_notify.cpp \
    KoTeKTools_helpers.cpp \
    KoTeKTools_drag.cpp \
    KoTeKTools_3D.cpp \
    KoTeKTools_texture.cpp \
    dff.cpp \
    bpc.cpp \
    atmosphere.cpp \
    painting.cpp \
    map.cpp \
    txd.cpp \
    btx_converter.cpp







HEADERS += KoTeKTools.h \
    dff.h \
    bpc.h \
    atmosphere.h \
    painting.h \
    map.h \
    listva_names.h \
    txd.h \
    stb_image_write.h \
    btx_converter.h







LIBS += -lzlib \
    -lopengl32 \
    -lzip