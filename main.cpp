#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtCharts>
QT_CHARTS_USE_NAMESPACE // wat
    
#include "chromaticitydiagram.h"
#include "colorconvert.h"
    
void printPrimaries()
{
    qDebug() << "";
    qDebug() << "Red Primary";
    QColor red(255, 0, 0);
    qDebug() << "sRGB" << RGBtoYxy(red, sRGB);
//    qDebug() << "AdobeRGB" << RGBtoYxy(red, AdobeRGB);
//    qDebug() << "ProPhotoRGB" << RGBtoYxy(red, ProPhotoRGB);

    qDebug() << "";
    qDebug() << "Green Primary";
    QColor green(0, 255, 0);
    qDebug() << "sRGB" << RGBtoYxy(green, sRGB);
//    qDebug() << "AdobeRGB" << RGBtoYxy(green, AdobeRGB);
//    qDebug() << "ProPhotoRGB" << RGBtoYxy(green, ProPhotoRGB);

    qDebug() << "";
    qDebug() << "Blue Primary";
    QColor blue(0, 0, 255);
    qDebug() << "sRGB" << RGBtoYxy(blue, sRGB);
//    qDebug() << "AdobeRGB" << RGBtoYxy(blue, AdobeRGB);
//    qDebug() << "ProPhotoRGB" << RGBtoYxy(blue, ProPhotoRGB);
}

void thereAndBackAgain()
{
    QColor gray(140, 140, 140);
    qDebug() << "     Gray" << gray;
    qDebug() << "Also Gray" << YxyToRGBQColor(RGBtoYxy(gray, sRGB), sRGB);

    //QColor blue(0.2 * 255, 0.3 * 255, 0.9 * 255);
    QColor blue(0, 0, 255);
    qDebug() << "     Blue" << blue;

    auto XYZ = LinearRGBtoXYZ(toVector(blue), sRGB);
    auto RGB = XYZtoLinearRGB(XYZ, sRGB);

    qDebug() << "XYZ" << XYZ;
    qDebug() << "RGB" << toColor(RGB);

    auto Yxy = XYZtoYxy(XYZ);
    qDebug() << "Yxy" << Yxy;
    auto XYZ2 = YxyToXYZ(Yxy);
    qDebug() << "YYZ 2" << XYZ2;

    qDebug() << "RGB 2" << XYZtoLinearRGB(XYZ, sRGB);
    qDebug() << "RGB 3" << XYZtoLinearRGB(YxyToXYZ(Yxy), sRGB);
    qDebug() << "RGB 4" << YxyToLinearRGB(Yxy, sRGB);

//    auto Yxy = LinearRGBtoYxy(toVector(blue), sRGB);

    qDebug() << "Also Blue" << YxyToRGBQColor(Yxy, sRGB);
}

class TestWindow : public QRasterWindow
{
public:
    TestWindow() {

    }

    QColor sample(QPoint position) {
        return m_image.pixelColor(position);
    }

    void paintEvent(QPaintEvent *ev) {
        QRect rect = QRect(QPoint(0, 0), size());

        // Render to indirect image to get readPixel access.
        if (m_image.size() != size()) {
            qreal dpr = devicePixelRatio();
            m_image = QImage(size() * dpr, QImage::Format_ARGB32_Premultiplied);
//            m_image.setDevicePixelRatio(dpr);

            QPainter p(&m_image);
            QLinearGradient gradient(rect.topLeft(), rect.bottomRight());

            gradient.setColorAt(0.1, Qt::green);
            gradient.setColorAt(0.3, Qt::blue);
            gradient.setColorAt(0.6, Qt::red);
            gradient.setColorAt(0.9, Qt::green);


            p.fillRect(rect, gradient);
        }

        QPainter p(this);
        p.fillRect(rect, m_image);

    }
private:
    QImage m_image;
};


class WindowColorController : public QWidget
{
public:
    WindowColorController(TestWindow *testWindow, ChromaticityDiagram *chromaticityDiagram) {
        setWindowTitle("Controller");

        m_testWindow = testWindow;
        m_chromaticityDiagram = chromaticityDiagram;

        m_testWindow->installEventFilter(this);
    }

    bool eventFilter(QObject *object, QEvent *ev)
    {
        // Filter for Mouse Move events.
        if (ev->type() != QEvent::MouseMove)
            return false;

        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(ev);
        //qDebug() << "mouse move" << mouseEvent->localPos();

        QPoint pos = mouseEvent->localPos().toPoint();
        if (pos.x() < 0 || pos.y() < 0)
            return false;

        // Create Color item on the diagram that is going to track the mouse cursor
        if (!m_item) {
            m_item = new ChromaticityColorItem();
            m_chromaticityDiagram->addColorItem(m_item);
        }

        // Sample test window at cursor position.
        QColor color = m_testWindow->sample(pos);
        m_item->setColor(color, sRGBLinear);

        return false;
    }

private:
    TestWindow *m_testWindow;
    ChromaticityDiagram *m_chromaticityDiagram;
    ChromaticityColorItem *m_item = nullptr;

};

class ColorViewer : public QWidget
{
public:
    ColorViewer() {

    }


private:


};

int main(int argc, char ** argv)
{
    QApplication app(argc, argv);

 //   printPrimaries();
//    thereAndBackAgain();
 //   printMonochromatic();

 // return 0;

    TestWindow testWindow;
    testWindow.setTitle("Test Window");

    ChromaticityDiagram chromaticityDiagram;

    {
        ChromaticityColorProfileItem *item = new ChromaticityColorProfileItem();
        item->setColorSpace(sRGB);
        chromaticityDiagram.addColorProfileItem(item);
    }

    {
        ChromaticityColorProfileItem *item = new ChromaticityColorProfileItem();
        item->setColorSpace(ProPhotoRGB);
        chromaticityDiagram.addColorProfileItem(item);
    }

    WindowColorController controller(&testWindow, &chromaticityDiagram);

    // Show all windows

    testWindow.setGeometry(100, 100, 300, 300);
    testWindow.raise();
    testWindow.show();

    chromaticityDiagram.setGeometry(600, 100, 500, 600);
    chromaticityDiagram.raise();
    chromaticityDiagram.show();

    controller.setGeometry(100, 500, 300, 300);
    controller.raise();
    controller.show();

    return app.exec();

}

// READing:
// http://www.ryanjuckett.com/programming/rgb-color-space-conversion/
// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
