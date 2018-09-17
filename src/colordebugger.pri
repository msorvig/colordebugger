INCLUDEPATH += $$PWD

include(colorconvert.pri)

QT += widgets charts

HEADERS += \
    $$PWD/chromaticitydiagram.h \

SOURCES += \
    $$PWD/chromaticitydiagram.cpp \
    $$PWD/chromaticitydiagram_data.cpp
