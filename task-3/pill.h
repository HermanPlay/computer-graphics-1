#ifndef PILL_H
#define PILL_H

#include "figure.h"
#include "qpoint.h"
class Pill : public Figure {
private:
    QPoint center1;
    QPoint center2;
    QPoint Point;

public: 
    Pill(QPoint center1, QPoint center2, QPoint Point) 
        : center1(center1), center2(center2), Point(Point) {
            color = Qt::black; // Default color
            thickness = 1; // Default thickness
        }
    
    void draw(QPainter& painter) override;
    std::vector<QPoint> getVertices() override {
        std::vector<QPoint> vector{};
        vector.push_back(center1);
        vector.push_back(center2);
        vector.push_back(Point);
        return vector;
    }

    void moveVertex(int selectedVertexIndex, QPoint delta) override {
        if (selectedVertexIndex == 0) {
            center1.rx() += delta.x();
            center1.ry() += delta.y();
        } else if (selectedVertexIndex == 1) {
            center2.rx() += delta.x();
            center2.ry() += delta.y();
            Point.rx() += delta.x();
            Point.ry() += delta.y();
        } else if (selectedVertexIndex == 2) {
            Point.rx() += delta.x();
            Point.ry() += delta.y();
        } else {
            qDebug() << "Invalid vertex index: " << selectedVertexIndex;
        }
    }

    ~Pill() = default;

    QString save() const override {
        return QString("Pill,%1,%2,%3,%4,%5")
            .arg(center1.x())
            .arg(center1.y())
            .arg(center2.x())
            .arg(center2.y())
            .arg(Point.x())
            .arg(Point.y())
            .arg(color.name());
    }

    std::vector<QPoint> getEdgeMidpoints() const override {
        return std::vector<QPoint>();
    }

    
};

#endif // PILL_H