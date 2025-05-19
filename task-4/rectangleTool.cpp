#include "rectangleTool.h"
#include "rectangle.h"
#include <QDebug>

Figure* RectangleTool::onMousePress(QMouseEvent* event)
{
    if (!isDrawing) {
        firstCorner = event->pos();
        secondCorner = firstCorner; // Initialize with same position
        isDrawing = true;
        previewRectangle = new Rectangle(firstCorner, secondCorner);
        previewRectangle->isPreview = true;
        qDebug() << "RectangleTool: First corner set at" << firstCorner;
        return nullptr;
    } else {
        secondCorner = event->pos();
        isDrawing = false;
        qDebug() << "RectangleTool: Second corner set at" << secondCorner;

        Rectangle* finalRectangle = previewRectangle;
        finalRectangle->isPreview = false;
        previewRectangle = nullptr; // Prevent deletion in the destructor
        return finalRectangle;
    }
}

bool RectangleTool::onMouseMove(QMouseEvent* event)
{
    if (isDrawing) {
        secondCorner = event->pos();
        if (previewRectangle) {
            delete previewRectangle;
        }
        previewRectangle = new Rectangle(firstCorner, secondCorner);
        return true;
    }
    return false;
}

void RectangleTool::draw(QPainter& painter)
{
    if (isDrawing && previewRectangle) {
        painter.setPen(Qt::DashLine);
        previewRectangle->draw(painter);
    }
}
