#include "polygonTool.h"
#include "qdebug.h"
#include <QMouseEvent>
#include <QDebug>
#include <cmath>

PolygonTool::PolygonTool() : isDrawing(false), previewPolygon(new Polygon()) {}

PolygonTool::~PolygonTool() {
    delete previewPolygon;
}

Figure* PolygonTool::onMousePress(QMouseEvent *event) {

    if (!isDrawing) {
        
        isDrawing = true;
        previewPolygon->addVertex(event->pos()); 
        previewPolygon->addVertex(event->pos()); 
        previewPolygon->setPreview(true); 
        qDebug() << "PolygonTool: Started drawing, added first vertex at" << event->pos();
        return nullptr;
    }

    if (previewPolygon->getVertexCount() >= 3) {
        QPoint firstVertex = previewPolygon->getFirstVertex();
        int dx = event->pos().x() - firstVertex.x();
        int dy = event->pos().y() - firstVertex.y();
        if (std::sqrt(dx * dx + dy * dy) <= previewPolygon->radius) {
            isDrawing = false;
            previewPolygon->setPreview(false); 
            Polygon* finalizedPolygon = previewPolygon;
            finalizedPolygon->popLast();
            previewPolygon = new Polygon(); 
            qDebug() << "PolygonTool: Finalized polygon";
            return finalizedPolygon;
        }
    }

    previewPolygon->addVertex(event->pos());
    qDebug() << "PolygonTool: Added vertex at" << event->pos();
    return nullptr; 
}

void PolygonTool::onMouseMove(QMouseEvent *event) {
    if (isDrawing) {
        if (!previewPolygon->isEmpty()) {
            previewPolygon->updateLastVertex(event->pos());
        }
    }
}

void PolygonTool::draw(QPainter &painter) {
    if (isDrawing && previewPolygon) {
        previewPolygon->draw(painter);
    }
}

