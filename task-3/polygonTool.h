#ifndef POLYGONTOOL_H
#define POLYGONTOOL_H

#include "qevent.h"
#include "qpainter.h"
#include "tool.h"
#include "polygon.h"

class PolygonTool : public Tool {
private:
    bool isDrawing; // True if the user is in the process of drawing a polygon
    Polygon* previewPolygon; // The polygon being drawn

public:
    PolygonTool();
    ~PolygonTool();

    Figure* onMousePress(QMouseEvent *event) override;
    void onMouseMove(QMouseEvent *event) override;
    std::string name() const override {
        return "Polygon tool";
    }
    void draw(QPainter &painter) override;

    void onMouseRelease(QMouseEvent *_) override {};
};

#endif // POLYGONTOOL_H