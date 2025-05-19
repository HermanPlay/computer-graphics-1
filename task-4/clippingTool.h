#ifndef CLIPPINGTOOL_H
#define CLIPPINGTOOL_H

#include "tool.h"
#include "clippingManager.h"
#include "canvas.h"
#include <QPoint>

class ClippingTool : public Tool {
private:
    Canvas* canvas;
    ClippingManager* clippingManager;
    enum class ClippingState {
        SELECTING_SOURCE,
        SELECTING_CLIPPER,
        CLIPPING_ACTIVE
    };

    ClippingState state;
    Figure* currentSourceFigure;  // Currently being selected source

public:
    ClippingTool(Canvas* canvas, ClippingManager* manager);
    ~ClippingTool() = default;

    Figure* onMousePress(QMouseEvent* event) override;
    bool onMouseMove(QMouseEvent* event) override;
    void onMouseRelease(QMouseEvent* event) override;
    void draw(QPainter& painter) override;

    std::string name() const override {
        return "Clipping Tool";
    }

    void updateAllClipping();

    void updateClippingForFigure(Figure* figure);

    void removeFigureFromClipping(Figure* figure);
};

#endif // CLIPPINGTOOL_H
