
#include "colorconvert.h"

// RGB <-> XYZ
// Matrices from http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html

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
       
       
QString colorSpaceText[ColorSpaceCount] =
{
    QString("sRGB"),
    QString("sRGBLinear"),
    QString("AdobeRGB"),
    QString("ProPhotoRGB")
};

QString colorSpaceName(RgbColorSpace colorSpace)
{
    return colorSpaceText[colorSpace];
}

static QGenericMatrix<3, 3, qreal> rgbToXYZMatrices[ColorSpaceCount] =
{
    QGenericMatrix<3, 3, qreal>(sRGBtoXYZData),
    QGenericMatrix<3, 3, qreal>(sRGBtoXYZData),
    QGenericMatrix<3, 3, qreal>(adobeRGBtoXYZData),
    QGenericMatrix<3, 3, qreal>(proPhotoXYZData)
};

static QGenericMatrix<3, 3, qreal> XYZtoRGBMatrices[ColorSpaceCount] =
{
    QGenericMatrix<3, 3, qreal>(XYZtosRGBData),
    QGenericMatrix<3, 3, qreal>(XYZtosRGBData),
    QGenericMatrix<3, 3, qreal>(XYZtoAdobeRGBData),
    QGenericMatrix<3, 3, qreal>(XYZtoProPhotoData)
};

QGenericMatrix<1, 3, qreal> LinearRGBtoXYZ(QGenericMatrix<1, 3, qreal> rgb, RgbColorSpace rgbColorSpace)
{
    return rgbToXYZMatrices[rgbColorSpace] * rgb;
}

QGenericMatrix<1, 3, qreal> XYZtoLinearRGB(QGenericMatrix<1, 3, qreal> XYZ, RgbColorSpace rgbColorSpace)
{
    return XYZtoRGBMatrices[rgbColorSpace] * XYZ;
}

// Gamma: linear <-> nonlinear RGB

static qreal sRGBGamma = qreal(2.4);
static qreal adobeRGBGamma = qreal(563.0 / 256.0); // ~ 2.2
static qreal proPhotoGamma = qreal(1.8);
static qreal gammas[ColorSpaceCount] =
{
    sRGBGamma,
    adobeRGBGamma,
    proPhotoGamma
};

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
        case sRGBLinear:
            linear = rgb;
        break;
        case sRGB: // ### TODO handle special sRGB case
        default:
            linear = pow(rgb, qreal(1.0) / gammas[rgbColorSpace]);
        break;
    }

    return linear;
}

QGenericMatrix<1, 3, qreal> toNonlinearRGB(QGenericMatrix<1, 3, qreal> rgb, RgbColorSpace rgbColorSpace)
{
    QGenericMatrix<1, 3, qreal> nonlinear;

    switch (rgbColorSpace) {
        case sRGBLinear:
            nonlinear = rgb;
        break;
        case sRGB: // ### TODO handle special sRGB case
        default:
            nonlinear = pow(rgb, gammas[rgbColorSpace]);
        break;
    }
    return nonlinear;
}

// RGB <-> Yxy: TODO: make this xyY

QGenericMatrix<1, 3, qreal> XYZtoYxy(QGenericMatrix<1, 3, qreal> XYZ)
{
    const qreal sum = XYZ(0, 0) + XYZ(1, 0) + XYZ(2, 0);
    const qreal Yxy[] = {
        XYZ(1, 0),
        XYZ(0, 0) / sum,
        XYZ(1, 0) / sum
    };
    return QGenericMatrix<1, 3, qreal>(Yxy);
}

QGenericMatrix<1, 3, qreal> LinearRGBtoYxy(QGenericMatrix<1, 3, qreal> rgb, RgbColorSpace rgbColorSpace)
{
    return XYZtoYxy(LinearRGBtoXYZ(rgb, rgbColorSpace));
}

QGenericMatrix<1, 3, qreal> RGBtoYxy(QGenericMatrix<1, 3, qreal> rgb, RgbColorSpace rgbColorSpace)
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

QGenericMatrix<1, 3, qreal> YxyToLinearRGB(QGenericMatrix<1, 3, qreal> Yxy, RgbColorSpace rgbColorSpace)
{
    return XYZtoLinearRGB(YxyToXYZ(Yxy), rgbColorSpace);
}

QGenericMatrix<1, 3, qreal> YxyToRGB(QGenericMatrix<1, 3, qreal> Yxy, RgbColorSpace rgbColorSpace)
{
    return toNonlinearRGB(YxyToLinearRGB(Yxy, rgbColorSpace), rgbColorSpace);
}

// QColor convenience for RGB <-> Yxy

qreal clamp(qreal val, qreal min, qreal max)
{
    return qMax(qMin(val, max), min);
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

QGenericMatrix<1, 3, qreal> RGBtoYxy(QColor rgb, RgbColorSpace rgbColorSpace)
{
    return RGBtoYxy(toVector(rgb), rgbColorSpace);
}

QColor YxyToRGBQColor(QGenericMatrix<1, 3, qreal> Yxy, RgbColorSpace rgbColorSpace)
{
    return toColor(YxyToRGB(Yxy, rgbColorSpace));
}
