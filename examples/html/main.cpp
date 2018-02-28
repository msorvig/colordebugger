#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtCharts>

#include "emscripten.h"

QT_CHARTS_USE_NAMESPACE
    
#include "chromaticitydiagram.h"
#include "colorconvert.h"

// Resolve ambigious activated function
static auto comboBoxActivatedIntFn = static_cast<void(QComboBox::*)(int)>(&QComboBox::activated);

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

int main(int argc, char ** argv)
{
    QApplication app(argc, argv);

    ChromaticityDiagramWindow chromaticityDiagram;
//    chromaticityDiagram.winId();
//    chromaticityDiagram.windowHandle()->showMaximized();

//    chromaticityDiagram.showFullScreen();
    
#ifdef Q_OS_HTML5
    // Signal that the application is ready. This should ideally be
    // done after we have painted the first frame.
    EM_ASM(console.log("appReady"));
    
    /*
    QTimer::singleShot(0, [](){
        EM_ASM(
            console.log("appReady")
            if (qt_appReady !== undefined)
                qt_appReady();
        );
    });
    */
#endif        
    
    return app.exec();

}
