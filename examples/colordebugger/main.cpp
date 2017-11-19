#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtCharts>
QT_CHARTS_USE_NAMESPACE // wat
    
#include "chromaticitydiagram.h"
#include "colorconvert.h"

// Resolve ambigious activated function
static auto comboBoxActivatedIntFn = static_cast<void(QComboBox::*)(int)>(&QComboBox::activated);

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

class TestContentWidget : public QWidget
{
public:
    TestContentWidget()
        :m_sourceColorSpace(sRGB)
        ,m_targetColorSpace(sRGB)
        ,m_displayColorSpace(sRGB)
    {
        setMouseTracking(true);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // Set default gradient
        QLinearGradient gradient;
        gradient.setColorAt(0.1, Qt::green);
        gradient.setColorAt(0.3, Qt::blue);
        gradient.setColorAt(0.6, Qt::red);
        gradient.setColorAt(0.9, Qt::green);
        setTestGradient(gradient, m_sourceColorSpace);
    }

    QSize sizeHint() const {
        return QSize(150, 150);
    }

    void setTargetColorSpace(const RGBColorSpace &rgbColorSpace)
    {
        m_targetColorSpace = rgbColorSpace;
        update();
    }

    void setTestImage(const QImage &image, const RGBColorSpace colorSpace) {
        m_sourceImage = image;
        m_sourceGradient = QLinearGradient();
        m_sourceColorSpace = colorSpace;
        update();
    }

    void setTestGradient(const QLinearGradient &gradient, const RGBColorSpace colorSpace) {
        m_sourceGradient = gradient;
        m_sourceImage = QImage();
        m_sourceColorSpace = colorSpace;
        update();
    }

    QColor sample(QPoint position) {
        if (position.x() < 0 || position.y() < 0 || position.x() >= width() || position.y() >= height())
            return QColor();

        return m_targetImage.pixelColor(position);
    }

    void paintEvent(QPaintEvent *) {
        QRect rect = QRect(QPoint(0, 0), size());

//        qDebug() << "paintEvent" << rect;

        // Render to indirect image to get readPixel access.
        if (m_targetImage.size() != size()) {
            m_targetImage = QImage(size(), QImage::Format_ARGB32_Premultiplied);
        }

        // Content may have changed; re-render
        {
            QPainter p(&m_targetImage);
            if (!m_sourceImage.isNull()) {
                QImage scaledImage = m_sourceImage.scaled(rect.size());
                p.fillRect(rect, QBrush(scaledImage));
            } else {
                m_sourceGradient.setStart(rect.topLeft());
                m_sourceGradient.setFinalStop(rect.bottomRight());
                p.fillRect(rect, QBrush(m_sourceGradient));
            }
        }

        RGBColorSpace::colorConvert(&m_targetImage, m_sourceColorSpace, m_targetColorSpace);

        QImage displayImage = m_targetImage.copy();
        RGBColorSpace::colorConvert(&displayImage, m_targetColorSpace, m_displayColorSpace);

        QPainter p(this);
        p.fillRect(rect, displayImage);

    }
private:
    QImage m_targetImage;
    QImage m_sourceImage;
    QLinearGradient m_sourceGradient;
    RGBColorSpace m_sourceColorSpace;
    RGBColorSpace m_targetColorSpace;
    RGBColorSpace m_displayColorSpace;
};

class TestSelectorWidget : public QWidget
{
public:
    TestSelectorWidget()
    {
        setWindowTitle("Source Selector");

        // Create UI
        QVBoxLayout *layout = new QVBoxLayout();
        layout->setSpacing(5);
        layout->setMargin(5);
        layout->setAlignment(Qt::AlignBottom);
        this->setLayout(layout);

        m_contentWidget = new TestContentWidget();
        m_contentWidget->resize(300, 300);
        m_contentWidget->setParent(this);
        layout->addWidget(m_contentWidget);

        // Create test selection UI

        QStringList gradients = { "Gradient" };
        QStringList images = { "Rose", "Iceland", "YellowFlower" };
        QList<QStringList> imageVariants = { { "sRGB", "AdobeRGB", "ProPhoto" },
                                             { "sRGB", "DCI-P3" },
                                             { "sRGB", "DCI-P3" } };
        QStringList contents = gradients + images;

        QComboBox *contentSelector = new QComboBox();
        layout->addWidget(contentSelector);
        contentSelector->addItems(contents);

        QComboBox *colorSpaceSelector = new QComboBox();
        layout->addWidget(colorSpaceSelector);
        colorSpaceSelector->addItems(colorSpaceNames());

        auto selectImageFileName = [=](int imageIndex, int colorSpaceIndex) -> QString {

            // Build image file name
            QString base = images.at(imageIndex);
            QString variant = imageVariants.at(imageIndex).at(colorSpaceIndex);
            QString extention = ".jpg";

            if (variant == "DCI-P3")
                variant = "P3";

            return  base + "-" + variant + ".jpg";
        };

        auto setTestContent = [=](int contentIndex, int colorSpaceIndex) {
            RGBColorSpace sourceColorSpace((RgbColorSpace(colorSpaceIndex)));
            if (contentIndex < gradients.count()) {
                QLinearGradient gradient;
                gradient.setColorAt(0.1, Qt::green);
                gradient.setColorAt(0.3, Qt::blue);
                gradient.setColorAt(0.6, Qt::red);
                gradient.setColorAt(0.9, Qt::green);
                m_contentWidget->setTestGradient(gradient, sourceColorSpace);
            } else {
                int imageIndex = contentIndex - gradients.count();
                int colorSpaceIndex = colorSpaceSelector->currentIndex();
                QString imageFileName = selectImageFileName(imageIndex, colorSpaceSelector->currentIndex());
                QString imageFilePath = ":/images/" + imageFileName;
                m_contentWidget->setTestImage(QImage(imageFilePath), sourceColorSpace);
            }
        };

        auto setColorspaceSelectorContent = [=](int contentIndex) {
            colorSpaceSelector->setCurrentIndex(0);
            colorSpaceSelector->clear();
            if (contentIndex < gradients.count()) {
                colorSpaceSelector->addItems(colorSpaceNames());
            } else {
                int imageIndex = contentIndex - gradients.count();
                colorSpaceSelector->addItems(imageVariants.at(imageIndex));
            }
        };

        connect(contentSelector, comboBoxActivatedIntFn, [=](int i) {
            setColorspaceSelectorContent(i);
            setTestContent(i, colorSpaceSelector->currentIndex());
        });

        connect(colorSpaceSelector, comboBoxActivatedIntFn, [=](int i) {
            setTestContent(contentSelector->currentIndex(), i);
        });
    }

    TestContentWidget *contentWidget() const
    {
        return m_contentWidget;
    }

    void setTargetColorSpace(const RGBColorSpace colorSpace)
    {
        m_contentWidget->setTargetColorSpace(colorSpace);
    }

private:
    TestContentWidget *m_contentWidget = nullptr;
};

class ChromaticityDiagramWindow : public QWidget
{
public:
    ChromaticityDiagramWindow()
        :m_colorSpace(sRGB)
    {
        setWindowTitle("Chromaticity Diagram");

        QVBoxLayout *layout = new QVBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        setLayout(layout);

        // Add diagram
        m_chromaticityDiagram = new ChromaticityDiagram();
        layout->addWidget(m_chromaticityDiagram);

        QString monospacedFont = "Courier New";
        {
            // Add line which displays input RGB, XYZ and xyY coordinates
            QHBoxLayout *line = new QHBoxLayout();
            line->setAlignment(Qt::AlignLeft);
            line->addSpacing(10);
            layout->addLayout(line);

            m_rgbInputColor = new QLabel("RGB: (0.5 0.5, 0.5)");
            m_rgbInputColor->setFont(QFont(monospacedFont));
            line->addWidget(m_rgbInputColor);

            m_XYZInputColor = new QLabel("XYZ: (- - -)");
            m_XYZInputColor->setFont(QFont(monospacedFont));
            line->addWidget(m_XYZInputColor);

            m_xyColor = new QLabel("xy: (0.5, 0.5)");
            m_xyColor->setFont(QFont(monospacedFont));
            line->addWidget(m_xyColor);
        }
        {
            // Add line which displays output/converted RGB
            QHBoxLayout *line = new QHBoxLayout();
            line->setAlignment(Qt::AlignLeft);
            line->addSpacing(10);
            layout->addLayout(line);

            QComboBox *spaceSelector = new QComboBox();
            line->addWidget(spaceSelector);
            spaceSelector->addItems(colorSpaceNames());
            connect(spaceSelector, comboBoxActivatedIntFn, [this](int i) {
                m_colorSpace = RGBColorSpace(RgbColorSpace(i));
            });

            m_rgbConverted = new QLabel("RGB: (0.5 0.5, 0.5)");
            m_rgbConverted->setFont(QFont(monospacedFont));
            line->addWidget(m_rgbConverted);
        }

        layout->addSpacing(5);
    }

    void setColor(QColor color, RGBColorSpace colorSpace)
    {
        if (!color.isValid()) {
            m_rgbInputColor->setText("RGB: (           )");
            m_XYZInputColor->setText("XYZ: (              )");
            m_xyColor->setText("xy: (     )");
            m_rgbConverted->setText("RGB: (           )");
            return;
        }

        m_rgbInputColor->setText(QString("RGB: (%1 %2 %3)").arg(color.red(), 3)
                                                           .arg(color.green(), 3)
                                                           .arg(color.blue(), 3));

        auto Yxy = colorSpace.convertRGBtoYxy(color);
        m_xyColor->setText(QString("xy: (%1 %2)").arg(Yxy(1, 0), 2, 'f', 2)
                                                 .arg(Yxy(2, 0), 2, 'f', 2));

        auto XYZ = colorSpace.convertRGBtoXYZ(color);
        m_XYZInputColor->setText(QString("XYZ: (%1 %2 %3)").arg(XYZ(0, 0), 2, 'f', 2)
                                                           .arg(XYZ(1, 0), 2, 'f', 2)
                                                           .arg(XYZ(2, 0), 2, 'f', 2));

        // Convert back to (possibly different) RGB color space, as selected by combobox
        QColor convertedRGB = m_colorSpace.convertYxyToRGB(Yxy);
        m_rgbConverted->setText(QString("RGB: (%1 %2 %3)").arg(convertedRGB.red(), 3)
                                                          .arg(convertedRGB.green(), 3)
                                                          .arg(convertedRGB.blue(), 3));
    }

    ChromaticityDiagram *diagram()
    {
        return m_chromaticityDiagram;
    }

private:
    ChromaticityDiagram *m_chromaticityDiagram;
    QLabel *m_rgbInputColor;
    QLabel *m_XYZInputColor;
    QLabel *m_xyColor;
    QLabel *m_rgbConverted;
    RGBColorSpace m_colorSpace;
};

class WindowColorController : public QWidget
{
public:
    WindowColorController(TestContentWidget *testWindow, ChromaticityDiagramWindow *chromaticityDiagramWindow)
    :m_testWindow(testWindow)
    ,m_colorSpace(sRGB)
    ,m_chromaticityDiagramWindow(chromaticityDiagramWindow)
    ,m_chromaticityDiagram(m_chromaticityDiagramWindow->diagram())
    ,m_colorItemCount(1)
    ,m_sampleRadius(10)
    {
        // Install event filter to sample from test window.
        m_testWindow->installEventFilter(this);
        
        setWindowTitle("Controller");
        
        // Create UI
        QVBoxLayout *layout = new QVBoxLayout();
        layout->setSpacing(5);
        //layout->setMargin(5);
        layout->setAlignment(Qt::AlignTop);
        this->setLayout(layout);

        // Create test window configuration UI
        layout->addWidget(new QLabel("<b>Working Color Space</b>"));
        QComboBox *spaceSelector = new QComboBox();
        layout->addWidget(spaceSelector);
        spaceSelector->addItems(colorSpaceNames());
        connect(spaceSelector, comboBoxActivatedIntFn, [this](int i) {
            m_colorSpace = RGBColorSpace(RgbColorSpace(i));
            m_testWindow->setTargetColorSpace(m_colorSpace);
        });

        {
            QHBoxLayout *displayLayout = new QHBoxLayout();
            displayLayout->setAlignment(Qt::AlignLeft);
            // disabled
            // layout->addLayout(displayLayout);
            displayLayout->addWidget(new QLabel("Source -> Workspace:"));
            QButtonGroup *group = new QButtonGroup(this);
            QRadioButton *colorConvert = new QRadioButton("Convert");
            group->addButton(colorConvert);
            colorConvert->setChecked(true);
            displayLayout->addWidget(colorConvert);
            QRadioButton *colorAssign = new QRadioButton("Assign");
            group->addButton(colorAssign);
            displayLayout->addWidget(colorAssign);
        }
        {
            QHBoxLayout *displayLayout = new QHBoxLayout();
            displayLayout->setAlignment(Qt::AlignLeft);
            // disabled
            // layout->addLayout(displayLayout);
            displayLayout->addWidget(new QLabel("Workspace -> Display:"));
            QRadioButton *colorConvert = new QRadioButton("Convert");
            colorConvert->setChecked(true);
            displayLayout->addWidget(colorConvert);
            QRadioButton *colorAssign = new QRadioButton("Assign");
            displayLayout->addWidget(colorAssign);
            layout->addSpacing(10);
        }

        // Create sampler configuration UI
        layout->addWidget(new QLabel("<b>Sampler</b>"));

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

        layout->addSpacing(10);

        // Define function that adds a color space selecor row
        m_addColorSelector = [this](QVBoxLayout *layout) {

            // Each row controls a color profile item on the diagram
            QSharedPointer<ChromaticityColorProfileItem> item(new ChromaticityColorProfileItem());
            m_chromaticityDiagram->addColorProfileItem(item.data());
            QSharedPointer<bool> wasActivated(new bool(false));

            // Create row lauyout
            QHBoxLayout *rowLayout = new QHBoxLayout();
            rowLayout->setAlignment(Qt::AlignLeft);
            layout->addLayout(rowLayout);

            // Create and populate combobox with all supported color spaces
            QComboBox *spaceSelector = new QComboBox();
            rowLayout->addWidget(spaceSelector);
            spaceSelector->addItem("<select color space>");
            spaceSelector->addItems(colorSpaceNames());

            // Add "remove" button
            QPushButton *remove = new QPushButton("X");
            remove->setVisible(false); // ### buggy, disable
            rowLayout->addWidget(remove);
            remove->setEnabled(false);
            connect(remove, &QPushButton::clicked, [layout, rowLayout](bool checked){
                Q_UNUSED(checked)
                // Clean up: the layout owns the UI widgets
                layout->removeItem(rowLayout);
                rowLayout->deleteLater();
            });

            // Add checkbox that copntrols visiiblity.
            QCheckBox *visible = new QCheckBox("Visble");
            visible->setEnabled(false);
            visible->setChecked(true);
            connect(visible, &QCheckBox::toggled, [item](bool checked) {
                item->setVisible(checked);
            });
            rowLayout->addWidget(visible);

            // Combox selection changed
            connect(spaceSelector, comboBoxActivatedIntFn, [this, item, layout, wasActivated, remove, visible](int i) {
                if (i < 1) {
                    item->setVisible(false);
                    remove->setEnabled(false);
                    visible->setEnabled(false);
                    return;
                }

                item->setColorSpace(RGBColorSpace(RgbColorSpace(i - 1)));
                item->setVisible(visible->isChecked());
                remove->setEnabled(true);
                visible->setEnabled(true);

               if (*wasActivated)
                    return;
                *wasActivated = true;
              // Enable this line and add next line
               this->m_addColorSelector(layout);
            });
        };
        
        layout->addWidget(new QLabel("<b>Diagram Color Spaces</b>"));
        m_addColorSelector(layout);
    }

    bool eventFilter(QObject *, QEvent *ev)
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

        // First point: sample at cursor position
        QColor color = m_testWindow->sample(pos);
        m_items.at(0)->setColor(color, m_colorSpace);

        // Set main color for RGB/XYZ output
        m_chromaticityDiagramWindow->setColor(color, m_colorSpace);

        // Rest of the points: sample around cursor position
        for (int i = 1; i < m_colorItemCount; ++i) {

            // Square around pos
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

    bool filterLeaveEvent(QEvent *) {
        for (auto item : m_items)
            item->setVisible(false);

        m_chromaticityDiagramWindow->setColor(QColor(), m_colorSpace);

        return false;
    }

private:
    TestContentWidget *m_testWindow;
    RGBColorSpace m_colorSpace;
    ChromaticityDiagramWindow *m_chromaticityDiagramWindow;
    ChromaticityDiagram *m_chromaticityDiagram;

    int m_colorItemCount;
    int m_sampleRadius;
    QList<ChromaticityColorItem *> m_items;
    std::function<void(QVBoxLayout *)> m_addColorSelector;
};


class QImageColorDebugger
{
public:
    QImageColorDebugger();

    void setImage(const QImage &image, RGBColorSpace colorSpace);
    QImage image();

    void setDiagramColorSpaces(QList<RGBColorSpace> colorSpaces);
    void setSamplePointCount(int count);
    void setSamplePointRadious(int radius);
    void setSamplePosition(QPoint imagePosition);

    void setDebuggerVisisble(bool visible);

    void setImageViewerVisible(bool visible);
    bool imageViewerVisible() const;

    void setChromaticityDiagramVisible(bool visible);
    bool chromaticityDiagramVisible() const;

    void setSampleControllerVisible(bool visible);
    bool sampleControllerVisible() const;

private:

};

int main(int argc, char ** argv)
{
    QApplication app(argc, argv);

    // Tests
    extern void colorConvertTest();
    colorConvertTest();
    //return 0;

 //   printPrimaries();
//    thereAndBackAgain();
 //   printMonochromatic();
//    fromPrimaries();

    TestSelectorWidget testSelectorWidget;
    testSelectorWidget.show();

    ChromaticityDiagramWindow chromaticityDiagram;

    WindowColorController controller(testSelectorWidget.contentWidget(), &chromaticityDiagram);

    // Show all windows

    testSelectorWidget.setGeometry(100, 100, 300, 300);
    testSelectorWidget.raise();
    testSelectorWidget.show();

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
