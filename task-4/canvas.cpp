#include "canvas.h"
#include "bucketTool.h"
#include "clippingManager.h"
#include "figure.h"
#include "qevent.h"
#include "qlogging.h"
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>
#include <QMenu>
#include <QContextMenuEvent>
#include <QFile>
#include <QTextStream>

Canvas::Canvas(QWidget *parent)
    : QWidget(parent), currentTool(nullptr)
{
    setMouseTracking(true); // Enable mouse tracking even when no button is pressed
    clippingManager = new ClippingManager();
}

void Canvas::setCurrentTool(Tool *tool)
{
    qDebug() << "Canvas: setting current tool";
    currentTool = tool;
}

void Canvas::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QImage image(this->width(), this->height(), QImage::Format_ARGB32);
    QPainter painter(&image);
    painter.setBrush(Qt::white);
    painter.drawRect(0, 0, this->width(), this->height());

    for (auto& figure : figures) {
        figure->draw(painter);
    }

    // If the current tool has a preview or finalized lines to draw, draw them
    if (currentTool) {
        currentTool->draw(painter);
    }
    clippingManager->drawClippedSegments(painter);

    // BucketTool* tool = dynamic_cast<BucketTool*>(currentTool);
    // if (tool) {
    //     tool->fill(painter);
    // }
    QPainter canvasPainter(this);
    canvasPainter.drawImage(0, 0, image);
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    if (currentTool) {
        Figure* newFigure = currentTool->onMousePress(event);
        if (newFigure != nullptr) {
            newFigure->setAntiAliasing(antiAliasingEnabled);
            figures.push_back(newFigure);
        }
        update(); // Trigger a repaint to show the finalized line or preview
    } else {
        qDebug() << "No tool is set!";
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (currentTool)
    {
        currentTool->onMouseRelease(event);
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    if (currentTool) {
        if (currentTool->onMouseMove(event)) {
            clippingManager->updateAllClipping();
            update(); // Trigger a repaint to show the preview
        }
    }
}

void Canvas::contextMenuEvent(QContextMenuEvent *event)
{
    if (currentTool->name() != "Pointer") return;
    Figure* figure = getFigureAt(event->pos());
    if (figure) {
        QMenu contextMenu(this);
        QAction* deleteAction = contextMenu.addAction("Delete");
        QAction* selectedAction = contextMenu.exec(event->globalPos());
        if (selectedAction == deleteAction) {
            auto it = std::find(figures.begin(), figures.end(), figure);
            if (it != figures.end()) {
                clippingManager->deletePairs(figure);
                figures.erase(it); // Remove the figure from the vector first
                delete figure; // Safely delete the figure
                update(); // Repaint the canvas
                qDebug() << "Figure deleted via context menu.";
            }
        }
    }
}

Figure* Canvas::getFigureAt(const QPoint& pos)
{
    for (Figure* figure : figures) {
        for (const QPoint& vertex : figure->getVertices()) {
            if (std::sqrt(std::pow(pos.x() - vertex.x(), 2) + std::pow(pos.y() - vertex.y(), 2)) <= figure->radius) {
                return figure;
            }
        }
    }
    return nullptr;
}

void Canvas::clearFigures() {
    for (Figure* figure : figures) {
        clippingManager->deletePairs(figure);
        delete figure;
    }
    figures.clear();
    update(); // Trigger a repaint to clear the canvas
}

void Canvas::saveFigures(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file for saving:" << filePath;
        return;
    }

    QTextStream out(&file);
    for (const Figure* figure : figures) {
        out << figure->save() << "\n";
    }

    file.close();
    qDebug() << "Figures saved to file:" << filePath;
}

void Canvas::loadFigures(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file for loading:" << filePath;
        return;
    }

    QTextStream in(&file);
    clearFigures(); // Clear existing figures before loading new ones

    while (!in.atEnd()) {
        QString line = in.readLine();
        Figure* figure = Figure::fromString(line); // Assuming a static method to parse a figure
        if (figure) {
            figures.push_back(figure);
        }
    }

    file.close();
    update(); // Repaint the canvas to show loaded figures
    for (auto* figure: figures) {
        qDebug() << figure;
    }
    qDebug() << "Figures loaded from file:" << filePath;
}

bool Canvas::isAntiAliasingEnabled() const {
    return antiAliasingEnabled;
}

void Canvas::setAntiAliasingEnabled(bool enabled) {
    qDebug() << "Toggled antialiasing" << enabled;
    if (antiAliasingEnabled != enabled) {
        antiAliasingEnabled = enabled;
        for (auto* figure : figures) {
            figure->setAntiAliasing(antiAliasingEnabled);
        }
        update(); // Repaint the canvas
    }
}


Canvas::~Canvas() {
    delete clippingManager;
    for (auto figure: figures) {
        delete figure;
    }
}

void Canvas::resetClipping() {
    clippingManager->reset();
}
