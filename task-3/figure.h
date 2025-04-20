#ifndef FIGURE_H
#define FIGURE_H

#include "qdebug.h"
#include "qlogging.h"
#include <QPainter>
#include <QPoint>
#include <vector>
#include <QString>

class Figure {
public:
    virtual ~Figure() = default;
    virtual void draw(QPainter& painter) = 0;
    virtual std::vector<QPoint> getVertices() = 0;
    virtual QString save() const = 0; // Save method to serialize figure data, including color
    virtual void moveVertex(int selectedVertexIndex, QPoint delta) = 0;

    static Figure* fromString(const QString& data); // Parse a figure from a string, including color
    virtual std::vector<QPoint> getEdgeMidpoints() const = 0;

    void setAntiAliasing(bool enabled) { antiAliasing = enabled; }
    bool isAntiAliasingEnabled() const { return antiAliasing; }

    QColor getColor() const { return color; } // Getter for color
    void setColor(const QColor& newColor) { color = newColor; } // Setter for color
    void setThickness(int newThickness) { thickness = newThickness;
    qDebug() << "Updated thikness" << thickness; } // Setter for thickness
    int getThickness() const { return thickness; } // Getter for thickness

    const int radius = 3; // Radius for selection
    int thickness = 1;

protected:
    QColor color = Qt::black; // Default color

private:
    bool antiAliasing = false; // Tracks anti-aliasing state
};

#endif // FIGURE_H