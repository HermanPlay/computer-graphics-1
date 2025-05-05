#include "line.h"
#include "polygon.h"
#include "qcolor.h"
#include "qpainter.h"
#include "qpoint.h"
#include <QDebug>


void Polygon::draw(QPainter& painter) {
    painter.setPen(color);

    if (vertices.size() < 2) {
        // Draw larger dots for individual vertices
        for (const auto& vertex : vertices) {
            painter.drawEllipse(vertex, radius, radius);
        }
        return; // Not enough vertices to draw edges
    }

    auto drawLineMidpoint = [&](const QPoint& p1, const QPoint& p2) {
        int x1 = p1.x(), y1 = p1.y();
        int x2 = p2.x(), y2 = p2.y();

        if (x1 > x2) {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }

        int dx = abs(x2 - x1), dy = abs(y2 - y1);
        bool steep = dy > dx;

        if (steep) {
            std::swap(x1, y1);
            std::swap(x2, y2);
            std::swap(dx, dy);
        }

        int sx = (x2 > x1) ? 1 : -1;
        int sy = (y2 > y1) ? 1 : -1;

        int d = 2 * dy - dx;
        int x = x1, y = y1;

        for (int i = 0; i <= dx; ++i) {
            painter.drawPoint(steep ? QPoint(y, x) : QPoint(x, y));
            if (d > 0) {
                y += sy;
                d -= 2 * dx;
            }
            x += sx;
            d += 2 * dy;
        }
    };

    // Draw all edges of the polygon
    for (size_t i = 0; i < vertices.size(); ++i) {
        Line line1(vertices[i], vertices[(i+1)%vertices.size()]);
        line1.setAntiAliasing(isAntiAliasingEnabled());
        line1.setColor(color);
        line1.setThickness(thickness);
        line1.draw(painter);    }

    // Draw larger dots for each vertex
    for (const QPoint& vertex : vertices) {
        painter.drawEllipse(vertex, radius, radius); // Radius of 3
    }

    if (!isPreview) {
        QColor currentBrush = painter.brush().color();
        QColor currentPen = painter.pen().color();
        painter.setBrush(Qt::red); // Set color for midpoints
        painter.setPen(Qt::red);
        for (const QPoint& midpoint : getEdgeMidpoints()) {
            painter.drawEllipse(midpoint, radius, radius);
        }

        // Draw the polygon's middle point
        painter.setBrush(Qt::blue); // Set color for polygon midpoint
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


void Polygon::guptaSproullsLine(QPainter& painter, QPoint start, QPoint end) {
    if (start.x() > end.x()) {
        std::swap(start, end);
    }

    int x1 = start.x(), y1 = start.y();
    int x2 = end.x(), y2 = end.y();

    int dx = x2 - x1;
    int dy = y2 - y1;

    bool swapped = false;
    if (abs(dy) > abs(dx)) {
        std::swap(x1, y1);
        std::swap(x2, y2);
        std::swap(dx, dy);
        swapped = true;
    }

    if (x1 > x2) { // Ensure we always draw from left to right
        std::swap(x1, x2);
        std::swap(y1, y2);
        dx = -dx;
        dy = -dy;
    }

    int d = 2 * dy - dx;
    int dE = 2 * dy;
    int dNE = 2 * (dy - dx);
    int x = x1, y = y1;
    int two_v_dx = 0;
    float invDenom = 1.0f / (2.0f * std::sqrt(dx * dx + dy * dy));
    float two_dx_invDenom = 2.0f * dx * invDenom;

    auto intensifyPixel = [&](int px, int py, float dist) {
        float cov = std::max(0.0f, 1.0f - dist);
        int alpha = static_cast<int>(cov * 255);
        QColor tempColor = QColor(color);
        tempColor.setAlpha(alpha);
        painter.setPen(tempColor);
        if (swapped) {
            painter.drawPoint(py, px); // Swap back if coordinates were swapped
        } else {
            painter.drawPoint(px, py);
        }
    };

    intensifyPixel(x, y, 0);
    for (int i = 1; ; ++i) {
        float dist = i * two_dx_invDenom;
        if (dist >= 1.0f) break;
        intensifyPixel(x, y + i, dist);
        intensifyPixel(x, y - i, dist);
    }

    while (x < x2) {
        ++x;
        if (d < 0) {
            two_v_dx = d + dx;
            d += dE;
        } else {
            two_v_dx = d - dx;
            d += dNE;
            ++y;
        }

        float baseDist = two_v_dx * invDenom;
        intensifyPixel(x, y, std::abs(baseDist));
        for (int i = 1; ; ++i) {
            float upDist = i * two_dx_invDenom - baseDist;
            float downDist = i * two_dx_invDenom + baseDist;
            if (upDist >= 1.0f && downDist >= 1.0f) break;
            if (upDist < 1.0f) intensifyPixel(x, y + i, std::abs(upDist));
            if (downDist < 1.0f) intensifyPixel(x, y - i, std::abs(downDist));
        }
    }
    painter.setPen(color);
}
