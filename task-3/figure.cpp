#include "figure.h"
#include "circle.h"
#include "polygon.h"
#include "line.h"
#include "qdebug.h"
#include "qpoint.h"

Figure* Figure::fromString(const QString& data) {
    QStringList parts = data.split(",");
    if (parts.isEmpty()) {
        return nullptr;
    }

    if (parts[0] == "Circle" && parts.size() == 6) {
        QPoint center(parts[1].toInt(), parts[2].toInt());
        QPoint pointOnCircle(parts[3].toInt(), parts[4].toInt());
        QColor color(parts[5]); // Parse color
        Circle* circle = new Circle(center, pointOnCircle);
        circle->setColor(color);
        return circle;
    } else if (parts[0] == "Polygon" && parts.size() > 2) {
        Polygon* polygon = new Polygon();
        QColor color(parts[1]); // Parse color
        qDebug() << "Parsed color" << color;
        for (int i = 2; i < parts.size(); i++){
            QStringList coords = parts[i].split(';');
            if (coords.size() == 2) {
                polygon->addVertex(QPoint(coords[0].toInt(), coords[1].toInt()));
            } else {
                qDebug() << "Invalid polygon format"; 
                delete polygon;
                return nullptr;
            }
        }
        polygon->setColor(color);
        return polygon;
    } else if (parts[0] == "Line" && parts.size() == 6) {
        QPoint start(parts[1].toInt(), parts[2].toInt());
        QPoint end(parts[3].toInt(), parts[4].toInt());
        QColor color(parts[5]); // Parse color
        Line* line = new Line(start, end);
        line->setColor(color);
        return line;
    }

    return nullptr;
}