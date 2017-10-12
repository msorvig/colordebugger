
#include <QtCore>
#include <QtWidgets>
#include <QtCharts>

#include "colorconvert.h"

QT_CHARTS_USE_NAMESPACE
    
// ChromaticityDiagram displays a CIE XY Chromacticity diagram with an
// outline of the monochromatic ("rainbow") colors and a color gradient
// fill (for illiustration).

// Colors and Color spaces can be plotted on the diagram. Colors are
// plotted as colored circles at the color xy position, while color 
// profiles are plotted as triangles with vertices at the xy postiions
// for the color profile RGB primaries.

class ChromaticityColorItem;
class ChromaticityColorProfileItem;
class ChromaticityDiagram : public QGraphicsView
{
public:
    ChromaticityDiagram();

    void addColorItem(ChromaticityColorItem *colorItem);
    void clearColorItems();

    void addColorProfileItem(ChromaticityColorProfileItem *colorProfileItem);

protected:
    void setPlotRange(QPointF plotRange);
    bool event(QEvent *event);
    bool gestureEvent(QGestureEvent *event);
    bool pinchGestureEvent(QPinchGesture *gesture);
    void resizeEvent(QResizeEvent *ev);

private:
    QGraphicsScene *m_scene;
    QChart *m_chart;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    
    QPointF m_plotRangeMinimum = QPointF(0.8, 0.9);
    QPointF m_plotRange = m_plotRangeMinimum;

    QList<ChromaticityColorItem *> m_colorItems;
    QList<ChromaticityColorProfileItem *> m_colorProfileItems;
};

// A Color item which is rendered as a circle on the diagram
class ChromaticityColorItem : public QGraphicsEllipseItem
{
public:
    ChromaticityColorItem();
    void setColor(QColor color, RGBColorSpace colorSpace);
    void setColor(qreal x, qreal y);


private:
    friend class ChromaticityDiagram;

    void setPlotArea(QRectF plotArea, QPointF plotRange);
    void setRenderColor(QColor color);
    void setScenePos();

    QPointF m_xy;
    QRectF m_plotArea;
    QPointF m_plotRange;
};

// A Color Profile item which is rendered as a triangle on the diagram
class ChromaticityColorProfileItem
{
public:
    ChromaticityColorProfileItem();
    ~ChromaticityColorProfileItem();

    void setColorSpace(RGBColorSpace colorSpace);
    void setVisible(bool visible);

private:
    friend class ChromaticityDiagram;
    void setPlotArea(QRectF plotArea, QPointF plotRange);
    void addItems(QGraphicsScene *scene);
    void removeItems();
    void setScenePos();

    QPointF m_xy[3];
    QRectF m_plotArea;
    QPointF m_plotRange;
    QList<QGraphicsLineItem *>m_lineItems;
    QGraphicsSimpleTextItem *m_titleItem;
    QGraphicsScene *m_scene = nullptr;
};
