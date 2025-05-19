#include "line.h"
#include "qcolor.h"
#include "qcontainerfwd.h"
#include "qdebug.h"
#include "qlogging.h"
#include "qnamespace.h"
#include "qpainter.h"
#include <algorithm>
#include <vector>
#include <cmath>
#include <numbers>



void Line::draw(QPainter& painter) {
    painter.setPen(borderColor);
    if (!isAntiAliasingEnabled()) {
        bresenhamLine(painter);
    } else {
        guptaSproullsLine(painter);
    }

    // Endpoint and midpoint markers
    if (drawTrash) {
        painter.setPen(Qt::black);
        painter.drawEllipse(start, radius, radius);
        painter.drawEllipse(end, radius, radius);

        const auto& midpoints = getEdgeMidpoints();
        QColor current = painter.brush().color();
        painter.setBrush(Qt::red);
        painter.drawEllipse(midpoints[0], radius, radius);
        painter.setBrush(current);
    }
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

float Line::cov(const float d, const float r) {
    if (d >= r)
        return 0;
    return acos(d/r)/std::numbers::pi_v<double> - d/(r*r*std::numbers::pi_v<double>)*sqrt(r*r-d*d);
}

float Line::coverage(const float thickness, const float dist, const float r) {
    float w = thickness / 2;
    float d = fabs(dist);
    if (w >= r) {
        if (w <= d) {
            return cov(d-w, r);
        } else {
            return 1 - cov(w-d, r);
        }
    } else {
        if (d >= 0 && d <= w) {
            return 1 - cov(w-d, r) - cov(w+d, r);
        } else if(d >= w && d <= r-w) {
            return cov(d-w, r) - cov(d+w, r);
        }else  {
            return cov(d-w, r);
        }
    }

}

void Line::guptaSproullsLine(QPainter& painter) {
    int x1 = start.x(), y1 = start.y();
    int x2 = end.x(), y2 = end.y();
    if (x1 == x2) {
        bresenhamLine(painter);
        return;
    }

    bool steep = std::abs(y2 - y1) > std::abs(x2 - x1);
    if (steep) {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }
    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    int dx = x2 - x1;
    int dy = y2 - y1;
    int ystep = (dy >= 0) ? 1 : -1;
    dy = std::abs(dy);

    int d = 2 * dy - dx;
    int dE = 2 * dy;
    int dNE = 2 * (dy - dx);
    int x = x1, y = y1;
    int two_v_dx = 0;
    float invDenom = 1.0f / (2.0f * std::sqrt(dx * dx + dy * dy));
    float two_dx_invDenom = 2.0f * dx * invDenom;

    auto intensifyPixel = [&](int x, int y, float thickness, float dist) {
        float r = 0.5f;
        float cov = coverage(thickness, dist, r);
        QColor tempColor = QColor(borderColor);
        tempColor.setAlphaF(cov);
        painter.setPen(tempColor);
        if (steep) {
            painter.drawPoint(y, x);
        } else {
            painter.drawPoint(x, y);
        }
        return cov;
    };

    intensifyPixel(x, y, thickness, two_v_dx*invDenom);
    for (int i = 1; intensifyPixel(x, y+i, thickness, i*two_dx_invDenom) ;++i);
    for (int i = 1; intensifyPixel(x, y-i, thickness, i*two_dx_invDenom) ;++i);

    while (x < x2) {
            x++;
        if (d < 0) {
            two_v_dx = d + dx;
            d += dE;
        } else {
            two_v_dx = d - dx;
            d += dNE;
            y+=ystep;
        }

        intensifyPixel(x, y, thickness, two_v_dx*invDenom);
        for (int i = 1; intensifyPixel(x, y+i*ystep, thickness, i*two_dx_invDenom - two_v_dx*invDenom) ;++i);
        for (int i = 1; intensifyPixel(x, y-i*ystep, thickness, i*two_dx_invDenom + two_v_dx*invDenom) ;++i);

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
        .arg(borderColor.name()); // Save color as a hex string
}
