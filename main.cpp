#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtCharts>
QT_CHARTS_USE_NAMESPACE // wat
    
#include "chromaticitydiagram.h"
#include "colorconvert.h"

/*    
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
    
*/

void fromPrimaries()
{
    // RGBColorSpace full( {0.7, 0.25}, {0.05, 0.85}, {0.15, 0.0}, 1.0, "full");
    
    // Test vector for matrix derivaton
    RGBColorSpace test((qreal []){0.640, 0.330}, (qreal []){0.300, 0.600}, (qreal []){0.150, 0.060}, 1.0, "test");
    // Expected Ci: CR = 0.6443606239 CG = 1.1919477979 C B = 1.2032052560
    // Expected NPM:
    //        ⎡0.4123907993 0.3575843394 0.1804807884⎤
    // NPM =  ⎢0.2126390059 0.7151686788 0.0721923154⎥
    //        ⎣0.0193308187 0.1191947798 0.9505321522⎦
} 

/*

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

*/

class TestWindow : public QRasterWindow
{
public:
    TestWindow() {

    }

    QColor sample(QPoint position) {
        if (position.x() < 0 || position.y() < 0 || position.x() >= width() || position.y() >= height())
            return QColor();

        return m_image.pixelColor(position);
    }

    void paintEvent(QPaintEvent *ev) {
        QRect rect = QRect(QPoint(0, 0), size());

        // Render to indirect image to get readPixel access.
        if (m_image.size() != size()) {
            m_image = QImage(size(), QImage::Format_ARGB32_Premultiplied);

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
    WindowColorController(TestWindow *testWindow, ChromaticityDiagram *chromaticityDiagram)
    :m_testWindow(testWindow)
    ,m_colorSpace(sRGB, 1.0)
    ,m_chromaticityDiagram(chromaticityDiagram)
    ,m_colorItemCount(1)
    ,m_sampleRadius(10)
    {
        // Install event filter to sample from test window.
        m_testWindow->installEventFilter(this);
        
        setWindowTitle("Controller");
        
        // Create UI
        QVBoxLayout *layout = new QVBoxLayout();
        //layout->setSpacing(0);
        layout->setAlignment(Qt::AlignTop);
        this->setLayout(layout);

        // Create Sampler config UI
        layout->addWidget(new QLabel("Sampler"));

        QHBoxLayout *samplerConfigLayout = new QHBoxLayout;
        layout->addLayout(samplerConfigLayout);

        samplerConfigLayout->addWidget(new QLabel("Points"));
        QSpinBox *sampleCount = new QSpinBox();
        samplerConfigLayout->addWidget(sampleCount);
        sampleCount->setValue(m_colorItemCount);
        sampleCount->setMinimum(1);

        samplerConfigLayout->addWidget(new QLabel("Radius"));
        QSpinBox *sampleRadius = new QSpinBox();
        samplerConfigLayout->addWidget(sampleRadius);
        sampleRadius->setDisabled(true); // needs more than one point
        sampleRadius->setValue(10);

        auto valueChangedIntFn = static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged);
        connect(sampleCount, valueChangedIntFn, [this, sampleRadius](int value){
            m_colorItemCount = value;
            sampleRadius->setDisabled(value < 2);
        });

        connect(sampleRadius, valueChangedIntFn, [this](int value){
            m_sampleRadius = value;
        });

        // Define function that adds a color space selecor row
        m_addColorSelector = [this](QVBoxLayout *layout) {
            // The row controls a color profile item on the diagram
            ChromaticityColorProfileItem *item = new ChromaticityColorProfileItem();
            m_chromaticityDiagram->addColorProfileItem(item);
            bool *wasActivated = new bool(false);

            // Create row lauyout
            QHBoxLayout *rowLayout = new QHBoxLayout();
            rowLayout->setAlignment(Qt::AlignLeft);
            layout->addLayout(rowLayout);

            // Create and populate combobox with all supported color spaces
            QComboBox *spaceSelector = new QComboBox();
            rowLayout->addWidget(spaceSelector);
            spaceSelector->addItem("<select color space>");
            for (int i = 0; i < ColorSpaceCount; ++i) {
                spaceSelector->addItem(colorSpaceName(RgbColorSpace(i)));
            }
#if 1
            // Add "remove" button
            QPushButton *remove = new QPushButton("X");
            remove->setVisible(false); // ### buggy, disable
            rowLayout->addWidget(remove);
            remove->setEnabled(false);
            connect(remove, &QPushButton::clicked, [layout, item, wasActivated, rowLayout](bool checked){
                // Clean up: the layout owns the UI widgets
                layout->removeItem(rowLayout);
                rowLayout->deleteLater();
                delete item;
            });
#endif
            // Add checkbox that copntrols visiiblity.
            QCheckBox *visible = new QCheckBox("Visble");
            visible->setEnabled(false);
            visible->setChecked(true);
            connect(visible, &QCheckBox::toggled, [item](bool checked) {
                item->setVisible(checked);
            });
            rowLayout->addWidget(visible);

            // Combox selection changed
            auto activatedIntFn = static_cast<void(QComboBox::*)(int)>(&QComboBox::activated);
            connect(spaceSelector, activatedIntFn, [this, item, layout, wasActivated, remove, visible](int i) {
                if (i >= 1) {
                    item->setColorSpace(RGBColorSpace(RgbColorSpace(i - 1), 1.0));
                    if (*wasActivated == false) {
                        *wasActivated = true;

                        remove->setEnabled(true);
                        visible->setEnabled(true);

                        // Add "next" selector row
                        this->m_addColorSelector(layout);
                    }
                }
            });
        };
        
        layout->addWidget(new QLabel("Color Spaces"));
        m_addColorSelector(layout);
    }

    bool eventFilter(QObject *object, QEvent *ev)
    {
        if (ev->type() == QEvent::MouseMove)
            return filterMouseMoveEvent(static_cast<QMouseEvent *>(ev));
        else if (ev->type() == QEvent::Leave)
            return filterLeaveEvent(ev);
        return false;
    }

    bool filterMouseMoveEvent(QMouseEvent *mouseEvent) {
        QPoint pos = mouseEvent->localPos().toPoint();
        if (pos.x() < 0 || pos.y() < 0)
            return false;

        // Create color items for the diagram
        if (m_items.count() != m_colorItemCount) {

            qDeleteAll(m_items);
            m_items.clear();

            for (int i = 0; i < m_colorItemCount; ++i) {
                auto item = new ChromaticityColorItem();
                m_items.append(item);
                m_chromaticityDiagram->addColorItem(item);
            }
        }


        // first point: sample at pos.
        QColor color = m_testWindow->sample(pos);
        m_items.at(0)->setColor(color, m_colorSpace);

        // Sample test window at/around cursor position.
        for (int i = 1; i < m_colorItemCount; ++i) {
            // square around pos
            int edgeCount = sqrt(m_colorItemCount - 1);
            int offsetIndex = i - 1;
            QPoint offset(-m_sampleRadius + (offsetIndex % edgeCount) * m_sampleRadius *2,
                          -m_sampleRadius + (offsetIndex / edgeCount) * m_sampleRadius *2);

            QPoint itemPos = pos + offset;
            QColor color = m_testWindow->sample(itemPos);
            m_items.at(i)->setColor(color, m_colorSpace);
        }

        return false;
    }

    bool filterLeaveEvent(QEvent *e) {
        for (auto item : m_items)
            item->setVisible(false);

        return false;
    }

private:
    TestWindow *m_testWindow;
    RGBColorSpace m_colorSpace;
    ChromaticityDiagram *m_chromaticityDiagram;
    int m_colorItemCount;
    int m_sampleRadius;
    QList<ChromaticityColorItem *> m_items;
    std::function<void(QVBoxLayout *)> m_addColorSelector;
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
    fromPrimaries();

    TestWindow testWindow;
    testWindow.setTitle("Test Window");
    ChromaticityDiagram chromaticityDiagram;
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
