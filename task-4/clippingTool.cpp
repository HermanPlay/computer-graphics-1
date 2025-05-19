#include "clippingTool.h"
#include "figure.h"
#include "polygon.h"
#include "qlogging.h"
#include "rectangle.h"
#include <QDebug>

ClippingTool::ClippingTool(Canvas* canvas, ClippingManager* manager)
    : canvas(canvas),
      clippingManager(manager),
      state(ClippingState::SELECTING_SOURCE),
      currentSourceFigure(nullptr) {
}

Figure* ClippingTool::onMousePress(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return nullptr;

    QPoint pos = event->pos();
    Figure* clickedFigure = canvas->getFigureAt(pos);

    if (!clickedFigure) return nullptr;

    // Check if the figure is a polygon or rectangle
    bool isValidFigure = dynamic_cast<Polygon*>(clickedFigure) ||
                         dynamic_cast<Rectangle*>(clickedFigure);

    if (!isValidFigure) {
        qDebug() << "Clipping: Selected figure is not a polygon or rectangle";
        return nullptr;
    }

    if (state == ClippingState::SELECTING_SOURCE) {
        currentSourceFigure = clickedFigure;
        state = ClippingState::SELECTING_CLIPPER;
        qDebug() << "Clipping: Selected source polygon";
    }
    else if (state == ClippingState::SELECTING_CLIPPER && currentSourceFigure) {
        if (clickedFigure == currentSourceFigure) {
            qDebug() << "Clipping: Can't use the same figure as source and clipper";
            return nullptr;
        }
        if (clickedFigure->isConcave()) {
            qDebug() << "Clipping is allowed against convex polygons!";
            return nullptr;
        }

        clippingManager->addClippingPair(currentSourceFigure, clickedFigure);
        state = ClippingState::CLIPPING_ACTIVE;
        qDebug() << "Clipping: Added new clipping pair";

        // Reset to start selecting a new source
        state = ClippingState::SELECTING_SOURCE;
        currentSourceFigure = nullptr;
    }

    return nullptr;
}

bool ClippingTool::onMouseMove(QMouseEvent* event) {
    // No action needed for mouse movement
    return false;
}

void ClippingTool::onMouseRelease(QMouseEvent* event) {
    // No action needed for mouse release
}

void ClippingTool::draw(QPainter& painter) {
    // Draw all clipped segments
    clippingManager->drawClippedSegments(painter);

    // Optionally, highlight the current source figure if one is selected
    if (currentSourceFigure && state == ClippingState::SELECTING_CLIPPER) {
        painter.save();
        QPen highlightPen(Qt::blue, 2, Qt::DashLine);
        painter.setPen(highlightPen);

        painter.restore();
    }
}
