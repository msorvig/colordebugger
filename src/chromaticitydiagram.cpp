#include "chromaticitydiagram.h"

#include "colorconvert.h"

// chromaticitydiagram_data.cpp
extern int begin_wl;
extern int end_wl;
extern int entries;
extern qreal monochromatic_xy[521][2];

// CIE xy coordinate to QGraphicsScene pos bounded by plotArea.
QPointF xyToScenePos(QPointF xy, QRectF plotArea, QPointF plotRange)
{
    return QPointF(xy.x() / plotRange.x() * plotArea.width() + plotArea.left(),
                  ((plotRange.y() - xy.y()) / plotRange.y()) * plotArea.height() + plotArea.top());
}

ChromaticityDiagram::ChromaticityDiagram() {
    setWindowTitle("Chromaticity Diagram");

    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    m_scene->setBackgroundBrush(QColor(240, 240, 240));

    // Add QChart to display aces
    m_chart = new QChart();
    m_scene->addItem(m_chart);
    m_chart->setFlags(m_chart->flags() & ~QGraphicsItem::ItemIsMovable);
    m_chart->setGeometry(QRectF(QPointF(0,0), QSizeF(size())));
    m_chart->legend()->hide();
    m_chart->setTitle("CIE 1931 xy Chromaticity");
    m_chart->layout()->setContentsMargins(0, 0, 0, 0);
    m_chart->setBackgroundRoundness(0);

    QLineSeries *series = new QLineSeries();
    m_chart->addSeries(series);

    m_axisX = new QValueAxis;
    m_axisX->setRange(0, m_plotRange.x());
    m_axisX->setTickCount(9);
    m_axisX->setLabelFormat("%g");
    m_axisX->setTitleText("x");
    m_chart->setAxisX(m_axisX, series);

    m_axisY = new QValueAxis;
    m_axisY->setRange(0, m_plotRange.y());
    m_axisY->setTickCount(10);
    m_axisX->setLabelFormat("%g");
    m_axisY->setTitleText("y");
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
        
        // A RGB color space that covers approxemately the entire chromaticity chart.
        RGBColorSpace allColors( (qreal []){0.74, 0.25}, (qreal []){0.05, 0.85}, (qreal []){0.17, 0.0}, 1.0, "allColors");

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

            // Fill line with RGB color corresponding to the CIE xy coordinate
            qreal CIE_y = m_plotRange.y() * ((qreal(xypolotHeight) - qreal(l)) / qreal(xypolotHeight));
            for (QRgb *pixel = begin; pixel <= end; ++pixel) {
                qreal CIE_Y = 1;
                qreal CIE_x = m_plotRange.x() * qreal(pixel - scanline) / xypolot.width();
                QGenericMatrix<1, 3, qreal> Yxy((qreal[]){ CIE_Y, CIE_x, CIE_y });
                QColor color = allColors.convertYxyToRGB(Yxy);
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
}

void ChromaticityDiagram::setPlotRange(QPointF plotRange) {
    // Zooming in breaks the color shading alogrithm, prevent it
    if (plotRange.x() < m_plotRangeMinimum.x())
        plotRange = m_plotRangeMinimum;

    m_plotRange = plotRange;
    m_axisX->setRange(0, m_plotRange.x());
    m_axisY->setRange(0, m_plotRange.y());
    m_scene->update(this->sceneRect());
}

bool ChromaticityDiagram::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture)
        return gestureEvent(static_cast<QGestureEvent*>(event));
    return QGraphicsView::event(event);
}

bool ChromaticityDiagram::gestureEvent(QGestureEvent *event) {
    if (QGesture *pinch = event->gesture(Qt::PinchGesture))
        return pinchGestureEvent(static_cast<QPinchGesture *>(pinch));

    return false;
}

bool ChromaticityDiagram::pinchGestureEvent(QPinchGesture *gesture) {
    QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
    if (changeFlags & QPinchGesture::ScaleFactorChanged) {
        setPlotRange(m_plotRange * 1.0f / gesture->scaleFactor());
    }
    return true;
}

void ChromaticityDiagram::resizeEvent(QResizeEvent *ev) {
    m_chart->setGeometry(QRectF(QPointF(0,0), QSizeF(ev->size())));
    QGraphicsView::resizeEvent(ev);
}

void ChromaticityDiagram::wheelEvent(QWheelEvent *event)
{
#ifdef Q_OS_WASM
     // wasm: don't scroll on wheel
    Q_UNUSED(event);
#else 
    QGraphicsView::wheelEvent(event)
#endif
}

void ChromaticityDiagram::addColorItem(ChromaticityColorItem *colorItem) {
    m_scene->addItem(colorItem);
    colorItem->setPlotArea(m_chart->plotArea(), m_plotRange);
    m_colorItems.append(colorItem);
}

void ChromaticityDiagram::clearColorItems()
{
   for (ChromaticityColorItem *item : m_colorItems)
       m_scene->removeItem(item);
   qDeleteAll(m_colorItems);
   m_colorItems.clear();
}

void ChromaticityDiagram::addColorProfileItem(ChromaticityColorProfileItem *colorProfileItem)
{
    colorProfileItem->addItems(m_scene);
    colorProfileItem->setPlotArea(m_chart->plotArea(), m_plotRange);
    m_colorProfileItems.append(colorProfileItem);
}


ChromaticityColorItem::ChromaticityColorItem()
:QGraphicsEllipseItem()
{
    setRect(-5, -5, 10, 10);
    setOpacity(0.5);

    QPen cosmetic(QColor(50,50,50));
    cosmetic.setWidth(2);
    cosmetic.setCosmetic(true);
    setPen(cosmetic);

    setBrush(QColor(Qt::gray));

/*

    rgbColor >> [this](tuple<QColor, RGBColorSpace> colorSpec) -> tuple<qreal, qreal> {

        QColor color = get<0>(colorSpec);
        RGBColorSpace space = get<1>(colorSpec);

        auto Yxy = colorSpace.convertRGBtoYxy(color);
        qreal Y = Yxy(0, 0);
        qreal x = Yxy(1, 0);
        qreal y = Yxy(2, 0);
        bool isValidColor = color.isValid() && !std::isnan(x) && !std::isnan(y);

        setVisible(isValidColor);
        if (!isValidColor)
            return make_tuple(0, 0);

        setOpacity((Y > 0) ? 0.5 : 0.2);
        setRenderColor(color);

        return make_tuple(x, y);

    } >> xyColor;

    xyColor >> [this](tuple<qreal, qreal> xyColor) {

        m_xy = QPointF(x, y);
        setScenePos();

    };
*/
}

// Set RGB color
void ChromaticityColorItem::setColor(QColor color, RGBColorSpace colorSpace)
{
    setVisible(color.isValid());
    if (color.isValid() == false)
        return;

    auto Yxy = colorSpace.convertRGBtoYxy(color);
    if (std::isnan(Yxy(1, 0)) || std::isnan(Yxy(2, 0))) {
        setVisible(false);
        return;
    }

    qreal Y = Yxy(0, 0);
    setOpacity((Y > 0) ? 0.5 : 0.2);

    setColor(Yxy(1, 0), Yxy(2, 0));
    setRenderColor(color);
}

// Set xy color (CIE xyY)
void ChromaticityColorItem::setColor(qreal x, qreal y)
{
    m_xy = QPointF(x, y);
    setScenePos();
}

void ChromaticityColorItem::setPlotArea(QRectF plotArea, QPointF plotRange)
{
    m_plotArea = plotArea;
    m_plotRange = plotRange;
    setScenePos();
}

void ChromaticityColorItem::setRenderColor(QColor color)
{
    setBrush(color);
}

void ChromaticityColorItem::setScenePos()
{
    if (m_xy.x() == -1 || m_plotArea.isEmpty())
        return;

    setPos(xyToScenePos(m_xy, m_plotArea, m_plotRange));
}

ChromaticityColorProfileItem::ChromaticityColorProfileItem()
{
    for (int i = 0; i < 3; ++i) {
        QGraphicsLineItem *lineItem = new QGraphicsLineItem();
        QPen cosmetic(QColor(20,20,20));
        cosmetic.setWidth(1);
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

ChromaticityColorProfileItem::~ChromaticityColorProfileItem()
{
    qDeleteAll(m_lineItems);
    m_lineItems.clear();
    delete m_titleItem;
}

void ChromaticityColorProfileItem::setColorSpace(RGBColorSpace colorSpace)
{
    if (m_titleItem)
        m_titleItem->setText(colorSpace.name());

    // Get primaries
    auto red   = colorSpace.convertRGBtoYxy(QColor(Qt::red));
    auto green = colorSpace.convertRGBtoYxy(QColor(Qt::green));
    auto blue  = colorSpace.convertRGBtoYxy(QColor(Qt::blue));

    // Set xy positions
    m_xy[0] = QPointF(red(1, 0), red(2, 0));
    m_xy[1] = QPointF(green(1, 0), green(2, 0));
    m_xy[2] = QPointF(blue(1, 0), blue(2, 0));

    setScenePos();
}

void ChromaticityColorProfileItem::setVisible(bool visible)
{
    for (auto item : m_lineItems)
        item->setVisible(visible);
    if (m_titleItem)
        m_titleItem->setVisible(visible);
}

void ChromaticityColorProfileItem::setPlotArea(QRectF plotArea, QPointF plotRange)
{
    m_plotArea = plotArea;
    m_plotRange = plotRange;
    setScenePos();
}

void ChromaticityColorProfileItem::addItems(QGraphicsScene *scene)
{
    m_scene = scene;
    for (auto item : m_lineItems)
        m_scene->addItem(item);
    if (m_titleItem)
        m_scene->addItem(m_titleItem);
}

void ChromaticityColorProfileItem::removeItems()
{
    if (m_scene == nullptr)
        return;
    for (auto item : m_lineItems)
        m_scene->removeItem(item);
    if (m_titleItem)
        m_scene->removeItem(m_titleItem);
}

void ChromaticityColorProfileItem::setScenePos()
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
