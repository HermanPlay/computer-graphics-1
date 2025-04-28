#include "mainwindow.h"
#include "circleTool.h"
#include "polygonTool.h"
#include "qdebug.h"
#include "qlabel.h"
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

    if (!canvas || !figureInfoLabel ||!thicknessSpinBox || !colorButton || !colorLabel || !thicknessLabel) {
        qDebug() << "Error: Required widgets not found in UI!";
        return;
    }

    thicknessSpinBox->setVisible(false);
    colorButton->setVisible(false);
    colorLabel->setVisible(false);
    thicknessLabel->setVisible(false);

    connect(findChild<QPushButton*>("mousePointerButton"), &QPushButton::clicked, this, &MainWindow::onPointerButtonClicked);
    connect(findChild<QPushButton*>("lineButton"), &QPushButton::clicked, this, &MainWindow::onLineButtonClicked);
    connect(findChild<QPushButton*>("polygonButton"), &QPushButton::clicked, this, &MainWindow::onPolygonButtonClicked);
    connect(findChild<QPushButton*>("circleButton"), &QPushButton::clicked, this, &MainWindow::onCircleButtonClicked);
    connect(findChild<QPushButton*>("pillButton"), &QPushButton::clicked, this, &MainWindow::onPillButtonClicked);


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

    connect(canvas, &Canvas::figureSelected, this, [this, figureInfoLabel, thicknessSpinBox, colorButton, colorLabel, thicknessLabel](const QColor& color, int thickness) {
        // Update UI elements when a figure is selected
        statusBar()->showMessage(QString("Selected Figure - Color: %1, Thickness: %2")
                                 .arg(color.name()).arg(thickness));
        figureInfoLabel->setVisible(false);
        colorButton->setStyleSheet(QString("background-color: %1;").arg(color.name()));
        colorButton->setVisible(true);
        colorLabel->setVisible(true);
        thicknessSpinBox->setVisible(true);
        thicknessLabel->setVisible(true);
        thicknessSpinBox->setValue(thickness);
    });

    connect(canvas, &Canvas::figureDeselected, this, [this, figureInfoLabel, thicknessSpinBox, colorButton, colorLabel, thicknessLabel]() {
        // Reset UI elements when no figure is selected
        statusBar()->clearMessage();
        canvas->selectFigure(nullptr);
        figureInfoLabel->setVisible(true);
        colorButton->setVisible(false);
        colorLabel->setVisible(false);
        thicknessSpinBox->setVisible(false);
        thicknessLabel->setVisible(false);
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
            QColor selectedColor = QColorDialog::getColor(canvas->selectedFigure->getColor(), this, "Select Color");
            if (selectedColor.isValid()) {
                qDebug() << "Changing color of selected figure to" << selectedColor.name();
                canvas->selectedFigure->setColor(selectedColor);
                colorButton->setStyleSheet(QString("background-color: %1;").arg(selectedColor.name()));
                canvas->update(); // Repaint the canvas
            }
        } else {
            qDebug() << "No figure selected while changing color.";
        }
    });

    pointerTool = new Pointer(canvas); 
    lineTool = new LineTool(); 
    polygonTool = new PolygonTool();
    circleTool = new CircleTool();
    pillTool = new PillTool();

    setCurrentTool(pointerTool);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete pointerTool;
    delete lineTool;  
    delete polygonTool;
    delete circleTool;
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

void MainWindow::onPillButtonClicked()
{
    // Placeholder for Circle tool logic
    setCurrentTool(pillTool);
}
void MainWindow::setCurrentTool(Tool* tool)
{
    currentTool = tool;
    canvas->setCurrentTool(tool); // Ensure the canvas is aware of the current tool
    qDebug() << "Current tool set to:" << QString::fromStdString(tool->name());
    statusBar()->showMessage(QString("Current tool: ") + QString::fromStdString(tool->name()));
}
