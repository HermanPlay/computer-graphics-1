#include "pillTool.h"
#include "pill.h"
#include <QMouseEvent>

PillTool::PillTool() : step(0), previewPill(nullptr) {}

PillTool::~PillTool() {
    if (previewPill) {
        delete previewPill;
    }
}

Figure* PillTool::onMousePress(QMouseEvent* event) {
    QPoint clickPosition = event->pos();

    if (step == 0) {
        center1 = clickPosition;
        step = 1;
    } else if (step == 1) {
        center2 = clickPosition;
        step = 2;
    } else if (step == 2) {
        QPoint radiusPoint = clickPosition;
        previewPill = new Pill(center1, center2, radiusPoint);
        step = 0; 
        Pill* finalizedPill = previewPill;
        previewPill = nullptr;
        return finalizedPill;
    }

    return nullptr;
}

void PillTool::onMouseMove(QMouseEvent* event) {
    if (step == 2 && previewPill) {
        QPoint radiusPoint = event->pos();
        previewPill->moveVertex(2, radiusPoint - previewPill->getVertices()[2]);
    }
}

void PillTool::draw(QPainter& painter) {
    if (previewPill) {
        previewPill->draw(painter); 
    }
}

std::string PillTool::name() const {
    return "Pill Tool";
}