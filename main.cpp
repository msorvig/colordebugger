#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtCharts>
QT_CHARTS_USE_NAMESPACE // wat

// RGB to XYZ and Yxy conversion. The conversion is dependent on
// which RGB color space is in use. This file implements support
// for a few popular RGB color spaces.
enum RgbColorSpace
{
    sRGB,
    AdobeRGB,
    ProPhotoRGB,
    ColorSpaceCount
};

// RGB <-> XYZ
// Matrices from http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html

static qreal sRGBtoXYZData[] =
    { 0.4124564, 0.3575761, 0.1804375,
      0.2126729, 0.7151522, 0.0721750,
      0.0193339, 0.1191920, 0.9503041 };

static qreal adobeRGBtoXYZData[] = 
    { 0.5767309, 0.1855540, 0.1881852,
      0.2973769, 0.6273491, 0.0752741,
      0.0270343, 0.0706872, 0.9911085 };

static qreal proPhotoXYZData[] =
     { 0.7976749, 0.1351917, 0.0313534,
       0.2880402, 0.7118741, 0.0000857,
       0.0000000, 0.0000000, 0.8252100 };

static QGenericMatrix<3, 3, qreal> rgbToXYZMatrices[ColorSpaceCount] = 
{ 
    QGenericMatrix<3, 3, qreal>(sRGBtoXYZData),
    QGenericMatrix<3, 3, qreal>(adobeRGBtoXYZData),
    QGenericMatrix<3, 3, qreal>(proPhotoXYZData)
};

static QGenericMatrix<3, 3, qreal> XYZtoRGBMatrices[ColorSpaceCount] = 
{ 
    rgbToXYZMatrices[sRGB].transposed(),
    rgbToXYZMatrices[AdobeRGB].transposed(),
    rgbToXYZMatrices[ProPhotoRGB].transposed()
};

QGenericMatrix<1, 3, qreal> LinearRGBtoXYZ(QGenericMatrix<1, 3, qreal> rgb, RgbColorSpace rgbColorSpace)
{
    return rgbToXYZMatrices[rgbColorSpace] * rgb;
}

QGenericMatrix<1, 3, qreal> XYZtoLinearRGB(QGenericMatrix<1, 3, qreal> xyz, RgbColorSpace rgbColorSpace)
{
    return XYZtoRGBMatrices[rgbColorSpace] * xyz;
}

// Gamma: linear <-> nonlinear RGB

static qreal sRGBGamma = qreal(2.4);
static qreal adobeRGBGamma = qreal(563.0 / 256.0); // ~ 2.2
static qreal proPhotoGamma = qreal(1.8);

//qreal srgbToLinear(qreal nonlinear)
//{    
//    return (nonlinear <= qreal(0.04045))
//           ? (nonlinear / qreal(12.92))
//           : (pow((nonlinear + qreal(0.055)/ qreal(1.055), sRGBGamma));
//}
  
//qreal srgbToNonLinear(qreal linear)
//{    
//    return   (linear <= qreal(0.0031308))
//           ? (qreal(12.92) * linear)
//           : (qreal(1.055) * pow(linear, qreal(1.0)/ qreal(sRGBGamma)) - qreal(0.055));
// }

QGenericMatrix<1, 3, qreal> pow(QGenericMatrix<1, 3, qreal> values, qreal power)
{
    const qreal raised[] = { qPow(values(0, 0), power),
                             qPow(values(1, 0), power), 
                             qPow(values(2, 0), power) };
    return QGenericMatrix<1, 3, qreal>(raised); 
}

QGenericMatrix<1, 3, qreal> toLinearRGB(QGenericMatrix<1, 3, qreal> rgb, RgbColorSpace rgbColorSpace)
{
    QGenericMatrix<1, 3, qreal> linear;
    
    switch (rgbColorSpace) {
        case sRGB: {
        break;
            linear = pow(rgb, qreal(1.0) / sRGBGamma);
        }
        case AdobeRGB:
            linear = pow(rgb, qreal(1.0) / adobeRGBGamma);
        break;
        case ProPhotoRGB:
            linear = pow(rgb, qreal(1.0) / proPhotoGamma);
        break;
        default:
            qDebug() << "unexpected this is";
        break;
    }
    return linear;
}

QGenericMatrix<1, 3, qreal> toNonlinearRGB(QGenericMatrix<1, 3, qreal> rgb, RgbColorSpace rgbColorSpace)
{
    QGenericMatrix<1, 3, qreal> nonlinear;

    switch (rgbColorSpace) {
        case sRGB: {
            nonlinear = pow(rgb, sRGBGamma);
        break;
        }
        case AdobeRGB:
            nonlinear = pow(rgb, adobeRGBGamma);
        break;
        case ProPhotoRGB:
            nonlinear = pow(rgb, proPhotoGamma);
        break;
        default:
            qDebug() << "unexpected this is";
        break;
    }
    return nonlinear;
}

// RGB <-> Yxy: TODO: make this xyY

QGenericMatrix<1, 3, qreal> LinearRGBtoYxy(QGenericMatrix<1, 3, qreal> rgb, RgbColorSpace rgbColorSpace)
{
    const QGenericMatrix<1, 3, qreal> XYZ = LinearRGBtoXYZ(rgb, rgbColorSpace);
    const qreal sum = XYZ(0, 0) + XYZ(1, 0) + XYZ(2, 0);
    const qreal Yxy[] = {
        XYZ(1, 0),
        XYZ(0, 0) / sum,
        XYZ(1, 0) / sum
    };
    return QGenericMatrix<1, 3, qreal>(Yxy);
}

QGenericMatrix<1, 3, qreal> RGBtoYxy(QGenericMatrix<1, 3, qreal> rgb, RgbColorSpace rgbColorSpace)
{
    return LinearRGBtoYxy(toLinearRGB(rgb, rgbColorSpace), rgbColorSpace);
}

QGenericMatrix<1, 3, qreal> YxyToLinearRGB(QGenericMatrix<1, 3, qreal> Yxy, RgbColorSpace rgbColorSpace)
{
    const qreal Y = Yxy(0,0);
    const qreal x = Yxy(1,0);
    const qreal y = Yxy(2,0);
    const qreal X = (Y / y) * x;
    const qreal Z = (Y / y) * (1 - x - y);
    const qreal XYZ[] = { X, Y, Z };
    return XYZtoLinearRGB(QGenericMatrix<1, 3, qreal>(XYZ), rgbColorSpace);
}

QGenericMatrix<1, 3, qreal> YxyToRGB(QGenericMatrix<1, 3, qreal> Yxy, RgbColorSpace rgbColorSpace)
{
    return toNonlinearRGB(XYZtoLinearRGB(Yxy, rgbColorSpace), rgbColorSpace);
}

// QColor convenience for RGB <-> Yxy

QGenericMatrix<1, 3, qreal> RGBtoYxy(QColor rgb, RgbColorSpace rgbColorSpace)
{
    const qreal rgbReal[3] = { rgb.redF(), rgb.greenF(), rgb.blueF() };
    return RGBtoYxy(QGenericMatrix<1, 3, qreal>(rgbReal), rgbColorSpace);
}

QColor YxyToRGBQColor(QGenericMatrix<1, 3, qreal> Yxy, RgbColorSpace rgbColorSpace)
{
    QGenericMatrix<1, 3, qreal> rgb = YxyToRGB(Yxy, rgbColorSpace);
    return QColor(rgb(0, 0) * 255.0, rgb(1, 0) * 255.0, rgb(2, 0) * 255.0);
}

void printPrimaries()
{
    qDebug() << "";
    qDebug() << "Red Primary";
    QColor green(255, 0, 0);
    qDebug() << "sRGB" << RGBtoYxy(green, sRGB);
    qDebug() << "AdobeRGB" << RGBtoYxy(green, AdobeRGB);
    qDebug() << "ProPhotoRGB" << RGBtoYxy(green, ProPhotoRGB);

    qDebug() << "";
    qDebug() << "Green Primary";
    QColor red(0, 255, 0);
    qDebug() << "sRGB" << RGBtoYxy(red, sRGB);
    qDebug() << "AdobeRGB" << RGBtoYxy(red, AdobeRGB);
    qDebug() << "ProPhotoRGB" << RGBtoYxy(red, ProPhotoRGB);

    qDebug() << "";
    qDebug() << "Blue Primary";
    QColor blue(0, 0, 255);
    qDebug() << "sRGB" << RGBtoYxy(blue, sRGB);
    qDebug() << "AdobeRGB" << RGBtoYxy(blue, AdobeRGB);
    qDebug() << "ProPhotoRGB" << RGBtoYxy(blue, ProPhotoRGB);
}

void thereAndBackAgain()
{
    QColor gray(140, 140, 140);
    qDebug() << "     Gray" << gray;
    qDebug() << "Also Gray" << YxyToRGBQColor(RGBtoYxy(gray, sRGB), sRGB);
}

class ChromaticityDiagram : public QGraphicsView
{
public:
    ChromaticityDiagram() {
        setWindowTitle("Chromaticity Diagram");
        
        m_scene = new QGraphicsScene(this);
        m_scene->setBackgroundBrush(QColor(240, 240, 240));
        setScene(m_scene);
        
        m_chart = new QChart();
        m_chart->setGeometry(QRectF(QPointF(0,0), QSizeF(480, 580)));
        
        QLineSeries *series = new QLineSeries();
        m_chart->addSeries(series);
        QValueAxis *axisX = new QValueAxis;
        axisX->setRange(0, 0.9);
        axisX->setTickCount(10);
        axisX->setLabelFormat("%g");
        axisX->setTitleText("x");
        
        QValueAxis *axisY = new QValueAxis;
        axisY->setRange(0, 0.9);
        axisY->setTickCount(10);
        axisY->setTitleText("y");
        m_chart->setAxisX(axisX, series);
        m_chart->setAxisY(axisY, series);
        m_chart->legend()->hide();
        m_chart->setTitle("CIE Chromaticity");
        m_scene->addItem(m_chart);
        
        
        qDebug() << "sceneRect" << m_scene->sceneRect();
        qDebug() << "m_chart geometry" << m_chart->geometry();
        
        //QGraphicsView::fitInView(m_scene->sceneRect(), Qt::IgnoreAspectRatio);
        
        setStyleSheet( "QGraphicsView { border-style: none; }" );
    }

private:
    QGraphicsScene *m_scene;
    QChart *m_chart;
};

class WindowColorController : public QWidget
{
public:
    WindowColorController(QWindow *window) {
        setWindowTitle("Window Color Controller");
        Q_UNUSED(window);
    }
    
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

    printPrimaries();
    thereAndBackAgain();
    
  //  return 0;

    QWindow window;
    window.setTitle("Test Window");
    window.resize(320, 200);
    window.show();
    
    WindowColorController controller(&window);
    controller.resize(320, 600);
   // controller.show();
    
    ChromaticityDiagram chromaticityDiagram;
    chromaticityDiagram.resize(500, 600);
    chromaticityDiagram.show();
    
    return app.exec();
    
}

// READing:
// http://www.ryanjuckett.com/programming/rgb-color-space-conversion/
// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
