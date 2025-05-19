#include "rectangle.h"
#include "line.h"
#include "qpoint.h"
#include <QDebug>

void Rectangle::draw(QPainter& painter) {
    painter.setPen(borderColor);

    QPoint topLeft = getTopLeft();
    QPoint topRight = getTopRight();
    QPoint bottomLeft = getBottomLeft();
    QPoint bottomRight = getBottomRight();

    Line topLine(topLeft, topRight);
    Line rightLine(topRight, bottomRight);
    Line bottomLine(bottomRight, bottomLeft);
    Line leftLine(bottomLeft, topLeft);

    topLine.setAntiAliasing(isAntiAliasingEnabled());
    rightLine.setAntiAliasing(isAntiAliasingEnabled());
    bottomLine.setAntiAliasing(isAntiAliasingEnabled());
    leftLine.setAntiAliasing(isAntiAliasingEnabled());

    topLine.setBorderColor(borderColor);
    rightLine.setBorderColor(borderColor);
    bottomLine.setBorderColor(borderColor);
    leftLine.setBorderColor(borderColor);

    topLine.setThickness(thickness);
    rightLine.setThickness(thickness);
    bottomLine.setThickness(thickness);
    leftLine.setThickness(thickness);

    auto vertices = getVertices();
    if (fillColor != nullptr) {
        FillFigure(painter, vertices, fillColor);
    } else if (texture != nullptr) {
        FillFigureWithTexture(painter, vertices, *texture);
    }
    topLine.draw(painter);
    rightLine.draw(painter);
    bottomLine.draw(painter);
    leftLine.draw(painter);

    painter.setPen(Qt::black);
    for (const auto& vertex : vertices) {
        painter.drawEllipse(vertex, radius, radius);
    }

    // Draw midpoints for edges if not in preview mode
    if (!isPreview) {
        QColor currentBrush = painter.brush().color();
        painter.setBrush(Qt::red);
        for (const QPoint& midpoint : getEdgeMidpoints()) {
            painter.drawEllipse(midpoint, radius, radius);
        }
        painter.setBrush(currentBrush);
    }
}

std::vector<QPoint> Rectangle::getVertices() {
    std::vector<QPoint> vertices;
    vertices.push_back(getTopLeft());
    vertices.push_back(getTopRight());
    vertices.push_back(getBottomRight());
    vertices.push_back(getBottomLeft());
    return vertices;
}

void Rectangle::moveVertex(int selectedVertexIndex, QPoint delta) {
    switch (selectedVertexIndex) {
        case 0: // Top-left
            corner1.rx() += delta.x();
            corner1.ry() += delta.y();
            break;
        case 1: // Top-right
            corner2.rx() += delta.x();
            corner1.ry() += delta.y();
            break;
        case 2: // Bottom-right
            corner2.rx() += delta.x();
            corner2.ry() += delta.y();
            break;
        case 3: // Bottom-left
            corner1.rx() += delta.x();
            corner2.ry() += delta.y();
            break;
        default:
            qDebug() << "Invalid vertex index: " << selectedVertexIndex;
    }
}

std::vector<QPoint> Rectangle::getEdgeMidpoints() const {
    std::vector<QPoint> midpoints;
    QPoint topLeft = getTopLeft();
    QPoint topRight = getTopRight();
    QPoint bottomRight = getBottomRight();
    QPoint bottomLeft = getBottomLeft();

    // Calculate midpoints for all four edges
    midpoints.push_back(QPoint((topLeft.x() + topRight.x()) / 2, (topLeft.y() + topRight.y()) / 2));
    midpoints.push_back(QPoint((topRight.x() + bottomRight.x()) / 2, (topRight.y() + bottomRight.y()) / 2));
    midpoints.push_back(QPoint((bottomRight.x() + bottomLeft.x()) / 2, (bottomRight.y() + bottomLeft.y()) / 2));
    midpoints.push_back(QPoint((bottomLeft.x() + topLeft.x()) / 2, (bottomLeft.y() + topLeft.y()) / 2));

    return midpoints;
}

QPoint Rectangle::getTopLeft() const {
    return QPoint(std::min(corner1.x(), corner2.x()), std::min(corner1.y(), corner2.y()));
}

QPoint Rectangle::getTopRight() const {
    return QPoint(std::max(corner1.x(), corner2.x()), std::min(corner1.y(), corner2.y()));
}

QPoint Rectangle::getBottomLeft() const {
    return QPoint(std::min(corner1.x(), corner2.x()), std::max(corner1.y(), corner2.y()));
}

QPoint Rectangle::getBottomRight() const {
    return QPoint(std::max(corner1.x(), corner2.x()), std::max(corner1.y(), corner2.y()));
}

void Rectangle::moveEdge(int edgeIndex, const QPoint& delta) {
    switch(edgeIndex) {
        case 0: // Top edge
            if (corner1.y() < corner2.y()) {
                corner1.setY(corner1.y() + delta.y());
            } else {
                corner2.setY(corner2.y() + delta.y());
            }
            break;
        case 1: // Right edge
            if (corner1.x() > corner2.x())
                corner1.setX(corner1.x() + delta.x());
            else
                corner2.setX(corner2.x() + delta.x());
            break;

        case 2: // Bottom edge
            if (corner1.y() > corner2.y())
                corner1.setY(corner1.y() + delta.y());
            else
                corner2.setY(corner2.y() + delta.y());
            break;

        case 3: // Left edge
            if (corner1.x() < corner2.x())
                corner1.setX(corner1.x() + delta.x());
            else
                corner2.setX(corner2.x() + delta.x());
            break;
    }
}
