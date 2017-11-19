
#include "colorconvert.h"

#include <iostream>

QGenericMatrix<1, 3, qreal> RGBtoYxy(QColor rgb, RGBColorSpace rgbColorSpace);
QColor YxyToRGBQColor(QGenericMatrix<1, 3, qreal> Yxy, RGBColorSpace rgbColorSpace);
QGenericMatrix<1, 3, qreal> toVector(QColor rgb);
QColor toColor(QGenericMatrix<1, 3, qreal> rgb);
QGenericMatrix<1, 3, qreal> toLinearRGB(QGenericMatrix<1, 3, qreal> rgb, RGBColorSpace rgbColorSpace);
QGenericMatrix<1, 3, qreal> toNonlinearRGB(QGenericMatrix<1, 3, qreal> rgb, RGBColorSpace rgbColorSpace);
QGenericMatrix<1, 3, qreal> LinearRGBtoXYZ(QGenericMatrix<1, 3, qreal> rgb, RGBColorSpace rgbColorSpace);
qreal colorSpaceGamma(RgbColorSpace colorSpace);

// RGB <-> XYZ Matrices
// From http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html

static qreal sRGBtoXYZData[] =
    { 0.4124564, 0.3575761, 0.1804375,
      0.2126729, 0.7151522, 0.0721750,
      0.0193339, 0.1191920, 0.9503041 };

static qreal XYZtosRGBData[] =
    {  3.2404542,  -1.5371385, -0.4985314,
      -0.9692660,   1.8760108,  0.0415560,
       0.0556434,  -0.2040259,  1.0572252 };

static qreal adobeRGBtoXYZData[] =
    { 0.5767309, 0.1855540, 0.1881852,
      0.2973769, 0.6273491, 0.0752741,
      0.0270343, 0.0706872, 0.9911085 };

static qreal XYZtoAdobeRGBData[] =
    { 2.0413690, -0.5649464, -0.3446944,
     -0.9692660,  1.8760108,  0.0415560,
      0.0134474, -0.1183897,  1.0154096 };

static qreal proPhotoXYZData[] =
     { 0.7976749, 0.1351917, 0.0313534,
       0.2880402, 0.7118741, 0.0000857,
       0.0000000, 0.0000000, 0.8252100 };

static qreal XYZtoProPhotoData[] =
     { 1.3459433, -0.2556075, -0.0511118,
      -0.5445989,  1.5081673,  0.0205351,
       0.0000000,  0.0000000,  1.2118128 };

// More color profiles: in RGB primaries + white point form
// { rx, ry, gx, gy, bx, by, wx, wy }
static qreal adobeWideGamutRGB[] = { 0.7347, 0.2653, 0.1152, 0.8264, 0.1566, 0.0177, 0.3457, 0.3585 };
static qreal rec709[] = { 0.64, 0.33, 0.30, 0.60, 0.15, 0.06, 0.3127, 0.3290 };
static qreal rec2020[] = { 0.708, 0.292, 0.170, 0.797, 0.131, 0.046, 0.3127, 0.3290 };
static qreal dci_p3[] = { 0.680, 0.320, 0.265, 0.690, 0.150, 0.060, 0.3127,	0.3290 };

static QGenericMatrix<3, 3, qreal> deriveNPMConversionMatrix(qreal rgbw_xy[]);
static QGenericMatrix<3, 3, qreal> inverted(const QGenericMatrix<3, 3, qreal> &m);

static QGenericMatrix<3, 3, qreal> rgbToXYZMatrices[ColorSpaceCount] =
{
    QGenericMatrix<3, 3, qreal>(sRGBtoXYZData),
    QGenericMatrix<3, 3, qreal>(adobeRGBtoXYZData),
    QGenericMatrix<3, 3, qreal>(proPhotoXYZData),
    deriveNPMConversionMatrix(adobeWideGamutRGB),
    deriveNPMConversionMatrix(rec709),
    deriveNPMConversionMatrix(rec2020),
    deriveNPMConversionMatrix(dci_p3)
};

static QGenericMatrix<3, 3, qreal> XYZtoRGBMatrices[ColorSpaceCount] =
{
    QGenericMatrix<3, 3, qreal>(XYZtosRGBData),
    QGenericMatrix<3, 3, qreal>(XYZtoAdobeRGBData),
    QGenericMatrix<3, 3, qreal>(XYZtoProPhotoData),
    inverted(rgbToXYZMatrices[AdobeWideGamutRGB]),
    inverted(rgbToXYZMatrices[Rec709]),
    inverted(rgbToXYZMatrices[Rec2020]),
    inverted(rgbToXYZMatrices[DCI_P3])
};

QString colorSpaceText[ColorSpaceCount] =
{
    QString("sRGB"),
    QString("AdobeRGB"),
    QString("ProPhotoRGB"),
    QString("AdobeWideGamutRGB"),
    QString("Rec709"),
    QString("Rec2020"),
    QString("DCI-P3")
};

QString colorSpaceName(RgbColorSpace colorSpace)
{
    return colorSpaceText[colorSpace];
}

QStringList colorSpaceNames()
{
    QStringList colorSpaceNames;
    for (int i = 0; i < ColorSpaceCount; ++i) {
        colorSpaceNames.append(colorSpaceName(RgbColorSpace(i)));
    }
    return colorSpaceNames;
}

// https://stackoverflow.com/questions/983999/simple-3x3-matrix-inverse-code-c
static QGenericMatrix<3, 3, qreal> inverted(const QGenericMatrix<3, 3, qreal> &m)
{
    qreal det = m(0, 0) * (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) -
                m(0, 1) * (m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0)) +
                m(0, 2) * (m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0));

    qreal invdet = qreal(1) / det;

    QGenericMatrix<3, 3, qreal> minv; // inverse of matrix m
    minv(0, 0) = (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) * invdet;
    minv(0, 1) = (m(0, 2) * m(2, 1) - m(0, 1) * m(2, 2)) * invdet;
    minv(0, 2) = (m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1)) * invdet;
    minv(1, 0) = (m(1, 2) * m(2, 0) - m(1, 0) * m(2, 2)) * invdet;
    minv(1, 1) = (m(0, 0) * m(2, 2) - m(0, 2) * m(2, 0)) * invdet;
    minv(1, 2) = (m(1, 0) * m(0, 2) - m(0, 0) * m(1, 2)) * invdet;
    minv(2, 0) = (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1)) * invdet;
    minv(2, 1) = (m(2, 0) * m(0, 1) - m(0, 0) * m(2, 1)) * invdet;
    minv(2, 2) = (m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1)) * invdet;
    
    return minv;
}

QGenericMatrix<1, 3, qreal> pow(QGenericMatrix<1, 3, qreal> values, qreal power)
{
    const qreal raised[] = { qPow(values(0, 0), power),
                             qPow(values(1, 0), power),
                             qPow(values(2, 0), power) };
    return QGenericMatrix<1, 3, qreal>(raised);
}

static QGenericMatrix<3, 3, qreal> deriveNPMConversionMatrix(qreal rxy[2], qreal gxy[2], qreal bxy[2], qreal wxy[2])
{
    // Derive RGB -> XYZ matrix as described in 
    // SMPTE RP (Recommended Practice) 177.

    // 3.3.2 Compute z
    qreal rz = 1 - (rxy[0] + rxy[1]);
    qreal gz = 1 - (gxy[0] + gxy[1]);
    qreal bz = 1 - (bxy[0] + bxy[1]);
    qreal wz = 1 - (wxy[0] + wxy[1]);
    
    // 3.3.3 Form P and W matrices
    QGenericMatrix<3, 3, qreal> P((qreal []) {
        rxy[0], gxy[0], bxy[0],
        rxy[1], gxy[1], bxy[1],
        rz,     gz,     bz
    });

    QGenericMatrix<1, 3, qreal> W((qreal []) {
         wxy[0] / wxy[1], 1, wz / wxy[1]
    });

    // 3.3.4 Compute coefficients Ci
    QGenericMatrix<1, 3, qreal> Ci = inverted(P) * W;
    
    // 3.3.5 Form the diagonal matrix C
    QGenericMatrix<3, 3, qreal> C((qreal []) {
          Ci(0, 0),  0,         0,
          0,         Ci(1, 0),  0,
          0,         0,         Ci(2, 0)
    });

    // 3.3.6 Compute the final normalized primary matrix NPM
    QGenericMatrix<3, 3, qreal> NPM = P * C;

    return NPM;
}

static QGenericMatrix<3, 3, qreal> deriveNPMConversionMatrix(qreal rgbw_xy[])
{
    return deriveNPMConversionMatrix(rgbw_xy + 0, rgbw_xy + 2, rgbw_xy + 4, rgbw_xy + 6);
}

QGenericMatrix<1, 3, qreal> LinearRGBtoXYZ(QGenericMatrix<1, 3, qreal> rgb, RGBColorSpace rgbColorSpace)
{
    return rgbColorSpace.RGBtoXYZMatrix() * rgb;
}

QGenericMatrix<1, 3, qreal> XYZtoLinearRGB(QGenericMatrix<1, 3, qreal> XYZ, RGBColorSpace rgbColorSpace)
{
    return rgbColorSpace.XYZtoRGBMatrix() * XYZ;
}

// Gamma: linear <-> nonlinear RGB

static qreal sRGBGamma = qreal(2.2); // ### ignoring sRGB complexities
static qreal adobeRGBGamma = qreal(563.0 / 256.0); // ~ 2.2
static qreal adobeWideGamutRGBGamma = qreal(563.0 / 256.0); // ~ 2.2
static qreal proPhotoGamma = qreal(1.8);
static qreal rec709Gamma = qreal(2.4);
static qreal rec20202Gamma = qreal(2.4);
static qreal dciP3Gamma = qreal(2.6);
static qreal displayP3Gamma = qreal(2.2); // Apple Display P3: DCI-P3 with sRGB gamma

static qreal gammas[ColorSpaceCount] =
{
    sRGBGamma,
    adobeRGBGamma,
    proPhotoGamma,
    adobeWideGamutRGBGamma,
    rec709Gamma,
    rec20202Gamma,
    dciP3Gamma
};

qreal colorSpaceGamma(RgbColorSpace colorSpace)
{
    // MARK FIX GAMMA
    //return 1.0;
    return gammas[colorSpace];
}

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

QGenericMatrix<1, 3, qreal> toLinearRGB(QGenericMatrix<1, 3, qreal> rgb, RGBColorSpace rgbColorSpace)
{
    return pow(rgb, rgbColorSpace.gamma());
}

QGenericMatrix<1, 3, qreal> toNonlinearRGB(QGenericMatrix<1, 3, qreal> rgb, RGBColorSpace rgbColorSpace)
{
    return pow(rgb, qreal(1.0) / rgbColorSpace.gamma());
}

// RGB <-> Yxy: TODO: make this xyY

QGenericMatrix<1, 3, qreal> XYZtoYxy(QGenericMatrix<1, 3, qreal> XYZ)
{
    const qreal X = XYZ(0, 0);
    const qreal Y = XYZ(1, 0);
    const qreal Z = XYZ(2, 0);
    const qreal sum = X + Y + Z;
    if (sum < 0.01) {
        // Black / dark grey colors cause numerical instability when the small
        // sum is used as a divisor below. Return the xy coordinates for the
        // white point (hardcode D65), whith a Y component of 0.
        const qreal Yxy[] = { 0, 0.3127, 0.3290 };
        return QGenericMatrix<1, 3, qreal>(Yxy);
    }

    const qreal x = X / sum;
    const qreal y = Y / sum;
    const qreal Yxy[] = { Y, x, y };
    return QGenericMatrix<1, 3, qreal>(Yxy);
}

QGenericMatrix<1, 3, qreal> LinearRGBtoYxy(QGenericMatrix<1, 3, qreal> rgb, RGBColorSpace rgbColorSpace)
{
    return XYZtoYxy(LinearRGBtoXYZ(rgb, rgbColorSpace));
}

QGenericMatrix<1, 3, qreal> RGBtoYxy(QGenericMatrix<1, 3, qreal> rgb, RGBColorSpace rgbColorSpace)
{
    return LinearRGBtoYxy(toLinearRGB(rgb, rgbColorSpace), rgbColorSpace);
}

QGenericMatrix<1, 3, qreal> YxyToXYZ(QGenericMatrix<1, 3, qreal> Yxy)
{
    const qreal Y = Yxy(0,0);
    const qreal x = Yxy(1,0);
    const qreal y = Yxy(2,0);
    const qreal X = (Y / y) * x;
    const qreal Z = (Y / y) * (1 - x - y);
    const qreal XYZ[] = { X, Y, Z };
    return QGenericMatrix<1, 3, qreal>(XYZ);
}

QGenericMatrix<1, 3, qreal> YxyToLinearRGB(QGenericMatrix<1, 3, qreal> Yxy, RGBColorSpace rgbColorSpace)
{
    return XYZtoLinearRGB(YxyToXYZ(Yxy), rgbColorSpace);
}

QGenericMatrix<1, 3, qreal> YxyToRGB(QGenericMatrix<1, 3, qreal> Yxy, RGBColorSpace rgbColorSpace)
{
    return toNonlinearRGB(YxyToLinearRGB(Yxy, rgbColorSpace), rgbColorSpace);
}

// QColor convenience for RGB <-> Yxy

qreal clamp(qreal val, qreal min, qreal max)
{
    return qMax(qMin(val, max), min);
}

QGenericMatrix<1, 3, qreal> clamp(QGenericMatrix<1, 3, qreal> rgb)
{
    return QGenericMatrix<1, 3, qreal>((qreal []) {
        clamp(rgb(0, 0), 0, 1),
        clamp(rgb(1, 0), 0, 1),
        clamp(rgb(2, 0), 0, 1)
    });
}

QGenericMatrix<1, 3, qreal> toVector(QColor rgb)
{
    const qreal rgbReal[3] = { rgb.redF(), rgb.greenF(), rgb.blueF() };
    return QGenericMatrix<1, 3, qreal>(rgbReal);
}

QColor toColor(QGenericMatrix<1, 3, qreal> rgb)
{
    return QColor(clamp(rgb(0, 0), 0, 1) * 255.0,
                  clamp(rgb(1, 0), 0, 1) * 255.0,
                  clamp(rgb(2, 0), 0, 1) * 255.0);
}

QGenericMatrix<1, 3, qreal> RGBtoYxy(QColor rgb, RGBColorSpace rgbColorSpace)
{
    return RGBtoYxy(toVector(rgb), rgbColorSpace);
}

QColor YxyToRGBQColor(QGenericMatrix<1, 3, qreal> Yxy, RGBColorSpace rgbColorSpace)
{
    return toColor(YxyToRGB(Yxy, rgbColorSpace));
}

QColor convertColor(QColor color, const RGBColorSpace &source, const RGBColorSpace &destination)
{
    // Create RGB -> RGB conversion matrix
    QGenericMatrix<3, 3, qreal> conversion = source.RGBtoXYZMatrix() * destination.XYZtoRGBMatrix();

     // Convert to Linear RGB
    QGenericMatrix<1, 3, qreal> sourceLinearRGB = toLinearRGB(toVector(color), source);

    // Color convert to destinationRGB
    QGenericMatrix<1, 3, qreal> destinationLinearRGB = clamp(conversion * sourceLinearRGB);

    // Apply gamma
    QGenericMatrix<1, 3, qreal> destinationRGB = toNonlinearRGB(destinationLinearRGB, destination);


    // Clip out-of-gamut colors
    QColor destinationColor = toColor(destinationRGB);

    return destinationColor;
}

void convertImage(QImage *image, const RGBColorSpace &source, const RGBColorSpace &destination)
{
    // Create RGB -> RGB conversion matrix
    QGenericMatrix<3, 3, qreal> conversion = destination.XYZtoRGBMatrix() * source.RGBtoXYZMatrix();

    int width = image->width();
    int height = image->height();

    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(image->scanLine(y));
        for (int x = 0; x < width; ++x) {
            QColor sourceColor = QColor::fromRgb(*(line + x));
            QGenericMatrix<1, 3, qreal> sourceLinearRGB = toLinearRGB(toVector(sourceColor), source);
            QGenericMatrix<1, 3, qreal> destinationLinearRGB = clamp(conversion * sourceLinearRGB);
            QGenericMatrix<1, 3, qreal> destinationRGB = toNonlinearRGB(destinationLinearRGB, destination);
            QColor destinationColor = toColor(destinationRGB);
            *(line + x) = destinationColor.rgb();
        }
    }
}


RGBColorSpace::RGBColorSpace()
:m_isValid(false)
,m_gamma(1.0)
,m_name("null")
{

}

RGBColorSpace::RGBColorSpace(RgbColorSpace rgbSpace)
:m_isValid(true)
,m_gamma(colorSpaceGamma(rgbSpace))
,m_name(colorSpaceName(rgbSpace))
{
    m_RGBtoXYZ = rgbToXYZMatrices[rgbSpace];
    m_XYZtoRGB = XYZtoRGBMatrices[rgbSpace];
}

RGBColorSpace::RGBColorSpace(RgbColorSpace rgbSpace, qreal gamma)
:m_isValid(true)
,m_gamma(gamma)
,m_name(colorSpaceName(rgbSpace))
{
    m_RGBtoXYZ = rgbToXYZMatrices[rgbSpace];
    m_XYZtoRGB = XYZtoRGBMatrices[rgbSpace];
}

RGBColorSpace::RGBColorSpace(qreal rxy[2], qreal gxy[2], qreal bxy[2], qreal gamma, const QString &name)
:m_isValid(true)
,m_gamma(gamma)
,m_name(name)
{
    // white point (hardcode D65)
    qreal wxy[2] = { 0.3127, 0.3290 };

    // create RGB <-> XYZ matrices
    m_RGBtoXYZ = deriveNPMConversionMatrix(rxy, gxy, bxy, wxy);
    m_XYZtoRGB = inverted(m_RGBtoXYZ);
}

QGenericMatrix<1, 3, qreal> RGBColorSpace::convertRGBtoYxy(QColor rgb)
{
    return RGBtoYxy(rgb, *this);
}

QGenericMatrix<1, 3, qreal> RGBColorSpace::convertRGBtoXYZ(QColor rgb)
{
    return LinearRGBtoXYZ(toLinearRGB(toVector(rgb), *this), *this);
}

QColor RGBColorSpace::convertYxyToRGB(QGenericMatrix<1, 3, qreal> Yxy)
{
    return YxyToRGBQColor(Yxy, *this);
}

QGenericMatrix<3, 3, qreal> RGBColorSpace::RGBtoXYZMatrix() const
{
    return m_RGBtoXYZ;
}

QGenericMatrix<3, 3, qreal> RGBColorSpace::XYZtoRGBMatrix() const
{
    return m_XYZtoRGB;
}

qreal RGBColorSpace::gamma()
{
    return m_gamma;
}

QString RGBColorSpace::name()
{
   return m_name;
}

QGenericMatrix<3, 3, qreal> RGBColorSpace::createRGBtoRGBMatrix(const RGBColorSpace &source,
                                                                const RGBColorSpace &destination)
{
    return source.RGBtoXYZMatrix() * destination.XYZtoRGBMatrix();
}

QColor RGBColorSpace::colorConvert(QColor color, const RGBColorSpace &source, const RGBColorSpace &destination)
{
    return convertColor(color, source, destination);
}

void RGBColorSpace::colorConvert(QImage *image, const RGBColorSpace &source, const RGBColorSpace &destination)
{
    convertImage(image, source, destination);
}

// Testing

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define COMPARE(a, b) if (!(a == b)) qFatal("%s", QString("Test fail at %1:%2").arg(__FILE__).arg(TOSTRING(__LINE__)).toLocal8Bit().constData());
#define VERIFY(a) if (!(a)) qFatal("%s", QString("Test fail at %1:%2").arg(__FILE__).arg(TOSTRING(__LINE__)).toLocal8Bit().constData());

// qFuzzyCompare is to strict for our use case.
bool almostEqual(qreal a, qreal b)
{
    return qAbs(a - b) < 0.00001;
}

static void printme(void *c, size_t n)
{
  unsigned char *t = (unsigned char *)c;
  if (c == NULL)
    return;
  while (n > 0) {
      int q;
      --n;
      for(q = 0x80; q; q >>= 1)
        printf("%x", !!(t[n] & q));
    }
  printf("\n");
}

void colorConvertTest()
{
    // Values from color calculator at
    // http://www.brucelindbloom.com/index.html?ColorCalculator.html

    // Test gamma
    RGBColorSpace sRGBSpace(sRGB);

    auto gammaEncoded = QGenericMatrix<1, 3, qreal>((qreal []) { 1.0, 0.5, 0.0 });
    auto linear = toLinearRGB(gammaEncoded, sRGBSpace);

    // Note to self: gamma encoding distributes available values to give more coverage to
    // lover intensities. It is therefore expected that the linear values will be
    // smaller (or equal, for the cases of 1.0 and 0.0).
    VERIFY(linear(0, 0) <= gammaEncoded(0, 0))
    VERIFY(linear(1, 0) <= gammaEncoded(1, 0))
    VERIFY(linear(2, 0) <= gammaEncoded(2, 0))

    // Encoding back should give the same result.
    auto gammaEncoded2 = toNonlinearRGB(linear, sRGBSpace);
    VERIFY(gammaEncoded == gammaEncoded2)

    // Verify converting to XYZ
    auto XYZ = LinearRGBtoXYZ(linear, sRGBSpace);
    // Reference for sRGB { 1.0, 0.5, 0.0 }
    // (http://davengrace.com/cgi-bin/cspace.pl)
    // 0.488941111836446 0.365682223672893 0.0448137039454821
    // qDebug() << XYZ;

    // Going back to RGB should give approxemately the same values
   auto linear2 = XYZtoLinearRGB(XYZ, sRGBSpace);
   VERIFY(almostEqual(linear(0, 0), linear2(0, 0)));
   VERIFY(almostEqual(linear(1, 0), linear2(1, 0)));
   VERIFY(almostEqual(linear(2, 0), linear2(2, 0)));

   auto roundtripMatrix = sRGBSpace.RGBtoXYZMatrix() * sRGBSpace.XYZtoRGBMatrix();
   auto linear3 = roundtripMatrix * linear;
   VERIFY(almostEqual(linear(0, 0), linear3(0, 0)));
   VERIFY(almostEqual(linear(1, 0), linear3(1, 0)));
   VERIFY(almostEqual(linear(2, 0), linear3(2, 0)));
}

