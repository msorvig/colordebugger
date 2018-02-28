TEMPLATE = app

include(../../src/colordebugger.pri)

SOURCES += main.cpp

QT += widgets
OBJECTS_DIR = .obj
MOC_DIR = .moc

# copy images to build output
QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../images/Rose-sRGB.jpg) $$quote($$OUT_PWD) $$escape_expand(\n\t)
QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../images/Rose-AdobeRGB.jpg) $$quote($$OUT_PWD) $$escape_expand(\n\t)
QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../images/Rose-ProPhoto.jpg) $$quote($$OUT_PWD) $$escape_expand(\n\t)
