#-------------------------------------------------
#
# Project created by QtCreator 2014-09-07T17:32:13
#
#-------------------------------------------------

QT       -= core gui
CONFIG   -= app_bundle
TARGET = shuf-t

TEMPLATE = app

SOURCES += utils.cpp \
    main.cpp \
    shuf-t.cpp \
    io_buf.cc

HEADERS += \
    metadata.h \
    shuf-t.h \
    utils.h \
    io_buf.h \
    v_array.h \
    settings.h \
    SimpleOpt.h

QMAKE_LFLAGS_RPATH=
#win32-g++{
 #QMAKE_LFLAGS += -static # to remove libgcc_s_dw2-1.dll dependance if mingw is used
#}
