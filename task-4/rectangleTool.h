#ifndef RECTANGLETOOL_H
#define RECTANGLETOOL_H

#include "tool.h"
#include <QPoint>
#include <QPainter>
#include <QMouseEvent>
#include "rectangle.h"

class RectangleTool : public Tool {
private:
    QPoint firstCorner;
    QPoint secondCorner;
    bool isDrawing;
    Rectangle* previewRectangle;

public:
    RectangleTool() : isDrawing(false), previewRectangle(nullptr) {}

    ~RectangleTool() {
        if (previewRectangle) {
            delete previewRectangle;
        }
    }

    Figure* onMousePress(QMouseEvent* event) override;
    bool onMouseMove(QMouseEvent* event) override;
    void draw(QPainter& painter) override;
    void onMouseRelease(QMouseEvent* event) override {}

    std::string name() const override {
        return "Rectangle";
    }
};

#endif // RECTANGLETOOL_H
