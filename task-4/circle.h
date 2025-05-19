#ifndef CIRCLE_H
#define CIRCLE_H

#include "figure.h"
#include "qdebug.h"
#include "qpoint.h"
#include <QPoint>
#include <cmath>
#include <vector>

class Circle : public Figure {
private:
    QPoint center;
    QPoint pointOnCircle;

public:
    Circle(QPoint center, QPoint pointOnCircle)
        : center(center), pointOnCircle(pointOnCircle) {}

    void draw(QPainter& painter) override ;
    std::vector<QPoint> getVertices() override {
        std::vector<QPoint> vector{};
        vector.push_back(center);
        vector.push_back(pointOnCircle);
        return vector;
    }

    void moveVertex(int selectedVertexIndex, QPoint delta) override {
        if (selectedVertexIndex == 0) {
            center.rx() += delta.x();
            center.ry() += delta.y();
            pointOnCircle.rx() += delta.x();
            pointOnCircle.ry() += delta.y();
        } else if (selectedVertexIndex == 1) {
            pointOnCircle.rx() += delta.x();
            pointOnCircle.ry() += delta.y();
        } else {
            qDebug() << "Invalid vertex index: " << selectedVertexIndex;
        }
    }

    ~Circle() = default;

    QString save() const override {
        return QString("Circle,%1,%2,%3,%4,%5")
            .arg(center.x())
            .arg(center.y())
            .arg(pointOnCircle.x())
            .arg(pointOnCircle.y())
            .arg(borderColor.name()); // Save color as a hex string
    }

    std::vector<QPoint> getEdgeMidpoints() const override {
        return std::vector<QPoint>();
    }
};

#endif // CIRCLE_H
