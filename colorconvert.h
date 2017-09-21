#ifndef COLORCONVERT_H
#define COLORCONVERT_H

#include <QtCore>
#include <QtGui>

// RGB to XYZ and Yxy conversion. The conversion is dependent on
// which RGB color space is in use. This file implements support
// for a few popular RGB color spaces.
enum RgbColorSpace
{
    sRGB,
    sRGBLinear,
    AdobeRGB,
    ProPhotoRGB,
    ColorSpaceCount
};

QString colorSpaceName(RgbColorSpace colorSpace);

QGenericMatrix<1, 3, qreal> LinearRGBtoXYZ(QGenericMatrix<1, 3, qreal> rgb, RgbColorSpace rgbColorSpace);
QGenericMatrix<1, 3, qreal> XYZtoLinearRGB(QGenericMatrix<1, 3, qreal> XYZ, RgbColorSpace rgbColorSpace);

// Gamma: linear <-> nonlinear RGB
QGenericMatrix<1, 3, qreal> pow(QGenericMatrix<1, 3, qreal> values, qreal power);
QGenericMatrix<1, 3, qreal> toLinearRGB(QGenericMatrix<1, 3, qreal> rgb, RgbColorSpace rgbColorSpace);
QGenericMatrix<1, 3, qreal> toNonlinearRGB(QGenericMatrix<1, 3, qreal> rgb, RgbColorSpace rgbColorSpace);

// RGB <-> Yxy: TODO: make this xyY
QGenericMatrix<1, 3, qreal> XYZtoYxy(QGenericMatrix<1, 3, qreal> XYZ);
QGenericMatrix<1, 3, qreal> LinearRGBtoYxy(QGenericMatrix<1, 3, qreal> rgb, RgbColorSpace rgbColorSpace);
QGenericMatrix<1, 3, qreal> RGBtoYxy(QGenericMatrix<1, 3, qreal> rgb, RgbColorSpace rgbColorSpace);
QGenericMatrix<1, 3, qreal> YxyToXYZ(QGenericMatrix<1, 3, qreal> Yxy);
QGenericMatrix<1, 3, qreal> YxyToLinearRGB(QGenericMatrix<1, 3, qreal> Yxy, RgbColorSpace rgbColorSpace);
QGenericMatrix<1, 3, qreal> YxyToRGB(QGenericMatrix<1, 3, qreal> Yxy, RgbColorSpace rgbColorSpace);

// QColor convenience for RGB <-> Yxy
QGenericMatrix<1, 3, qreal> toVector(QColor rgb);
QColor toColor(QGenericMatrix<1, 3, qreal> rgb);
QGenericMatrix<1, 3, qreal> RGBtoYxy(QColor rgb, RgbColorSpace rgbColorSpace);
QColor YxyToRGBQColor(QGenericMatrix<1, 3, qreal> Yxy, RgbColorSpace rgbColorSpace);

#endif
