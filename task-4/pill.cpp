#include "pill.h"
#include "line.h"

#include <QPainter>
#include <cmath>

void Pill::draw(QPainter& painter) {
    int circleRadius = std::sqrt(std::pow(Point.x() - center2.x(), 2) + std::pow(Point.y() - center2.y(), 2));

    auto determineCurrentCenter = [&](const QPointF& point) -> QPointF {
        QPointF vectorToPoint = point - QPointF(center1.x(), center1.y());
        QPointF vectorCenter1ToCenter2 = center2 - center1;

        double dotProduct = QPointF::dotProduct(vectorToPoint, vectorCenter1ToCenter2);
        return (dotProduct > 0) ? center2 : center1;
    };

    auto putPixel = [&](int x, int y) {
        QPointF currentCenter = determineCurrentCenter(QPointF(center1.x()+x, center1.y()+y));
        painter.drawPoint(currentCenter.x() + x, currentCenter.y() + y);

        currentCenter = determineCurrentCenter(QPointF(center1.x()+x, center1.y()-y));
        painter.drawPoint(currentCenter.x() + x, currentCenter.y() - y);

        currentCenter = determineCurrentCenter(QPointF(center1.x()-x, center1.y()+y));
        painter.drawPoint(currentCenter.x() - x, currentCenter.y() + y);

        currentCenter = determineCurrentCenter(QPointF(center1.x()-x, center1.y()-y));
        painter.drawPoint(currentCenter.x() - x, currentCenter.y() - y);

        currentCenter = determineCurrentCenter(QPointF(center1.x()+y, center1.y()+x));
        painter.drawPoint(currentCenter.x() + y, currentCenter.y() + x);

        currentCenter = determineCurrentCenter(QPointF(center1.x()-y, center1.y()+x));
        painter.drawPoint(currentCenter.x() - y, currentCenter.y() + x);

        currentCenter = determineCurrentCenter(QPointF(center1.x()+y, center1.y()-x));
        painter.drawPoint(currentCenter.x() + y, currentCenter.y() - x);

        currentCenter = determineCurrentCenter(QPointF(center1.x()-y, center1.y()-x));
        painter.drawPoint(currentCenter.x() - y, currentCenter.y() - x);
    };

    int dE = 3;
    int dSE = 5 - 2 * circleRadius;
    int d = 1 - circleRadius;
    int x = 0;
    int y = circleRadius;

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

    painter.drawEllipse(center1, radius, radius);
    painter.drawEllipse(center2, radius, radius);
    painter.drawEllipse(Point, radius, radius);

    QPointF vector = center2 - center1;
    double length = std::sqrt(vector.x() * vector.x() + vector.y() * vector.y());
    QPointF unitVector = vector / length;

    QPointF perpendicular(-unitVector.y(), unitVector.x());
    QPointF offset = perpendicular * circleRadius;

    QPoint rectStart1 = center1 + offset.toPoint();
    QPoint rectStart2 = center1 - offset.toPoint();
    QPoint rectEnd1 = center2 + offset.toPoint();
    QPoint rectEnd2 = center2 - offset.toPoint();


    Line line1(rectStart1, rectEnd1);
    line1.setAntiAliasing(isAntiAliasingEnabled());
    line1.setBorderColor(color);
    line1.setThickness(thickness);
    line1.draw(painter);

    Line line2(rectStart2, rectEnd2);
    line2.setAntiAliasing(isAntiAliasingEnabled());
    line2.setBorderColor(color);
    line2.setThickness(thickness);
    line2.draw(painter);
}
