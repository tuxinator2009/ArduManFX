TEMPLATE = app
TARGET = ArduManFX
DESTDIR = ..

DEFINES += QT_DEPRECATED_WARNINGS
#LIBS += -langelscript

QT += core gui serialport network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++11
CONFIG += debug

SOURCES += \
    ArduManFX.cpp \
    game.cpp \
    main.cpp \
    mainwindow.cpp \
    progress.cpp \
    tester.cpp
    #angelscript/scriptbuilder.cpp \
    #angelscript/scriptstdstring.cpp

HEADERS += \
    ArduManFX.h \
    game.h \
    mainwindow.h \
    progress.h \
    tester.h
    #angelscript/scriptbuilder.h \
    #angelscript/scriptstdstring.h

FORMS += \
    game.ui \
    mainwindow.ui \
    progress.ui \
    tester.ui
        
RESOURCES += resources.qrc
#RC_FILE = icon.rc
