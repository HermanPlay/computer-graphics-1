#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "figure.h"
#include <QPoint>
#include <vector>

class Rectangle : public Figure {
private:
    QPoint corner1;
    QPoint corner2;
public:
    Rectangle(QPoint corner1, QPoint corner2) : corner1(corner1), corner2(corner2), isPreview(false) {}

    bool isPreview;
    void draw(QPainter& painter) override;
    std::vector<QPoint> getVertices() override;
    void moveVertex(int selectedVertexIndex, QPoint delta) override;
    void moveEdge(int edgeIndex, const QPoint& delta);
    ~Rectangle() = default;

    QString save() const override {
        QStringList parts;
        parts << "Rectangle"
              << QString::number(corner1.x())
              << QString::number(corner1.y())
              << QString::number(corner2.x())
              << QString::number(corner2.y())
              << borderColor.name();

        parts << (fillColor.isValid() ? fillColor.name() : "none");

        parts << (texturePath.isEmpty() ? "none" : texturePath);

        return parts.join(",");
    }
    bool isConcave() override {
        return false;
    }

    std::vector<QPoint> getEdgeMidpoints() const override;

    // Helper methods to get all four corners of the rectangle
    QPoint getTopLeft() const;
    QPoint getTopRight() const;
    QPoint getBottomLeft() const;
    QPoint getBottomRight() const;
};

#endif // RECTANGLE_H
