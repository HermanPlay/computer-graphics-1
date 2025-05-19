#ifndef LINETOOL_H
#define LINETOOL_H

#include "qevent.h"
#include "qpainter.h"
#include "qpoint.h"
#include "tool.h"
#include <QPoint>
#include <QPainter>

class LineTool : public Tool {
private:
    QPoint startPoint;
    QPoint endPoint;
    bool isDrawing; // True if the user is in the process of drawing a line
public:
    LineTool() : isDrawing(false) {
        startPoint = QPoint();
        endPoint = QPoint();
    }

    Figure* onMousePress(QMouseEvent *event) override;
    bool onMouseMove(QMouseEvent *event) override;
    void draw(QPainter& painter) override;

    std::string name() const override {
        return "Line";
    }

    void onMouseRelease(QMouseEvent *_) override {};


};

#endif // LINETOOL_H
