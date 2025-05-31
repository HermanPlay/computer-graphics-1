#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cube.h"
#include <QVBoxLayout> // For layout
#include <QSlider>    // For sliders

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // It's better to create the Cube widget here and add it to a layout
    cube = new Cube(this); // Create the Cube widget

    QSlider *xSlider = new QSlider(Qt::Horizontal, this);
    xSlider->setRange(-360, 360);
    xSlider->setValue(0);
    xSlider->setTickInterval(1);
    connect(xSlider, &QSlider::valueChanged, this, &MainWindow::on_xRotationSlider_valueChanged);

    QSlider *ySlider = new QSlider(Qt::Horizontal, this);
    ySlider->setRange(-360, 360);
    ySlider->setValue(0);
    ySlider->setTickInterval(1);
    connect(ySlider, &QSlider::valueChanged, this, &MainWindow::on_yRotationSlider_valueChanged);

    // Layout to arrange cube and sliders
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(cube);
    layout->addWidget(xSlider);
    layout->addWidget(ySlider);

    // Set the layout on the central widget
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    // cube->setMinimumSize(200, 200); cube->setMaximumSize(400, 400); cube->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

}

MainWindow::~MainWindow()
{
    delete ui;
    // No need to delete cube if it's a child of MainWindow and layout takes ownership
}

void MainWindow::on_xRotationSlider_valueChanged(int value)
{
    if(cube) {
        cube->rotateX(value);
    }
}

void MainWindow::on_yRotationSlider_valueChanged(int value)
{
    if(cube) {
        cube->rotateY(value);
    }
}
