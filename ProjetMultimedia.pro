#-------------------------------------------------
#
# Project created by QtCreator 2016-05-12T15:36:07
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ProjetMultimedia
TEMPLATE = app

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.11
SOURCES += main.cpp\
        mainwindow.cpp \
    myglwidget.cpp \
    game.cpp

HEADERS  += mainwindow.h \
    myglwidget.h \
    game.h

FORMS    += mainwindow.ui
INCLUDEPATH += /usr/local/include

LIBS +=-L/usr/local/lib \
     -lopencv_core \
     -lopencv_imgproc \
     -lopencv_features2d \
     -lopencv_highgui \
    -lopencv_flann \
    -lopencv_video \

RESOURCES += \
    ressources.qrc


mac: LIBS += -framework GLUT
else:unix|win32: LIBS += -lGLUT
