#ifndef POINTER_H
#define POINTER_H

#include "qevent.h"
#include "qpainter.h"
#include "tool.h"
#include "canvas.h"
#include <QPoint>

class Pointer : public Tool {
private:
    Canvas* canvas; // Reference to the canvas to access figures
    QPoint* selectedPoint = nullptr; // Pointer to the currently selected point (vertex or endpoint)
    Figure* selectedFigure = nullptr;
    int selectedVertexIndex = 0;
    int selectedEdgeIndex = -1; // Index of the selected edge midpoint
    QPoint lastMousePos;

public:
    explicit Pointer(Canvas* canvas);
    Figure* onMousePress(QMouseEvent* event) override;
    std::string name() const override {
        return "Pointer";
    }
    bool onMouseMove(QMouseEvent* event) override;
    void draw(QPainter& painter) override;

    void onMouseRelease(QMouseEvent *event) override;

    Figure* getSelectedFigure() {return selectedFigure;}; // Getter for selected figure
    void clearSelection() { selectedFigure = nullptr; }          // Clear the selection
};

#endif // POINTER_H
