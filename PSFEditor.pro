#-------------------------------------------------
#
# Project created by QtCreator 2017-07-24T06:29:58
#
#-------------------------------------------------

QT       += core gui
QT       -= opengl
LIBS -= "-framework OpenGL"

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PSFEditor
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_NO_STYLE_GTK

INCLUDEPATH += include/

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/qfontglypheditor.cpp \
    src/qglyphlistwidgetitemdelegate.cpp \
    src/psfutil.cpp \
    src/psf.cpp \
    src/dlgsymbinfo.cpp

HEADERS  += include/mainwindow.h \
    include/qfontglypheditor.h \
    include/qglyphlistwidgetitemdelegate.h \
    include/psfutil.h \
    include/psf.h \
    include/mini_utf8.h \
    include/dlgsymbinfo.h

FORMS    += ui/mainwindow.ui \
    ui/dlgsymbinfo.ui

RESOURCES += \
    rc/psfeditor.qrc
