TEMPLATE = app
TARGET = colorspaceviewer
INCLUDEPATH += .
QT += core gui widgets charts

# Input

HEADERS += \
    chromaticitydiagram.h \
    colorconvert.h \

SOURCES += \
    chromaticitydiagram.cpp \
    chromaticitydiagram_data.cpp \
    colorconvert.cpp

SOURCES += main.cpp
