#include "pointer.h"
#include "qevent.h"
#include "qpoint.h"
#include "polygon.h"

Pointer::Pointer(Canvas* canvas) : canvas(canvas), selectedPoint(nullptr) {}

Figure* Pointer::onMousePress(QMouseEvent *event) 
{
    if (event->button() != Qt::LeftButton) return nullptr;

    QPoint pos = event->pos();
    selectedFigure = nullptr; // Reset selection

    for (Figure* fig : canvas->getFigures()) {
        int index = 0;
        for (const QPoint& vertex : fig->getVertices()) {
            if (QLineF(pos, vertex).length() < fig->radius) {
                selectedFigure = fig;
                selectedVertexIndex = index;
                lastMousePos = pos;

                // Emit signal to display figure information
                emit canvas->figureSelected(fig->getColor(), fig->getThickness());
                canvas->selectFigure(fig);
                return nullptr;
            }
            ++index;
        }
        
        // Check if the click is near an edge midpoint
        const auto& midpoints = fig->getEdgeMidpoints();
        for (size_t i = 0; i < midpoints.size(); ++i) {
            if (QLineF(pos, midpoints[i]).length() < fig->radius) {
                selectedFigure = fig;
                selectedVertexIndex = -1; // Indicate midpoint selection
                selectedEdgeIndex = i;   // Store the edge index
                lastMousePos = pos;

                emit canvas->figureDeselected();
                canvas->selectFigure(nullptr);
                return nullptr;
            }
        }

        if (Polygon* polygon = dynamic_cast<Polygon*>(fig)) {
            if (QLineF(pos, polygon->getPolygonMidpoint()).length() < polygon->radius) {
                selectedFigure = fig;
                selectedVertexIndex = -1;
                selectedEdgeIndex = -1; // Indicate polygon midpoint selection
                lastMousePos = pos;

                emit canvas->figureDeselected();
                canvas->selectFigure(nullptr);
                return nullptr;
            }
        }
    }

    // If no figure is selected, emit signal to clear information
    emit canvas->figureDeselected();
    return nullptr;
}

void Pointer::onMouseMove(QMouseEvent* event) { 
    if (!selectedFigure) return;

    QPoint delta = event->pos() - lastMousePos;

    if (selectedVertexIndex >= 0) {
        // Move a single vertex
        selectedFigure->moveVertex(selectedVertexIndex, delta);
    } else if (selectedEdgeIndex >= 0) {
        // Move the vertices of the selected edge
        const auto& vertices = selectedFigure->getVertices();
        int v1Index = selectedEdgeIndex;
        int v2Index = (selectedEdgeIndex + 1) % vertices.size();
        selectedFigure->moveVertex(v1Index, delta);
        selectedFigure->moveVertex(v2Index, delta);
    } else {
        // Move the entire polygon
        for (size_t i = 0; i < selectedFigure->getVertices().size(); ++i) {
            selectedFigure->moveVertex(i, delta);
        }
    }

    lastMousePos = event->pos();
    canvas->update(); // Trigger repaint
}

void Pointer::draw(QPainter& _) {
}

void Pointer::onMouseRelease(QMouseEvent *_) {
    selectedFigure = nullptr;
    selectedVertexIndex = -1;
    selectedEdgeIndex = -1;
    lastMousePos = QPoint();
}