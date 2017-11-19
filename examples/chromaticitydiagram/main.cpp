#include <QtWidgets>

#include <chromaticitydiagram.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    ChromaticityDiagram diagram;
    diagram.setAttribute(Qt::WA_OpaquePaintEvent);
    diagram.show();

    return app.exec();
}
