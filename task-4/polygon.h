#ifndef POLYGON_H
#define POLYGON_H

#include "figure.h"
#include "qcolor.h"
#include "qdebug.h"
#include "qnamespace.h"
#include <QPoint>
#include <vector>
#include <QStringList>

class Polygon : public Figure {
private:
    bool isPreview; // Flag to indicate if the polygon is in preview mode
    std::vector<QPoint> vertices; // Store the vertices of the polygon
public:

    Polygon() : isPreview(false) {
        borderColor =Qt::black;
        fillColor = nullptr;
    }

    void addVertex(const QPoint& vertex) {
        vertices.push_back(vertex); // Add the provided vertex
    }

    void updateLastVertex(const QPoint& vertex) {
        if (!vertices.empty()) {
            vertices.back() = vertex; // Update the last vertex
        }
    }

    bool isConcave() override;

    bool isEmpty() const {
        return vertices.empty();
    }

    QPoint getFirstVertex() const {
        if (!vertices.empty()) {
            return vertices.front(); // Return the first vertex
        }
        return QPoint(); // Return a default point if empty
    }

    int getVertexCount() const {
        return vertices.size(); // Return the number of vertices
    }

    void setPreview(bool preview) {
        isPreview = preview;
    }

    void draw(QPainter& painter) override;

    void popLast();

    std::vector<QPoint> getVertices() override {
        return vertices;
    }

    void moveVertex(int selectedVertexIndex, QPoint delta) override {
        vertices[selectedVertexIndex].rx() += delta.x();
        vertices[selectedVertexIndex].ry() += delta.y();
    }

    ~Polygon() {
        vertices.clear();
    }

    QString save() const override {
        QStringList parts;
        parts << "Polygon" << borderColor.name();

        parts << (fillColor.isValid() ? fillColor.name() : "none");

        parts << (texturePath.isEmpty() ? "none" : texturePath);

        for (const QPoint& vertex : vertices) {
            parts << QString("%1;%2").arg(vertex.x()).arg(vertex.y());
        }

        return parts.join(",");
    }

    std::vector<QPoint> getEdgeMidpoints() const override;

    QPoint getPolygonMidpoint() const;
    void guptaSproullsLine(QPainter& painter, QPoint start, QPoint end);
};

#endif // POLYGON_
