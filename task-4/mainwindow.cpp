#include "mainwindow.h"
#include "circleTool.h"
#include "clippingTool.h"
#include "polygonTool.h"
#include "bucketTool.h"
#include "qdebug.h"
#include "qfiledialog.h"
#include "qlabel.h"
#include "qobject.h"
#include "qpushbutton.h"
#include "rectangleTool.h"
#include "ui_mainwindow.h"
#include "canvas.h"
#include <QPushButton>
#include <QDebug>
#include <QFileDialog>
#include <QLabel>
#include <QSpinBox>
#include <QColorDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentTool(nullptr)
{
    ui->setupUi(this);

    canvas = findChild<Canvas*>("canvas");
    QLabel* figureInfoLabel = findChild<QLabel*>("figureInfoLabel");
    QSpinBox* thicknessSpinBox = findChild<QSpinBox*>("thicknessSpinBox");
    QLabel* colorLabel = findChild<QLabel*>("colorLabel");
    QLabel* thicknessLabel = findChild<QLabel*>("thicknessLabel");
    QPushButton* colorButton = findChild<QPushButton*>("colorButton"); // Border color button
    QLabel* fillColorLabel = findChild<QLabel*>("fillColorLabel");
    QPushButton* fillColorButton = findChild<QPushButton*>("fillColorButton"); // Fill color button
    QPushButton* clearColorButton = findChild<QPushButton*>("clearColorButton");
    QPushButton* selectImgButton = findChild<QPushButton*>("selectImgButton");
    QPushButton* removeImgButton = findChild<QPushButton*>("removeImgButton");
    QPushButton* bucketButton = findChild<QPushButton*>("bucketButton");


    if (!canvas || !figureInfoLabel || !thicknessSpinBox || !colorButton ||
        !colorLabel || !thicknessLabel || !fillColorLabel || !fillColorButton || !clearColorButton || !selectImgButton || !removeImgButton || !bucketButton) {
        qDebug() << "Error: Required widgets not found in UI (ensure bucketButton is present)!";
    }

    // Initialize tools
    pointerTool = new Pointer(canvas);
    lineTool = new LineTool();
    polygonTool = new PolygonTool();
    circleTool = new CircleTool();
    rectangleTool = new RectangleTool();
    clippingTool = new ClippingTool(canvas, canvas->clippingManager);
    bucketTool = new BucketTool(canvas); // Initialize BucketTool with canvas

    // Initial UI state
    figureInfoLabel->setVisible(true);
    thicknessSpinBox->setVisible(false);
    colorButton->setVisible(false);
    colorLabel->setVisible(false);
    thicknessLabel->setVisible(false);
    fillColorButton->setVisible(false);
    fillColorLabel->setVisible(false);
    clearColorButton->setVisible(false);
    selectImgButton->setVisible(false);
    removeImgButton->setVisible(false);


    connect(findChild<QPushButton*>("mousePointerButton"), &QPushButton::clicked, this, &MainWindow::onPointerButtonClicked);
    connect(findChild<QPushButton*>("lineButton"), &QPushButton::clicked, this, &MainWindow::onLineButtonClicked);
    connect(findChild<QPushButton*>("polygonButton"), &QPushButton::clicked, this, &MainWindow::onPolygonButtonClicked);
    connect(findChild<QPushButton*>("circleButton"), &QPushButton::clicked, this, &MainWindow::onCircleButtonClicked);
    connect(findChild<QPushButton*>("rectangleButton"), &QPushButton::clicked, this, &MainWindow::onRectangleButtonClicked);
    connect(findChild<QPushButton*>("cliperkaButton"), &QPushButton::clicked, this, &MainWindow::onCliperkaButtonClicked);
    connect(bucketButton, &QPushButton::clicked, this, &MainWindow::onBucketButtonClicked);



    connect(findChild<QAction*>("actionClear"), &QAction::triggered, this, [this]() {
        if (canvas) {
            canvas->clearFigures();
        }
    });

    connect(findChild<QAction*>("actionSave"), &QAction::triggered, this, [this]() {
        if (canvas) {
            QString filePath = QFileDialog::getSaveFileName(this, "Save Figures", "", "Text Files (*.txt)");
            if (!filePath.isEmpty()) {
                canvas->saveFigures(filePath);
            }
        }
    });

    connect(findChild<QAction*>("actionOpen"), &QAction::triggered, this, [this]() {
        if (canvas) {
            QString filePath = QFileDialog::getOpenFileName(this, "Open Figures", "", "Text Files (*.txt)");
            if (!filePath.isEmpty()) {
                canvas->loadFigures(filePath);
            }
        }
    });

    connect(findChild<QAction*>("action_Exit"), &QAction::triggered, this, &MainWindow::close);

    QAction* antiAliasingAction = findChild<QAction*>("actionAnti_aliasing");
    if (antiAliasingAction) {
        connect(antiAliasingAction, &QAction::toggled, canvas, &Canvas::setAntiAliasingEnabled);
    }

    connect(canvas, &Canvas::figureSelected, this,
        [this, figureInfoLabel, thicknessSpinBox, colorButton,
         colorLabel, thicknessLabel, fillColorButton, fillColorLabel, clearColorButton, selectImgButton, removeImgButton]
        (const QColor& borderColor, const QColor& fillColor, int thickness) {
        // This slot is for when a figure is selected by the Pointer tool
        if (currentTool != pointerTool) return;

        statusBar()->showMessage(QString("Selected Figure - Border: %1, Fill: %2, Thickness: %3")
                             .arg(borderColor.name()).arg(fillColor.name()).arg(thickness));
        figureInfoLabel->setVisible(false);
        colorButton->setStyleSheet(QString("background-color: %1;").arg(borderColor.name()));
        colorButton->setVisible(true);
        colorLabel->setVisible(true);
        if (fillColor.isValid()) {
            fillColorButton->setStyleSheet(QString("background-color: %1;").arg(fillColor.name()));
        } else {
            fillColorButton->setStyleSheet("background-color: none;");
        }
        fillColorButton->setVisible(true);
        fillColorLabel->setVisible(true);
        thicknessSpinBox->setVisible(true);
        thicknessLabel->setVisible(true);
        thicknessSpinBox->setValue(thickness);
        clearColorButton->setVisible(true);
        selectImgButton->setVisible(true);
        removeImgButton->setVisible(true);
    });
    connect(canvas, &Canvas::figureDeselected, this,
        [this, figureInfoLabel, thicknessSpinBox, colorButton,
         colorLabel, thicknessLabel, fillColorButton, fillColorLabel, clearColorButton, selectImgButton, removeImgButton]() {
        // This slot is for when a figure is deselected by the Pointer tool
        if (currentTool != pointerTool) return;

        statusBar()->clearMessage();
        // canvas->selectFigure(nullptr); // Already handled by Pointer tool or setCurrentTool
        figureInfoLabel->setVisible(true);
        figureInfoLabel->setText("No figure selected.");
        colorButton->setVisible(false);
        colorLabel->setVisible(false);
        fillColorButton->setVisible(false);
        fillColorLabel->setVisible(false);
        thicknessSpinBox->setVisible(false);
        thicknessLabel->setVisible(false);
        clearColorButton->setVisible(false);
        selectImgButton->setVisible(false);
        removeImgButton->setVisible(false);
    });
    connect(thicknessSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (currentTool == pointerTool && canvas->selectedFigure != nullptr) {
            qDebug() << "Changing thickness of selected figure to" << value;
            canvas->selectedFigure->setThickness(value);
            canvas->update(); // Repaint the canvas
        } else {
            qDebug() << "No figure selected or pointer tool not active while adjusting thickness.";
        }
    });

    connect(colorButton, &QPushButton::clicked, this, [this, colorButton]() { // Border color button
        if (currentTool == pointerTool && canvas->selectedFigure != nullptr) {
            QColor selectedColor = QColorDialog::getColor(canvas->selectedFigure->getBorderColor(), this, "Select Border Color");
            if (selectedColor.isValid()) {
                qDebug() << "Changing border color of selected figure to" << selectedColor.name();
                canvas->selectedFigure->setBorderColor(selectedColor);
                colorButton->setStyleSheet(QString("background-color: %1;").arg(selectedColor.name()));
                canvas->update(); // Repaint the canvas
            }
        } else {
            qDebug() << "No figure selected or pointer tool not active while changing border color.";
        }
    });

    connect(fillColorButton, &QPushButton::clicked, this, [this, fillColorButton]() { // Fill color button
        if (currentTool == bucketTool) {
            QColor selectedColor = QColorDialog::getColor(bucketTool->getFillColor(), this, "Select Bucket Fill Color");
            if (selectedColor.isValid()) {
                bucketTool->setFillColor(selectedColor);
                fillColorButton->setStyleSheet(QString("background-color: %1;").arg(selectedColor.name()));
            }
        } else if (currentTool == pointerTool && canvas->selectedFigure != nullptr) {
            QColor selectedColor = QColorDialog::getColor(canvas->selectedFigure->getFillColor(),
                                                       this, "Select Figure Fill Color");
            if (selectedColor.isValid()) {
                qDebug() << "Changing fill color of selected figure to" << selectedColor.name();
                canvas->selectedFigure->setFillColor(selectedColor);
                fillColorButton->setStyleSheet(QString("background-color: %1;").arg(selectedColor.name()));
                canvas->update(); // Repaint the canvas
            }
        } else {
            qDebug() << "No figure selected (for pointer) or bucket tool not active while changing fill color.";
        }
    });

    connect(clearColorButton, &QPushButton::clicked, this, [this, fillColorButton]() {
        if (currentTool == bucketTool) {
            bucketTool->setFillColor(Qt::black); // Default to black, and clear texture
            bucketTool->setFillTexture(nullptr);
            fillColorButton->setStyleSheet(QString("background-color: %1;").arg(QColor(Qt::black).name()));
        } else if (currentTool == pointerTool && canvas->selectedFigure != nullptr) {
            canvas->selectedFigure->setFillColor(QColor()); // Set to invalid color (no fill)
            canvas->selectedFigure->removeTexture(); // Also remove texture
            fillColorButton->setStyleSheet("background-color: none;");
            canvas->update();
        }
    });

    connect(selectImgButton, &QPushButton::clicked, this, [this]() {
        QString selectedTexturePath = QFileDialog::getOpenFileName(
                        this,
                        "Select Texture Image",
                        "",
                        "Images (*.png *.jpg *.jpeg *.bmp *.gif)"
                    );
        if (!selectedTexturePath.isEmpty()) {
            if (currentTool == bucketTool) {
                bucketToolTexture.load(selectedTexturePath);
                if (!bucketToolTexture.isNull()) {
                    bucketTool->setFillTexture(&bucketToolTexture);
                    qDebug() << "Bucket tool texture set to:" << selectedTexturePath;
                } else {
                    qDebug() << "Failed to load texture for bucket tool:" << selectedTexturePath;
                    bucketTool->setFillTexture(nullptr); // Fallback
                }
            } else if (currentTool == pointerTool && canvas->selectedFigure != nullptr) {
                qDebug() << "Selected texture path for figure:" << selectedTexturePath;
                canvas->selectedFigure->setTexturePath(selectedTexturePath);
                canvas->update();
            }
        }
    });

    connect(removeImgButton, &QPushButton::clicked, this, [this]() {
        if (currentTool == bucketTool) {
            bucketTool->setFillTexture(nullptr); // Clears texture, bucket tool reverts to color fill
            qDebug() << "Bucket tool texture removed.";
        } else if (currentTool == pointerTool && canvas->selectedFigure != nullptr) {
            canvas->selectedFigure->removeTexture();
            canvas->update();
        }
    });

    setCurrentTool(pointerTool); // Set pointer as default tool
}

MainWindow::~MainWindow()
{
    delete ui;
    delete pointerTool;
    delete lineTool;
    delete polygonTool;
    delete circleTool;
    delete rectangleTool;
    delete clippingTool;
    delete bucketTool;
}

void MainWindow::onPointerButtonClicked()
{
    setCurrentTool(pointerTool);
}

void MainWindow::onLineButtonClicked()
{
    setCurrentTool(lineTool);
}

void MainWindow::onPolygonButtonClicked()
{
    setCurrentTool(polygonTool);
}

void MainWindow::onCircleButtonClicked()
{
    setCurrentTool(circleTool);
}

void MainWindow::onRectangleButtonClicked()
{
    setCurrentTool(rectangleTool);
}

void MainWindow::onCliperkaButtonClicked() {
    setCurrentTool(clippingTool);
}

void MainWindow::onBucketButtonClicked()
{
    setCurrentTool(bucketTool);
}

void MainWindow::setCurrentTool(Tool* tool)
{
    currentTool = tool;
    canvas->setCurrentTool(tool);
    qDebug() << "Current tool set to:" << QString::fromStdString(tool->name());
    statusBar()->showMessage(QString("Current tool: ") + QString::fromStdString(tool->name()));

    // Get UI elements - ensure they are consistently named and available
    QLabel* figureInfoLabel = findChild<QLabel*>("figureInfoLabel");
    QSpinBox* thicknessSpinBox = findChild<QSpinBox*>("thicknessSpinBox");
    QLabel* colorLabel = findChild<QLabel*>("colorLabel"); // Border color label
    QLabel* thicknessLabel = findChild<QLabel*>("thicknessLabel");
    QPushButton* borderColorButton = findChild<QPushButton*>("colorButton"); // Border color button
    QLabel* fillColorLabelWidget = findChild<QLabel*>("fillColorLabel");
    QPushButton* fillColorButtonWidget = findChild<QPushButton*>("fillColorButton");
    QPushButton* clearColorButtonWidget = findChild<QPushButton*>("clearColorButton");
    QPushButton* selectImgButtonWidget = findChild<QPushButton*>("selectImgButton");
    QPushButton* removeImgButtonWidget = findChild<QPushButton*>("removeImgButton");

    // Default: hide all figure-specific controls
    if (figureInfoLabel) figureInfoLabel->setVisible(true);
    if (thicknessSpinBox) thicknessSpinBox->setVisible(false);
    if (borderColorButton) borderColorButton->setVisible(false);
    if (colorLabel) colorLabel->setVisible(false);
    if (thicknessLabel) thicknessLabel->setVisible(false);
    if (fillColorButtonWidget) fillColorButtonWidget->setVisible(false);
    if (fillColorLabelWidget) fillColorLabelWidget->setVisible(false);
    if (clearColorButtonWidget) clearColorButtonWidget->setVisible(false);
    if (selectImgButtonWidget) selectImgButtonWidget->setVisible(false);
    if (removeImgButtonWidget) removeImgButtonWidget->setVisible(false);


    if (tool == pointerTool) {
        if (figureInfoLabel) figureInfoLabel->setText("Select a figure or a point.");
        // Visibility of controls for pointer tool is handled by figureSelected/Deselected signals
        // If a figure is already selected, its info should be shown.
        // We might need to explicitly call a refresh here if canvas->selectedFigure exists.
        if (canvas->selectedFigure) {
             emit canvas->figureSelected(canvas->selectedFigure->getBorderColor(), canvas->selectedFigure->getFillColor(), canvas->selectedFigure->getThickness());
        } else {
             emit canvas->figureDeselected();
        }
    } else if (tool == bucketTool) {
        if (figureInfoLabel) figureInfoLabel->setText("Click to fill area.");
        if (fillColorButtonWidget) {
            fillColorButtonWidget->setVisible(true);
            fillColorButtonWidget->setStyleSheet(QString("background-color: %1;").arg(bucketTool->getFillColor().name()));
        }
        if (fillColorLabelWidget) fillColorLabelWidget->setVisible(true);
        if (clearColorButtonWidget) clearColorButtonWidget->setVisible(true);
        if (selectImgButtonWidget) selectImgButtonWidget->setVisible(true);
        if (removeImgButtonWidget) removeImgButtonWidget->setVisible(true);
    } else { // For other drawing tools
        if (figureInfoLabel) figureInfoLabel->setText(QString("Drawing %1").arg(QString::fromStdString(tool->name())));
        // All figure modification controls remain hidden
    }
}
