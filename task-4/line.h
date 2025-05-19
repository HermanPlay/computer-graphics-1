#ifndef LINE_H
#define LINE_H

#include "figure.h"
#include <QPoint>
#include "qdebug.h"
#include "qlogging.h"
#include "qpainter.h"
#include "qpoint.h"

class Line : public Figure {
private:
    QPoint start;
    QPoint end;
    float cov(const float d, const float r);
    float coverage(const float thickness, const float distance, const float r);
public:
    Line(QPoint start, QPoint end) : start(start), end(end) {}
    bool drawTrash = true;

    void draw(QPainter& painter) override;

    QPoint& getStart() { return start; } // Getter for start point
    QPoint& getEnd() { return end; }     // Getter for end point

    std::vector<QPoint> getVertices() override {
        std::vector<QPoint> vector = std::vector<QPoint>();
        vector.push_back(start);
        vector.push_back(end);
        return vector;
    }

    void moveVertex(int selectedVertexIndex, QPoint delta) override {
        if (selectedVertexIndex == 0) {
            start.rx() += delta.x();
            start.ry() += delta.y();
        } else if (selectedVertexIndex == 1) {
            end.rx() += delta.x();
            end.ry() += delta.y();
        } else {
            qDebug() << "Invalid vertex index: " << selectedVertexIndex;
        }
    }
    ~Line() = default;

    QString save() const override;

    void guptaSproullsLine(QPainter& painter);
    void bresenhamLine(QPainter& painter);


    std::vector<QPoint> getEdgeMidpoints() const override;
};

#endif // LINE_H
