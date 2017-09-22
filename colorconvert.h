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
    AdobeRGB,
    ProPhotoRGB,
    AdobeWideGamutRGB,
    Rec709,
    Rec2020,
    DCI_P3,
    ColorSpaceCount
};
QString colorSpaceName(RgbColorSpace colorSpace);

// The RGBColorSpace represents a spesific RGB color space defined by the xy
// coordinates for the red green and blue primaries, and a gamma value. Several
// pre-defined colorspaces are also provided. (sRGB, AdobeRGB, ProPhotoRGB)
// 
// The class supports converting beween RGB and xyY colors.
//
// Implementation notes:
//    - some color spaces (like sRGB) has picewise "gamma" functions. This
//      class ignores this and uses a single gamma value.
//    - the white point is hardcoded to D65 white. 

class RGBColorSpace
{
public:
    RGBColorSpace(RgbColorSpace rgbSpace, qreal gamma);
    RGBColorSpace(qreal rxy[2], qreal gxy[2], qreal bxy[2],
                  qreal gamma, const QString &name);

    QGenericMatrix<1, 3, qreal> convertRGBtoYxy(QColor rgb);
    QColor convertYxyToRGB(QGenericMatrix<1, 3, qreal> Yxy);

    QGenericMatrix<3, 3, qreal> RGBtoXYZMatrix();
    QGenericMatrix<3, 3, qreal> XYZtoRGBMatrix();
    qreal gamma();
    QString name();
    
    static QGenericMatrix<3, 3, qreal> createRGBtoRGBMatrix(const RGBColorSpace &source,
                                                            const RGBColorSpace &destination);
private:
    qreal m_gamma;
    QString m_name;
    QGenericMatrix<3, 3, qreal> m_RGBtoXYZ;
    QGenericMatrix<3, 3, qreal> m_XYZtoRGB;
};

#endif
