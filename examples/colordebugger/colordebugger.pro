TEMPLATE = app

include(../../src/colordebugger.pri)
RESOURCES += ../../images/images.qrc

SOURCES += main.cpp

QT += widgets
OBJECTS_DIR = .obj
MOC_DIR = .moc
