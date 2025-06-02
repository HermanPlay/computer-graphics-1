#include "mainwindow.h"
#include "rasterizer.h"
#include "scene.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout> // For layout

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Set up status bar
    mousePositionLabel = new QLabel("Mouse: (0, 0)");
    statusBar()->addWidget(mousePositionLabel);

    // Scene setup
    Scene *scene = new Scene();
    scene->readFromObjFile(
        ":/assets/models/cube.obj"); // Load cube model from OBJ file
    scene->readFromObjFile(":/assets/models/cube2.obj");
    QImage *targetImage =
        new QImage(this->width(), this->height(), QImage::Format_ARGB32);
    rasterizer = new Rasterizer(targetImage, scene);
    rasterizer->renderScene();

    // Set size policies to make rasterizer expand to fill all available space
    rasterizer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rasterizer->setMinimumSize(1, 1);

    // Connect mouse position signal
    connect(rasterizer, &Rasterizer::mousePositionChanged, this,
            &MainWindow::updateMousePosition);

    // Set the rasterizer directly as the central widget to eliminate all
    // margins
    setCentralWidget(rasterizer);
}

MainWindow::~MainWindow() {
    delete ui;
    // No need to delete cube if it's a child of MainWindow and layout takes
    // ownership
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    const int step = 10;             // Step size for camera movement
    const float rotationStep = 5.0f; // Rotation step in degrees

    bool shiftPressed = event->modifiers() & Qt::ShiftModifier;

    switch (event->key()) {
    case Qt::Key_W:
        if (shiftPressed) {
            // Rotate camera up (pitch up)
            rasterizer->RotateCamera(0, rotationStep);
        } else {
            // Move camera forward
            rasterizer->TranslateCamera(QVector3D(0, 0, step));
        }
        break;
    case Qt::Key_S:
        if (shiftPressed) {
            // Rotate camera down (pitch down)
            rasterizer->RotateCamera(0, -rotationStep);
        } else {
            // Move camera backward
            rasterizer->TranslateCamera(QVector3D(0, 0, -step));
        }
        break;
    case Qt::Key_A:
        if (shiftPressed) {
            // Rotate camera left (yaw left)
            rasterizer->RotateCamera(-rotationStep, 0);
        } else {
            // Move camera left
            rasterizer->TranslateCamera(QVector3D(-step, 0, 0));
        }
        break;
    case Qt::Key_D:
        if (shiftPressed) {
            // Rotate camera right (yaw right)
            rasterizer->RotateCamera(rotationStep, 0);
        } else {
            // Move camera right
            rasterizer->TranslateCamera(QVector3D(step, 0, 0));
        }
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::updateMousePosition(int x, int y) {
    mousePositionLabel->setText(QString("Mouse: (%1, %2)").arg(x).arg(y));
}