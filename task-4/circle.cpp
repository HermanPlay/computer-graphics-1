#include "circle.h"
#include "qpainter.h"
#include <cmath>

void Circle::draw(QPainter& painter)
{
    painter.setPen(borderColor);

    int radius = std::sqrt(std::pow(pointOnCircle.x() - center.x(), 2) +
                           std::pow(pointOnCircle.y() - center.y(), 2));

    auto putPixel = [&](int x, int y) {
        for (int dx = -thickness / 2; dx <= thickness / 2; ++dx) {
            for (int dy = -thickness / 2; dy <= thickness / 2; ++dy) {
                painter.drawPoint(center.x() + x + dx, center.y() + y + dy);
                painter.drawPoint(center.x() - x + dx, center.y() + y + dy);
                painter.drawPoint(center.x() + x + dx, center.y() - y + dy);
                painter.drawPoint(center.x() - x + dx, center.y() - y + dy);
                painter.drawPoint(center.x() + y + dx, center.y() + x + dy);
                painter.drawPoint(center.x() - y + dx, center.y() + x + dy);
                painter.drawPoint(center.x() + y + dx, center.y() - x + dy);
                painter.drawPoint(center.x() - y + dx, center.y() - x + dy);
            }
        }
    };

    int dE = 3;
    int dSE = 5 - 2 * radius;
    int d = 1 - radius;
    int x = 0;
    int y = radius;

    putPixel(x, y);
    while (y > x) {
        if (d < 0) {
            d += dE;
            dE += 2;
            dSE += 2;
        } else {
            d += dSE;
            dE += 2;
            dSE += 4;
            --y;
        }
        ++x;
        putPixel(x, y);
    }

    painter.drawEllipse(center, this->radius, this->radius); // Small dot for center
    painter.drawEllipse(pointOnCircle, this->radius, this->radius); // Small dot for point on circle
}
