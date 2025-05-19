#ifndef CIRCLETOOL_H
#define CIRCLETOOL_H

#include "qevent.h"
#include "tool.h"
#include "circle.h"
#include <QPoint>

class CircleTool : public Tool {
private:
    bool isDrawing = false; // True if the user is in the process of drawing a circle
    QPoint center;          // Center of the circle
    Circle* previewCircle = nullptr; // Circle being drawn

public:
    CircleTool() = default;
    ~CircleTool() {
        if (previewCircle) {
            delete previewCircle;
        }
    }

    Figure* onMousePress(QMouseEvent* event) override {
        if (!isDrawing) {
            // First click: Set the center of the circle
            center = event->pos();
            previewCircle = new Circle(center, center); // Initialize with center as both points
            isDrawing = true;
            return nullptr;
        } else {
            // Second click: Finalize the circle
            QPoint pointOnCircle(center.x() + abs(event->pos().x() - center.x()), center.y());
            previewCircle->moveVertex(1, pointOnCircle - previewCircle->getVertices()[1]); // Adjust the radius
            isDrawing = false;
            Circle* finalizedCircle = previewCircle;
            previewCircle = nullptr;
            return finalizedCircle;
        }
    }

    bool onMouseMove(QMouseEvent* event) override {
        if (isDrawing && previewCircle) {
            // Update the point on the circle, pinned to the right of the center
            QPoint pointOnCircle(center.x() + abs(event->pos().x() - center.x()), center.y());
            previewCircle->moveVertex(1, pointOnCircle - previewCircle->getVertices()[1]);
            return true;
        }
        return false;
    }

    void draw(QPainter& painter) override {
        if (previewCircle) {
            previewCircle->draw(painter); // Draw the preview circle
        }
    }

    std::string name() const override {
        return "Circle Tool";
    }

    void onMouseRelease(QMouseEvent *_) override {};
};

#endif // CIRCLETOOL_H
