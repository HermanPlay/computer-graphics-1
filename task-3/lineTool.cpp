#include "lineTool.h"
#include "line.h"
#include "qlogging.h"
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

Figure* LineTool::onMousePress(QMouseEvent *event)
{
    if (!isDrawing) {
        startPoint = event->pos();
        endPoint = startPoint; 
        isDrawing = true;
        qDebug() << "LineTool: Start point set at" << startPoint;
        return nullptr; 
    } else {
        endPoint = event->pos();
        isDrawing = false;
        qDebug() << "LineTool: End point set at" << endPoint;
        if (startPoint.x() > endPoint.x()) {
            std::swap(startPoint, endPoint);
        }

        return new Line(startPoint, endPoint);
    }
}

void LineTool::onMouseMove(QMouseEvent *event)
{
    if (isDrawing) {
        endPoint = event->pos();
    }
} 

void LineTool::draw(QPainter &painter)
{
    if (isDrawing) {
        painter.setPen(Qt::DashLine); 
        painter.drawLine(startPoint, endPoint);
    }
}
