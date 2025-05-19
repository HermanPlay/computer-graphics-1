#include "figure.h"
#include "line.h"
#include "polygon.h"
#include "qcolor.h"
#include "qpainter.h"
#include "qpoint.h"
#include <QDebug>


void Polygon::draw(QPainter& painter) {
    painter.setPen(borderColor);

    if (vertices.size() < 2) {
        for (const auto& vertex : vertices) {
            painter.drawEllipse(vertex, radius, radius);
        }
        return;
    }
    if (fillColor != nullptr) {
        FillFigure(painter, vertices, fillColor);
    } else if (texture != nullptr) {
        FillFigureWithTexture(painter, vertices, *texture);
    }

    // Draw all edges of the polygon
    for (size_t i = 0; i < vertices.size(); ++i) {
        Line line1(vertices[i], vertices[(i+1)%vertices.size()]);
        line1.setAntiAliasing(isAntiAliasingEnabled());
        line1.setBorderColor(borderColor);
        line1.setThickness(thickness);
        line1.draw(painter);    }

    for (const QPoint& vertex : vertices) {
        painter.drawEllipse(vertex, radius, radius); // Radius of 3
    }

    if (!isPreview) {
        QColor currentBrush = painter.brush().color();
        QColor currentPen = painter.pen().color();
        painter.setBrush(Qt::red);
        painter.setPen(Qt::red);
        for (const QPoint& midpoint : getEdgeMidpoints()) {
            painter.drawEllipse(midpoint, radius, radius);
        }

        painter.setBrush(Qt::blue);
        painter.setPen(Qt::blue);
        painter.drawEllipse(getPolygonMidpoint(), radius, radius);

        painter.setBrush(currentBrush);
        painter.setPen(currentPen);
    }
}

std::vector<QPoint> Polygon::getEdgeMidpoints() const {
    std::vector<QPoint> midpoints;
    if (vertices.size() < 2) return midpoints;

    for (size_t i = 0; i < vertices.size(); ++i) {
        const QPoint& p1 = vertices[i];
        const QPoint& p2 = vertices[(i + 1) % vertices.size()];
        midpoints.push_back(QPoint((p1.x() + p2.x()) / 2, (p1.y() + p2.y()) / 2));
    }
    return midpoints;
}

QPoint Polygon::getPolygonMidpoint() const {
    if (vertices.empty()) return QPoint();

    int sumX = 0, sumY = 0;
    for (const QPoint& vertex : vertices) {
        sumX += vertex.x();
        sumY += vertex.y();
    }
    return QPoint(sumX / vertices.size(), sumY / vertices.size());
}

void Polygon::popLast() {
    vertices.pop_back();
}

bool Polygon::isConcave() {
    if (vertices.size() < 4) {
        return false;
    }

    bool hasPositiveCrossProduct = false;
    bool hasNegativeCrossProduct = false;

    for (size_t i = 0; i < vertices.size(); ++i) {
        QPoint p1 = vertices[i];
        QPoint p2 = vertices[(i + 1) % vertices.size()];
        QPoint p3 = vertices[(i + 2) % vertices.size()];

        int dx1 = p2.x() - p1.x();
        int dy1 = p2.y() - p1.y();
        int dx2 = p3.x() - p2.x();
        int dy2 = p3.y() - p2.y();

        int crossProduct = dx1 * dy2 - dy1 * dx2;

        if (crossProduct > 0) {
            hasPositiveCrossProduct = true;
        } else if (crossProduct < 0) {
            hasNegativeCrossProduct = true;
        }

        if (hasPositiveCrossProduct && hasNegativeCrossProduct) {
            return true;
        }
    }

    return false;
}
