#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtCharts>
#include <QtOpenGL>
QT_CHARTS_USE_NAMESPACE // wat

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

QString colorSpaceText[ColorSpaceCount] =
{
    QString("sRGB"),
    QString("sRGBLinear"),
    QString("AdobeRGB"),
    QString("ProPhotoRGB")
};

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
        case sRGB: // ### TODO handle special case
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
        case sRGB: // ### TODO handle special case
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

// Data generated from monochromatic_xy.py
int begin_wl = 360;
int end_wl = 830;
int delta_wl = 1;
int entries = 521;
qreal monochromatic_xy[521][3] = {
    { 0.1755602318, 0.0052938370 },
    { 0.1754825277, 0.0052863391 },
    { 0.1754000224, 0.0052786420 },
    { 0.1753170495, 0.0052709688 },
    { 0.1752367395, 0.0052634939 },
    { 0.1751612185, 0.0052563459 },
    { 0.1750877942, 0.0052468445 },
    { 0.1750149389, 0.0052355703 },
    { 0.1749451894, 0.0052261569 },
    { 0.1748801248, 0.0052207849 },
    { 0.1748206077, 0.0052206009 },
    { 0.1747702522, 0.0052286672 },
    { 0.1747220367, 0.0052375202 },
    { 0.1746653680, 0.0052361607 },
    { 0.1745950503, 0.0052183223 },
    { 0.1745097209, 0.0051816398 },
    { 0.1744092494, 0.0051267609 },
    { 0.1743084582, 0.0050675925 },
    { 0.1742217721, 0.0050170315 },
    { 0.1741555944, 0.0049814449 },
    { 0.1741122344, 0.0049637260 },
    { 0.1740883072, 0.0049636001 },
    { 0.1740725909, 0.0049725426 },
    { 0.1740570243, 0.0049820361 },
    { 0.1740362706, 0.0049859614 },
    { 0.1740079175, 0.0049805486 },
    { 0.1739719298, 0.0049640831 },
    { 0.1739316786, 0.0049434066 },
    { 0.1738890358, 0.0049260485 },
    { 0.1738452562, 0.0049160931 },
    { 0.1738007726, 0.0049154119 },
    { 0.1737544380, 0.0049248537 },
    { 0.1737053527, 0.0049370984 },
    { 0.1736551894, 0.0049437910 },
    { 0.1736060182, 0.0049398953 },
    { 0.1735599065, 0.0049232026 },
    { 0.1735144497, 0.0048954467 },
    { 0.1734684982, 0.0048645791 },
    { 0.1734236662, 0.0048363121 },
    { 0.1733799960, 0.0048133383 },
    { 0.1733368655, 0.0047967434 },
    { 0.1732912857, 0.0047858456 },
    { 0.1732379205, 0.0047788879 },
    { 0.1731742388, 0.0047751308 },
    { 0.1731010122, 0.0047740307 },
    { 0.1730209655, 0.0047750504 },
    { 0.1729342569, 0.0047811472 },
    { 0.1728427561, 0.0047907929 },
    { 0.1727511526, 0.0047987621 },
    { 0.1726621056, 0.0048020844 },
    { 0.1725765508, 0.0047993019 },
    { 0.1724894774, 0.0047952544 },
    { 0.1723956034, 0.0047961186 },
    { 0.1722960018, 0.0048026295 },
    { 0.1721923604, 0.0048148852 },
    { 0.1720866308, 0.0048325242 },
    { 0.1719824459, 0.0048550102 },
    { 0.1718710194, 0.0048885319 },
    { 0.1717412137, 0.0049393325 },
    { 0.1715872394, 0.0050103442 },
    { 0.1714074339, 0.0051021710 },
    { 0.1712061135, 0.0052112578 },
    { 0.1709925742, 0.0053339078 },
    { 0.1707705964, 0.0054701212 },
    { 0.1705406619, 0.0056209699 },
    { 0.1703009888, 0.0057885050 },
    { 0.1700501587, 0.0059738951 },
    { 0.1697858688, 0.0061768075 },
    { 0.1695046025, 0.0063980369 },
    { 0.1692029217, 0.0066387059 },
    { 0.1688775207, 0.0069002439 },
    { 0.1685246603, 0.0071840439 },
    { 0.1681461455, 0.0074906797 },
    { 0.1677462198, 0.0078208185 },
    { 0.1673283257, 0.0081753997 },
    { 0.1668952904, 0.0085556064 },
    { 0.1664463271, 0.0089644004 },
    { 0.1659767582, 0.0094017162 },
    { 0.1654832990, 0.0098646810 },
    { 0.1649626637, 0.0103507435 },
    { 0.1644117564, 0.0108575583 },
    { 0.1638284328, 0.0113848656 },
    { 0.1632098960, 0.0119373858 },
    { 0.1625521395, 0.0125200299 },
    { 0.1618514381, 0.0131373071 },
    { 0.1611045796, 0.0137933588 },
    { 0.1603095950, 0.0144913782 },
    { 0.1594659458, 0.0152320646 },
    { 0.1585731111, 0.0160151564 },
    { 0.1576311656, 0.0168398710 },
    { 0.1566409326, 0.0177048050 },
    { 0.1556050956, 0.0186086065 },
    { 0.1545246125, 0.0195556978 },
    { 0.1533972293, 0.0205537335 },
    { 0.1522192362, 0.0216117110 },
    { 0.1509854084, 0.0227401933 },
    { 0.1496905648, 0.0239503302 },
    { 0.1483368171, 0.0252473984 },
    { 0.1469282265, 0.0266351859 },
    { 0.1454683718, 0.0281184333 },
    { 0.1439603960, 0.0297029703 },
    { 0.1424050902, 0.0313935840 },
    { 0.1407956467, 0.0332131546 },
    { 0.1391206824, 0.0352005728 },
    { 0.1373637579, 0.0374030904 },
    { 0.1355026712, 0.0398791215 },
    { 0.1335093410, 0.0426923900 },
    { 0.1313706352, 0.0458759752 },
    { 0.1290857866, 0.0494498107 },
    { 0.1266621570, 0.0534259198 },
    { 0.1241184767, 0.0578025134 },
    { 0.1214685839, 0.0625876721 },
    { 0.1187012765, 0.0678304435 },
    { 0.1158073588, 0.0735807080 },
    { 0.1127760548, 0.0798958229 },
    { 0.1095943236, 0.0868425112 },
    { 0.1062607353, 0.0944860722 },
    { 0.1027758629, 0.1028637388 },
    { 0.0991275999, 0.1120070330 },
    { 0.0953040562, 0.1219448633 },
    { 0.0912935070, 0.1327020425 },
    { 0.0870824317, 0.1443165827 },
    { 0.0826795345, 0.1568659581 },
    { 0.0781159857, 0.1704204865 },
    { 0.0734372599, 0.1850318805 },
    { 0.0687059213, 0.2007232177 },
    { 0.0639930237, 0.2174676054 },
    { 0.0593158280, 0.2352537402 },
    { 0.0546665229, 0.2540955907 },
    { 0.0500314971, 0.2740018032 },
    { 0.0453907347, 0.2949759646 },
    { 0.0407573153, 0.3169810808 },
    { 0.0361951092, 0.3398999344 },
    { 0.0317564704, 0.3635976932 },
    { 0.0274941905, 0.3879213283 },
    { 0.0234599425, 0.4127034791 },
    { 0.0197046363, 0.4377558887 },
    { 0.0162684713, 0.4629545080 },
    { 0.0131830412, 0.4882070684 },
    { 0.0104757007, 0.5134042452 },
    { 0.0081680280, 0.5384230705 },
    { 0.0062848516, 0.5630684563 },
    { 0.0048754300, 0.5871164380 },
    { 0.0039824254, 0.6104474976 },
    { 0.0036363842, 0.6330113828 },
    { 0.0038585209, 0.6548231511 },
    { 0.0046457132, 0.6758984586 },
    { 0.0060109131, 0.6961200613 },
    { 0.0079883958, 0.7153415163 },
    { 0.0106032906, 0.7334129427 },
    { 0.0138702461, 0.7501864280 },
    { 0.0177661242, 0.7656121544 },
    { 0.0222442057, 0.7796299232 },
    { 0.0272732624, 0.7921035028 },
    { 0.0328203575, 0.8029256730 },
    { 0.0388518024, 0.8120160214 },
    { 0.0453279848, 0.8193908005 },
    { 0.0521766909, 0.8251635426 },
    { 0.0593255334, 0.8294257763 },
    { 0.0667158860, 0.8322737393 },
    { 0.0743024248, 0.8338030913 },
    { 0.0820533952, 0.8340903145 },
    { 0.0899417396, 0.8332889189 },
    { 0.0979397501, 0.8315926665 },
    { 0.1060211073, 0.8291781866 },
    { 0.1141607196, 0.8262069598 },
    { 0.1223473670, 0.8227703996 },
    { 0.1305456681, 0.8189278529 },
    { 0.1387023492, 0.8147743826 },
    { 0.1467732157, 0.8103946065 },
    { 0.1547220612, 0.8058635454 },
    { 0.1625354247, 0.8012384804 },
    { 0.1702371955, 0.7965185422 },
    { 0.1778495280, 0.7916865791 },
    { 0.1853907574, 0.7867277728 },
    { 0.1928760979, 0.7816292164 },
    { 0.2003087981, 0.7763994161 },
    { 0.2076899897, 0.7710547987 },
    { 0.2150295500, 0.7655950961 },
    { 0.2223366038, 0.7600199997 },
    { 0.2296196726, 0.7543290899 },
    { 0.2368847206, 0.7485244652 },
    { 0.2441325565, 0.7426139917 },
    { 0.2513634089, 0.7366055814 },
    { 0.2585775085, 0.7305066019 },
    { 0.2657750850, 0.7243239249 },
    { 0.2729576035, 0.7180621864 },
    { 0.2801289425, 0.7117247346 },
    { 0.2872924091, 0.7053162739 },
    { 0.2944502809, 0.6988420220 },
    { 0.3016037994, 0.6923077624 },
    { 0.3087599231, 0.6857120606 },
    { 0.3159143944, 0.6790634800 },
    { 0.3230662654, 0.6723673980 },
    { 0.3302155454, 0.6656280254 },
    { 0.3373633329, 0.6588482901 },
    { 0.3445131984, 0.6520282092 },
    { 0.3516644113, 0.6451721742 },
    { 0.3588136867, 0.6382873365 },
    { 0.3659593573, 0.6313790809 },
    { 0.3731015439, 0.6244508598 },
    { 0.3802438355, 0.6175021522 },
    { 0.3873789780, 0.6105418025 },
    { 0.3945065488, 0.6035713368 },
    { 0.4016259188, 0.5965924220 },
    { 0.4087362557, 0.5896068689 },
    { 0.4158357747, 0.5826179681 },
    { 0.4229209267, 0.5756306883 },
    { 0.4299886265, 0.5686488913 },
    { 0.4370364226, 0.5616757740 },
    { 0.4440624636, 0.5547139028 },
    { 0.4510649410, 0.5477660441 },
    { 0.4580406656, 0.5408366292 },
    { 0.4649863330, 0.5339300531 },
    { 0.4718987439, 0.5270505692 },
    { 0.4787747912, 0.5202023072 },
    { 0.4856115871, 0.5133886610 },
    { 0.4924049823, 0.5066149244 },
    { 0.4991506683, 0.4998873404 },
    { 0.5058452838, 0.4932111781 },
    { 0.5124863668, 0.4865907881 },
    { 0.5190725104, 0.4800286122 },
    { 0.5256004890, 0.4735273740 },
    { 0.5320655992, 0.4670913637 },
    { 0.5384627619, 0.4607252538 },
    { 0.5447865056, 0.4544341146 },
    { 0.5510310502, 0.4482245029 },
    { 0.5571929061, 0.4420991395 },
    { 0.5632693124, 0.4360580617 },
    { 0.5692568241, 0.4301019736 },
    { 0.5751513114, 0.4242322349 },
    { 0.5809526052, 0.4184468798 },
    { 0.5866501869, 0.4127584212 },
    { 0.5922248001, 0.4071895286 },
    { 0.5976581621, 0.4017619350 },
    { 0.6029327856, 0.3964966336 },
    { 0.6080351111, 0.3914091517 },
    { 0.6129769996, 0.3864861573 },
    { 0.6177787256, 0.3817057568 },
    { 0.6224592951, 0.3770472864 },
    { 0.6270365998, 0.3724911452 },
    { 0.6315209429, 0.3680260108 },
    { 0.6358998196, 0.3636654024 },
    { 0.6401561595, 0.3594277243 },
    { 0.6442729607, 0.3553313698 },
    { 0.6482331060, 0.3513949163 },
    { 0.6520282357, 0.3476279607 },
    { 0.6556691792, 0.3440182948 },
    { 0.6591661347, 0.3405532254 },
    { 0.6625282221, 0.3372209926 },
    { 0.6657635762, 0.3340106512 },
    { 0.6688741437, 0.3309185530 },
    { 0.6718586671, 0.3279470743 },
    { 0.6747195111, 0.3250951820 },
    { 0.6774588883, 0.3223620767 },
    { 0.6800788497, 0.3197472171 },
    { 0.6825815742, 0.3172487060 },
    { 0.6849706014, 0.3148628150 },
    { 0.6872504546, 0.3125859640 },
    { 0.6894263030, 0.3104140113 },
    { 0.6915039730, 0.3083422606 },
    { 0.6934896350, 0.3063656908 },
    { 0.6953886381, 0.3044785552 },
    { 0.6972055698, 0.3026750727 },
    { 0.6989439104, 0.3009504250 },
    { 0.7006060606, 0.2993006993 },
    { 0.7021925885, 0.2977245119 },
    { 0.7037086910, 0.2962171181 },
    { 0.7051628534, 0.2947702921 },
    { 0.7065632467, 0.2933761532 },
    { 0.7079177916, 0.2920271089 },
    { 0.7092309854, 0.2907186222 },
    { 0.7105003945, 0.2894529412 },
    { 0.7117241462, 0.2882321048 },
    { 0.7129012311, 0.2870573204 },
    { 0.7140315971, 0.2859288735 },
    { 0.7151170535, 0.2848451061 },
    { 0.7161591986, 0.2838044492 },
    { 0.7171586136, 0.2828064119 },
    { 0.7181161426, 0.2818502566 },
    { 0.7190329416, 0.2809349515 },
    { 0.7199115529, 0.2800580782 },
    { 0.7207527066, 0.2792189598 },
    { 0.7215545225, 0.2784195147 },
    { 0.7223149156, 0.2776618704 },
    { 0.7230316026, 0.2769483577 },
    { 0.7237019160, 0.2762818362 },
    { 0.7243280189, 0.2756600749 },
    { 0.7249144051, 0.2750781841 },
    { 0.7254667761, 0.2745299780 },
    { 0.7259923175, 0.2740076825 },
    { 0.7264947267, 0.2735052733 },
    { 0.7269749705, 0.2730250295 },
    { 0.7274318380, 0.2725681620 },
    { 0.7278643108, 0.2721356892 },
    { 0.7282717283, 0.2717282717 },
    { 0.7286564871, 0.2713435129 },
    { 0.7290200303, 0.2709799697 },
    { 0.7293609507, 0.2706390493 },
    { 0.7296777832, 0.2703222168 },
    { 0.7299690128, 0.2700309872 },
    { 0.7302339491, 0.2697660509 },
    { 0.7304741653, 0.2695258347 },
    { 0.7306933067, 0.2693066933 },
    { 0.7308962522, 0.2691037478 },
    { 0.7310893956, 0.2689106044 },
    { 0.7312796359, 0.2687203641 },
    { 0.7314670509, 0.2685329491 },
    { 0.7316499709, 0.2683500291 },
    { 0.7318263335, 0.2681736665 },
    { 0.7319932998, 0.2680067002 },
    { 0.7321504222, 0.2678495778 },
    { 0.7322998311, 0.2677001689 },
    { 0.7324428229, 0.2675571771 },
    { 0.7325814936, 0.2674185064 },
    { 0.7327188940, 0.2672811060 },
    { 0.7328586473, 0.2671413527 },
    { 0.7330002053, 0.2669997947 },
    { 0.7331416711, 0.2668583289 },
    { 0.7332811789, 0.2667188211 },
    { 0.7334169672, 0.2665830328 },
    { 0.7335505858, 0.2664494142 },
    { 0.7336832964, 0.2663167036 },
    { 0.7338127167, 0.2661872833 },
    { 0.7339356903, 0.2660643097 },
    { 0.7340473003, 0.2659526997 },
    { 0.7341425569, 0.2658574431 },
    { 0.7342214703, 0.2657785297 },
    { 0.7342864464, 0.2657135536 },
    { 0.7343409200, 0.2656590800 },
    { 0.7343901650, 0.2656098350 },
    { 0.7344377127, 0.2655622873 },
    { 0.7344821704, 0.2655178296 },
    { 0.7345229306, 0.2654770694 },
    { 0.7345595184, 0.2654404816 },
    { 0.7345916616, 0.2654083384 },
    { 0.7346210947, 0.2653789053 },
    { 0.7346488968, 0.2653511032 },
    { 0.7346733780, 0.2653266220 },
    { 0.7346900454, 0.2653099546 },
    { 0.7346900233, 0.2653099767 },
    { 0.7346899871, 0.2653100129 },
    { 0.7346900065, 0.2653099935 },
    { 0.7346900066, 0.2653099934 },
    { 0.7346899973, 0.2653100027 },
    { 0.7346900103, 0.2653099897 },
    { 0.7346900018, 0.2653099982 },
    { 0.7346899899, 0.2653100101 },
    { 0.7346899975, 0.2653100025 },
    { 0.7346900150, 0.2653099850 },
    { 0.7346899882, 0.2653100118 },
    { 0.7346900140, 0.2653099860 },
    { 0.7346900168, 0.2653099832 },
    { 0.7346899873, 0.2653100127 },
    { 0.7346899925, 0.2653100075 },
    { 0.7346900137, 0.2653099863 },
    { 0.7346900176, 0.2653099824 },
    { 0.7346900031, 0.2653099969 },
    { 0.7346899834, 0.2653100166 },
    { 0.7346899761, 0.2653100239 },
    { 0.7346900041, 0.2653099959 },
    { 0.7346899877, 0.2653100123 },
    { 0.7346899744, 0.2653100256 },
    { 0.7346899848, 0.2653100152 },
    { 0.7346899568, 0.2653100432 },
    { 0.7346899996, 0.2653100004 },
    { 0.7346899764, 0.2653100236 },
    { 0.7346899554, 0.2653100446 },
    { 0.7346900300, 0.2653099700 },
    { 0.7346899780, 0.2653100220 },
    { 0.7346899520, 0.2653100480 },
    { 0.7346900330, 0.2653099670 },
    { 0.7346899579, 0.2653100421 },
    { 0.7346899404, 0.2653100596 },
    { 0.7346900833, 0.2653099167 },
    { 0.7346899925, 0.2653100075 },
    { 0.7346899933, 0.2653100067 },
    { 0.7346900041, 0.2653099959 },
    { 0.7346899885, 0.2653100115 },
    { 0.7346899970, 0.2653100030 },
    { 0.7346900057, 0.2653099943 },
    { 0.7346900011, 0.2653099989 },
    { 0.7346899860, 0.2653100140 },
    { 0.7346900032, 0.2653099968 },
    { 0.7346899860, 0.2653100140 },
    { 0.7346900002, 0.2653099998 },
    { 0.7346899873, 0.2653100127 },
    { 0.7346900044, 0.2653099956 },
    { 0.7346899861, 0.2653100139 },
    { 0.7346900224, 0.2653099776 },
    { 0.7346900107, 0.2653099893 },
    { 0.7346899955, 0.2653100045 },
    { 0.7346900316, 0.2653099684 },
    { 0.7346900170, 0.2653099830 },
    { 0.7346900202, 0.2653099798 },
    { 0.7346900018, 0.2653099982 },
    { 0.7346899699, 0.2653100301 },
    { 0.7346900215, 0.2653099785 },
    { 0.7346899885, 0.2653100115 },
    { 0.7346900109, 0.2653099891 },
    { 0.7346899520, 0.2653100480 },
    { 0.7346899585, 0.2653100415 },
    { 0.7346900519, 0.2653099481 },
    { 0.7346899877, 0.2653100123 },
    { 0.7346899555, 0.2653100445 },
    { 0.7346899188, 0.2653100812 },
    { 0.7346900536, 0.2653099464 },
    { 0.7346900904, 0.2653099096 },
    { 0.7346900063, 0.2653099937 },
    { 0.7346899937, 0.2653100063 },
    { 0.7346899990, 0.2653100010 },
    { 0.7346900030, 0.2653099970 },
    { 0.7346900127, 0.2653099873 },
    { 0.7346899879, 0.2653100121 },
    { 0.7346900081, 0.2653099919 },
    { 0.7346899852, 0.2653100148 },
    { 0.7346900012, 0.2653099988 },
    { 0.7346899868, 0.2653100132 },
    { 0.7346900094, 0.2653099906 },
    { 0.7346899914, 0.2653100086 },
    { 0.7346899837, 0.2653100163 },
    { 0.7346900220, 0.2653099780 },
    { 0.7346899738, 0.2653100262 },
    { 0.7346900006, 0.2653099994 },
    { 0.7346900034, 0.2653099966 },
    { 0.7346899852, 0.2653100148 },
    { 0.7346899987, 0.2653100013 },
    { 0.7346899688, 0.2653100312 },
    { 0.7346900009, 0.2653099991 },
    { 0.7346900322, 0.2653099678 },
    { 0.7346899540, 0.2653100460 },
    { 0.7346899627, 0.2653100373 },
    { 0.7346900220, 0.2653099780 },
    { 0.7346899980, 0.2653100020 },
    { 0.7346900217, 0.2653099783 },
    { 0.7346899746, 0.2653100254 },
    { 0.7346900711, 0.2653099289 },
    { 0.7346900386, 0.2653099614 },
    { 0.7346900723, 0.2653099277 },
    { 0.7346899459, 0.2653100541 },
    { 0.7346899880, 0.2653100120 },
    { 0.7346900035, 0.2653099965 },
    { 0.7346900080, 0.2653099920 },
    { 0.7346900048, 0.2653099952 },
    { 0.7346899926, 0.2653100074 },
    { 0.7346899908, 0.2653100092 },
    { 0.7346899913, 0.2653100087 },
    { 0.7346900147, 0.2653099853 },
    { 0.7346899929, 0.2653100071 },
    { 0.7346899911, 0.2653100089 },
    { 0.7346899978, 0.2653100022 },
    { 0.7346900133, 0.2653099867 },
    { 0.7346900106, 0.2653099894 },
    { 0.7346900170, 0.2653099830 },
    { 0.7346899903, 0.2653100097 },
    { 0.7346900095, 0.2653099905 },
    { 0.7346899755, 0.2653100245 },
    { 0.7346899713, 0.2653100287 },
    { 0.7346899975, 0.2653100025 },
    { 0.7346899818, 0.2653100182 },
    { 0.7346899843, 0.2653100157 },
    { 0.7346900649, 0.2653099351 },
    { 0.7346900409, 0.2653099591 },
    { 0.7346901069, 0.2653098931 },
    { 0.7346899511, 0.2653100489 },
    { 0.7346899698, 0.2653100302 },
    { 0.7346898490, 0.2653101510 },
    { 0.7346898573, 0.2653101427 },
    { 0.7346900443, 0.2653099557 },
    { 0.7346901793, 0.2653098207 },
    { 0.7346899588, 0.2653100412 },
    { 0.7346899588, 0.2653100412 },
    { 0.7232791480, 0.2600035881 },
    { 0.7118683373, 0.2546971349 },
    { 0.7004575265, 0.2493906818 },
    { 0.6890467158, 0.2440842286 },
    { 0.6776359050, 0.2387777755 },
    { 0.6662250942, 0.2334713223 },
    { 0.6548142835, 0.2281648692 },
    { 0.6434034727, 0.2228584160 },
    { 0.6319926620, 0.2175519629 },
    { 0.6205818512, 0.2122455097 },
    { 0.6091710405, 0.2069390566 },
    { 0.5977602297, 0.2016326035 },
    { 0.5863494190, 0.1963261503 },
    { 0.5749386082, 0.1910196972 },
    { 0.5635277974, 0.1857132440 },
    { 0.5521169867, 0.1804067909 },
    { 0.5407061759, 0.1751003377 },
    { 0.5292953652, 0.1697938846 },
    { 0.5178845544, 0.1644874314 },
    { 0.5064737437, 0.1591809783 },
    { 0.4950629329, 0.1538745251 },
    { 0.4836521222, 0.1485680720 },
    { 0.4722413114, 0.1432616188 },
    { 0.4608305006, 0.1379551657 },
    { 0.4494196899, 0.1326487125 },
    { 0.4380088791, 0.1273422594 },
    { 0.4265980684, 0.1220358062 },
    { 0.4151872576, 0.1167293531 },
    { 0.4037764469, 0.1114229000 },
    { 0.3923656361, 0.1061164468 },
    { 0.3809548254, 0.1008099937 },
    { 0.3695440146, 0.0955035405 },
    { 0.3581332038, 0.0901970874 },
    { 0.3467223931, 0.0848906342 },
    { 0.3353115823, 0.0795841811 },
    { 0.3239007716, 0.0742777279 },
    { 0.3124899608, 0.0689712748 },
    { 0.3010791501, 0.0636648216 },
    { 0.2896683393, 0.0583583685 },
    { 0.2782575286, 0.0530519153 },
    { 0.2668467178, 0.0477454622 },
    { 0.2554359070, 0.0424390090 },
    { 0.2440250963, 0.0371325559 },
    { 0.2326142855, 0.0318261027 },
    { 0.2212034748, 0.0265196496 },
    { 0.2097926640, 0.0212131965 },
    { 0.1983818533, 0.0159067433 },
    { 0.1869710425, 0.0106002902 },
    { 0.1755602318, 0.0052938370 },
};

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

void printMonochromatic()
{
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

// CIE xy coordinate to QGraphicsScene pos bounded by plotArea.
QPointF xyToScenePos(QPointF xy, QRectF plotArea, QPointF plotRange)
{
    return QPointF(xy.x() / plotRange.x() * plotArea.width() + plotArea.left(),
                  ((plotRange.y() - xy.y()) / plotRange.y()) * plotArea.height() + plotArea.top());
}

// A Color item which is rendered as a circle on the diagram
class ChromaticityColorItem : public QGraphicsEllipseItem
{
public:
    ChromaticityColorItem()
    :QGraphicsEllipseItem()
    {
        setRect(-5, -5, 10, 10);
        setOpacity(0.5);

        QPen cosmetic(QColor(50,50,50));
        cosmetic.setWidth(2);
        cosmetic.setCosmetic(true);
        setPen(cosmetic);

        setBrush(QColor(Qt::gray));
    }

    // Set RGB color
    void setColor(QColor color, RgbColorSpace colorSpace)
    {
        auto Yxy = RGBtoYxy(color, colorSpace);
        setColor(Yxy(1, 0), Yxy(2, 0));

        setRenderColor(color);
    }

    // Set xy color (CIE xyY)
    void setColor(qreal x, qreal y)
    {
        m_xy = QPointF(x, y);
        setScenePos();
    }

    void setPlotArea(QRectF plotArea, QPointF plotRange)
    {
        m_plotArea = plotArea;
        m_plotRange = plotRange;
        setScenePos();
    }

private:
    void setRenderColor(QColor color)
    {
        setBrush(color);
    }

    void setScenePos()
    {
        if (m_xy.x() == -1 || m_plotArea.isEmpty())
            return;

        setPos(xyToScenePos(m_xy, m_plotArea, m_plotRange));
    }

    QPointF m_xy;
    QRectF m_plotArea;
    QPointF m_plotRange;
};

// A Color Profile item which is rendered as a triangle on the diagram
class ChromaticityColorProfileItem
{
public:
    ChromaticityColorProfileItem()
    {
        for (int i = 0; i < 3; ++i) {
            QGraphicsLineItem *lineItem = new QGraphicsLineItem();
            QPen cosmetic(QColor(20,20,20));
            cosmetic.setWidth(1.5);
            cosmetic.setCosmetic(true);
            lineItem->setPen(cosmetic);
            m_lineItems.append(lineItem);
        }

        m_titleItem = new QGraphicsSimpleTextItem();
        QFont font;
        font.setPointSize(10);
        m_titleItem->setFont(font);
        m_titleItem->setBrush(QColor(20, 20, 20));
        m_titleItem->setText("sRGB");
    }

    ~ChromaticityColorProfileItem()
    {
        qDeleteAll(m_lineItems);
        m_lineItems.clear();
    }

    // Set RGB color
    void setColorSpace(RgbColorSpace colorSpace)
    {
        if (m_titleItem)
            m_titleItem->setText(colorSpaceText[colorSpace]);

        // Get primaries
        auto red   = RGBtoYxy(QColor(Qt::red), colorSpace);
        auto green = RGBtoYxy(QColor(Qt::green), colorSpace);
        auto blue  = RGBtoYxy(QColor(Qt::blue), colorSpace);

        // Set xy positions
        m_xy[0] = QPointF(red(1, 0), red(2, 0));
        m_xy[1] = QPointF(green(1, 0), green(2, 0));
        m_xy[2] = QPointF(blue(1, 0), blue(2, 0));

        setScenePos();
    }

    void setPlotArea(QRectF plotArea, QPointF plotRange)
    {
        m_plotArea = plotArea;
        m_plotRange = plotRange;
        setScenePos();
    }

    void addItems(QGraphicsScene *scene)
    {
        m_scene = scene;
        for (auto item : m_lineItems)
            m_scene->addItem(item);
        if (m_titleItem)
            m_scene->addItem(m_titleItem);
    }

    void removeItems()
    {
        if (m_scene == nullptr)
            return;
        for (auto item : m_lineItems)
            m_scene->removeItem(item);
        if (m_titleItem)
            m_scene->removeItem(m_titleItem);
    }

private:
    void setScenePos()
    {
        if (m_plotArea.isEmpty())
            return;

        // Create gamut triangle
        m_lineItems[0]->setLine(QLineF(xyToScenePos(m_xy[0], m_plotArea, m_plotRange), xyToScenePos(m_xy[1], m_plotArea, m_plotRange)));
        m_lineItems[1]->setLine(QLineF(xyToScenePos(m_xy[1], m_plotArea, m_plotRange), xyToScenePos(m_xy[2], m_plotArea, m_plotRange)));
        m_lineItems[2]->setLine(QLineF(xyToScenePos(m_xy[2], m_plotArea, m_plotRange), xyToScenePos(m_xy[0], m_plotArea, m_plotRange)));

        // Label gamut triangle
        if (m_titleItem) {
            QPointF aboveGreen(xyToScenePos(m_xy[1], m_plotArea, m_plotRange) + QPointF(-15, -15));
            m_titleItem->setPos(aboveGreen);
        }
    }

    QPointF m_xy[3];
    QRectF m_plotArea;
    QPointF m_plotRange;
    QList<QGraphicsLineItem *>m_lineItems;
    QGraphicsSimpleTextItem *m_titleItem;
    QGraphicsScene *m_scene = nullptr;
};


class ChromaticityDiagram : public QGraphicsView
{
private:
    QPointF m_plotRangeMinimum = QPointF(0.8, 0.9);;
    QPointF m_plotRange = m_plotRangeMinimum;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;

public:
    ChromaticityDiagram() {
        setWindowTitle("Chromaticity Diagram");

        // Create Graphics scene with chart for axes
        m_scene = new QGraphicsScene(this);
        setScene(m_scene);
        m_scene->setBackgroundBrush(QColor(240, 240, 240));

        m_chart = new QChart();
        m_scene->addItem(m_chart);
        m_chart->setGeometry(QRectF(QPointF(0,0), QSizeF(size())));
        m_chart->legend()->hide();
        m_chart->setTitle("CIE Chromaticity");
        QLineSeries *series = new QLineSeries();
        m_chart->addSeries(series);

        m_axisX = new QValueAxis;
        m_axisX->setRange(0, m_plotRange.x());
        m_axisX->setTickCount(9);
        m_axisX->setLabelFormat("%g");
        m_axisX->setTitleText("CIE x");
        m_chart->setAxisX(m_axisX, series);

        m_axisY = new QValueAxis;
        m_axisY->setRange(0, m_plotRange.y());
        m_axisY->setTickCount(10);
        m_axisX->setLabelFormat("%g");
        m_axisY->setTitleText("CIE y");
        m_chart->setAxisY(m_axisY, series);

        // Diagram background (monochromoatic light outline, color
        // gradient fill) is drawn on a QImage using QPainter, and
        // displayed on the scene using this pixmap item.
        QGraphicsPixmapItem *pix = new QGraphicsPixmapItem();
        m_scene->addItem(pix);

        // Create monochromatic light "horseshoe" shape from data table
        QPainterPath path;
        path.moveTo(monochromatic_xy[0][0], monochromatic_xy[0][1]);
        for (int i = 1; i < entries; i+=1) {
            path.lineTo(monochromatic_xy[i][0], monochromatic_xy[i][1]);
        }

        // Update diagram background on plot area change.
        connect(m_chart, &QChart::plotAreaChanged, [this, pix, path](const QRectF &plotArea){

            // Create cache image for drawing the xy plot, filled with transparent pixels
            QSize imageSize = plotArea.size().toSize();
            qreal dpr = devicePixelRatioF();
            QImage xypolot = QImage(imageSize * dpr, QImage::Format_ARGB32_Premultiplied);
            xypolot.setDevicePixelRatio(dpr);
            xypolot.fill(QColor(0, 0, 0, 0));

            {
                // Create painter, scaled to have a logical coordinate system in the
                // 0..m_plotRange.x()/m_plotRange.y(), with the origin at the bottom right.
                QPainter p(&xypolot);
                p.setRenderHint(QPainter::Antialiasing, true);
                p.scale(imageSize.width() / m_plotRange.x(), imageSize.height() / m_plotRange.y());
                p.scale(1, -1);
                p.translate(0, -m_plotRange.y());

                // Draw monochromatic light "horseshoe" outline
                QPen cosmetic(QColor(50,50,50));
                cosmetic.setWidth(2);
                cosmetic.setCosmetic(true);
                p.strokePath(path, cosmetic);
            }

            // Fill horseshoe interior with color
            int xypolotHeight = xypolot.height();
            for (int l = 0; l < xypolotHeight; ++l) {
                int scanLinePixels = xypolot.bytesPerLine() / 4;
                QRgb* scanline = (QRgb*)xypolot.scanLine(l);

                bool inside = false;
                bool online = false;
                QRgb* begin = nullptr;
                QRgb* end = nullptr;

                for (int p = 0; p < scanLinePixels; ++p) {
                    QRgb* pixel = scanline + p;
                    bool signal = qBlue(*pixel) > 10;

                    // transition to inside on falling edge
                    if (!inside && online && !signal) {
                        begin = pixel;
                        inside = true;
                    }

                    // transition to outside on rising edge
                    if (inside && !online && signal) {
                        end = pixel;
                        break; // only fill once
                    }

                    online = signal;
                }

                // Don't fill if there was no beginning or no end of area
                if (begin == nullptr || end == nullptr)
                    continue;

                // Fill line with RGB color corresponding to the CIE x,y coordinate
                qreal CIE_y = m_plotRange.y() * ((qreal(xypolotHeight) - qreal(l)) / qreal(xypolotHeight));
                for (QRgb *pixel = begin; pixel <= end; ++pixel) {
                    qreal CIE_Y = 1;
                    qreal CIE_x = m_plotRange.x() * qreal(pixel - scanline) / xypolot.width();
                    qreal data[] = { CIE_Y, CIE_x, CIE_y };
                    QGenericMatrix<1, 3, qreal> Yxy(data);
                    QColor color = toColor(YxyToLinearRGB(Yxy, ProPhotoRGB));
                    *pixel = color.rgba();
                }
            }

            // Update pixmap item with image and postion
            pix->setPixmap(QPixmap::fromImage(xypolot));
            pix->setOpacity(0.8);
            QPointF itemPosition = plotArea.topLeft();
            pix->setPos(itemPosition);
            xypolot = QImage();
        });


        // Update color item positions on resize
        connect(m_chart, &QChart::plotAreaChanged, [this](const QRectF &plotArea){
            for (ChromaticityColorItem *item : m_colorItems)
                item->setPlotArea(plotArea, m_plotRange);
            for (ChromaticityColorProfileItem *item : m_colorProfileItems)
                item->setPlotArea(plotArea, m_plotRange);
        });


        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setStyleSheet( "QGraphicsView { border-style: none; }" );
        setRenderHints(QPainter::Antialiasing);

        // Enable zoom on pinch
        grabGesture(Qt::PinchGesture);

//        QGLWidget *viewPort = new QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::AlphaChannel));
//        setViewport(viewPort);
    }

    void setPlotRange(QPointF plotRange) {
        // Zooming in breaks the color shading alogrithm, prevent it
        if (plotRange.x() < m_plotRangeMinimum.x())
            plotRange = m_plotRangeMinimum;

        m_plotRange = plotRange;
        m_axisX->setRange(0, m_plotRange.x());
        m_axisY->setRange(0, m_plotRange.y());
        m_scene->update(this->sceneRect());
    }

    bool event(QEvent *event)
    {
        if (event->type() == QEvent::Gesture)
            return gestureEvent(static_cast<QGestureEvent*>(event));
        return QGraphicsView::event(event);
    }

    bool gestureEvent(QGestureEvent *event) {
        if (QGesture *pinch = event->gesture(Qt::PinchGesture))
            return pinchGestureEvent(static_cast<QPinchGesture *>(pinch));

        return false;
    }

    bool pinchGestureEvent(QPinchGesture *gesture) {
        QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
        if (changeFlags & QPinchGesture::ScaleFactorChanged) {
            setPlotRange(m_plotRange * 1.0f / gesture->scaleFactor());
        }
        return true;
    }

    void resizeEvent(QResizeEvent *ev) {
        m_chart->setGeometry(QRectF(QPointF(0,0), QSizeF(ev->size())));
        QGraphicsView::resizeEvent(ev);
    }


    void addColorItem(ChromaticityColorItem *colorItem) {
        m_scene->addItem(colorItem);
        colorItem->setPlotArea(m_chart->plotArea(), m_plotRange);
        m_colorItems.append(colorItem);
    }

    void clearColorItems()
    {
       for (ChromaticityColorItem *item : m_colorItems)
           m_scene->removeItem(item);
       qDeleteAll(m_colorItems);
       m_colorItems.clear();
    }

    void addColorProfileItem(ChromaticityColorProfileItem *colorProfileItem)
    {
        colorProfileItem->addItems(m_scene);
        colorProfileItem->setPlotArea(m_chart->plotArea(), m_plotRange);
        m_colorProfileItems.append(colorProfileItem);
    }

private:
    QGraphicsScene *m_scene;
    QChart *m_chart;
    QList<ChromaticityColorItem *> m_colorItems;
    QList<ChromaticityColorProfileItem *> m_colorProfileItems;
};

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
