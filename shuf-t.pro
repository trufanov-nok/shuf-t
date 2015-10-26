#-------------------------------------------------
#
# Project created by QtCreator 2014-09-07T17:32:13
#
#-------------------------------------------------

QT       -= core
QT       -= gui

TARGET = shuf-t

CONFIG   -= console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS = -lboost_program_options

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
    settings.h


