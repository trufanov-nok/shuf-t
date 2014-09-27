#-------------------------------------------------
#
# Project created by QtCreator 2014-09-07T17:32:13
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = shuf-t
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += utils.cpp \
    main.cpp \
    shuf-t.cpp \
    qforkedtextstream.cpp

HEADERS += \
    metadata.h \
    shuf-t.h \
    utils.h \
    qforkedtextstream.h \
    qforkedtextstream_p.h
