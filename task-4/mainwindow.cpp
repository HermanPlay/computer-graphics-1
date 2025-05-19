#include "mainwindow.h"
#include "circleTool.h"
#include "clippingTool.h"
#include "polygonTool.h"
#include "qdebug.h"
#include "qfiledialog.h"
#include "qguiapplication_platform.h"
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
    QPushButton* colorButton = findChild<QPushButton*>("colorButton");
    QLabel* fillColorLabel = findChild<QLabel*>("fillColorLabel");
    QPushButton* fillColorButton = findChild<QPushButton*>("fillColorButton");
    QPushButton* clearColorButton = findChild<QPushButton*>("clearColorButton");
    QPushButton* selectImgButton = findChild<QPushButton*>("selectImgButton");
    QPushButton* removeImgButton = findChild<QPushButton*>("removeImgButton");


    if (!canvas || !figureInfoLabel || !thicknessSpinBox || !colorButton ||
        !colorLabel || !thicknessLabel || !fillColorLabel || !fillColorButton || !clearColorButton || !selectImgButton || !removeImgButton) {
        qDebug() << "Error: Required widgets not found in UI!";
        return;
    }

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
        // Update UI elements when a figure is selected
        statusBar()->showMessage(QString("Selected Figure - Border: %1, Fill: %2, Thickness: %3")
                             .arg(borderColor.name()).arg(fillColor.name()).arg(thickness));
        figureInfoLabel->setVisible(false);
        colorButton->setStyleSheet(QString("background-color: %1;").arg(borderColor.name()));
        colorButton->setVisible(true);
        colorLabel->setVisible(true);
        fillColorButton->setStyleSheet(QString("background-color: %1;").arg(fillColor.name()));
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
        // Reset UI elements when no figure is selected
        statusBar()->clearMessage();
        canvas->selectFigure(nullptr);
        figureInfoLabel->setVisible(true);
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
        if (canvas->selectedFigure != nullptr) {
            qDebug() << "Changing thickness of selected figure to" << value;
            canvas->selectedFigure->setThickness(value);
            canvas->update(); // Repaint the canvas
        } else {
            qDebug() << "No figure selected while adjusting thickness.";
        }
    });

    connect(colorButton, &QPushButton::clicked, this, [this, colorButton]() {
        if (canvas->selectedFigure != nullptr) {
            QColor selectedColor = QColorDialog::getColor(canvas->selectedFigure->getBorderColor(), this, "Select Color");
            if (selectedColor.isValid()) {
                qDebug() << "Changing color of selected figure to" << selectedColor.name();
                canvas->selectedFigure->setBorderColor(selectedColor);
                colorButton->setStyleSheet(QString("background-color: %1;").arg(selectedColor.name()));
                canvas->update(); // Repaint the canvas
            }
        } else {
            qDebug() << "No figure selected while changing color.";
        }
    });

    connect(fillColorButton, &QPushButton::clicked, this, [this, fillColorButton]() {
        if (canvas->selectedFigure != nullptr) {
            QColor selectedColor = QColorDialog::getColor(canvas->selectedFigure->getFillColor(),
                                                       this, "Select Fill Color");
            if (selectedColor.isValid()) {
                qDebug() << "Changing fill color of selected figure to" << selectedColor.name();
                canvas->selectedFigure->setFillColor(selectedColor);
                fillColorButton->setStyleSheet(QString("background-color: %1;").arg(selectedColor.name()));
                canvas->update(); // Repaint the canvas
            }
        } else {
            qDebug() << "No figure selected while changing fill color.";
        }
    });

    connect(clearColorButton, &QPushButton::clicked, this, [this, clearColorButton]() {
        if (canvas->selectedFigure != nullptr) {
            canvas->selectedFigure->setFillColor(nullptr);
            canvas->update();
        }
    });

    connect(selectImgButton, &QPushButton::clicked, this, [this, selectImgButton]() {
        if (canvas->selectedFigure != nullptr) {
            QString selectedTexturePath= QFileDialog::getOpenFileName(
                            this,
                            "Select Texture Image",
                            "",
                            "Images (*.png *.jpg *.jpeg *.bmp *.gif)"
                        );
            if (!selectedTexturePath.isEmpty()) {
                qDebug() << "Selected texture path:" << selectedTexturePath;
                canvas->selectedFigure->setTexturePath(selectedTexturePath);
                canvas->update();
            }
        }
    });

    connect(removeImgButton, &QPushButton::clicked, this, [this, removeImgButton]() {
        if (canvas->selectedFigure != nullptr) {
            canvas->selectedFigure->removeTexture();
            canvas->update();
        }
    });

    pointerTool = new Pointer(canvas);
    lineTool = new LineTool();
    polygonTool = new PolygonTool();
    circleTool = new CircleTool();
    rectangleTool = new RectangleTool();
    clippingTool = new ClippingTool(canvas, canvas->clippingManager);
    setCurrentTool(pointerTool);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete pointerTool;
    delete lineTool;
    delete polygonTool;
    delete circleTool;
    delete rectangleTool;
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
    // Placeholder for Polygon tool logic
    setCurrentTool(polygonTool);
}

void MainWindow::onCircleButtonClicked()
{
    // Placeholder for Circle tool logic
    setCurrentTool(circleTool);
}

void MainWindow::onRectangleButtonClicked()
{
    // Set the current tool to Rectangle
    setCurrentTool(rectangleTool);
}

void MainWindow::onCliperkaButtonClicked() {
    // Set the current tool to clipping tool
    setCurrentTool(clippingTool);
}

void MainWindow::setCurrentTool(Tool* tool)
{
    currentTool = tool;
    canvas->setCurrentTool(tool); // Ensure the canvas is aware of the current tool
    qDebug() << "Current tool set to:" << QString::fromStdString(tool->name());
    statusBar()->showMessage(QString("Current tool: ") + QString::fromStdString(tool->name()));
}
