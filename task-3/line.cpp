#include "line.h"
#include "qcolor.h"
#include "qcontainerfwd.h"
#include "qdebug.h"
#include "qnamespace.h"
#include "qpainter.h"
#include <vector>

void Line::draw(QPainter& painter) {
    painter.setPen(color);
    if (!isAntiAliasingEnabled()) {
        bresenhamLine(painter);
    } else {
        guptaSproullsLine(painter);
    }

    // Endpoint and midpoint markers
    painter.setPen(Qt::black);
    painter.drawEllipse(start, radius, radius);
    painter.drawEllipse(end, radius, radius);

    const auto& midpoints = getEdgeMidpoints();
    QColor current = painter.brush().color();
    painter.setBrush(Qt::red);
    painter.drawEllipse(midpoints[0], radius, radius);
    painter.setBrush(current);
}

void Line::bresenhamLine(QPainter& painter) {

    int x1 = start.x();
    int y1 = start.y();
    int x2 = end.x();
    int y2 = end.y();
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    int halfThickness = (thickness - 1) / 2;
    bool isEvenThickness = (static_cast<int>(thickness) % 2 == 0);

    bool isSteep = dy > dx;

    while (true) {
        if (isSteep) {
            for (int i = -halfThickness; i <= halfThickness; ++i) {
                painter.drawPoint(x1 + i, y1);
            }
            if (isEvenThickness && thickness > 1) painter.drawPoint(x1 + halfThickness + 1, y1); 
        } else {
            for (int i = -halfThickness; i <= halfThickness; ++i) {
                painter.drawPoint(x1, y1 + i);
            }
            if (isEvenThickness && thickness > 1) painter.drawPoint(x1, y1+ halfThickness + 1);
        }

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx)  { err += dx; y1 += sy; }
    }
}

void Line::guptaSproullsLine(QPainter& painter) {
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
}

std::vector<QPoint> Line::getEdgeMidpoints() const{
    std::vector<QPoint> midpoints;
    midpoints.push_back(QPoint((start.x() + end.x()) / 2, (start.y() + end.y()) / 2));
    return midpoints;
}

QString Line::save() const {
    return QString("Line,%1,%2,%3,%4,%5")
        .arg(start.x())
        .arg(start.y())
        .arg(end.x())
        .arg(end.y())
        .arg(color.name()); // Save color as a hex string
}