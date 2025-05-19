#ifndef FIGURE_H
#define FIGURE_H

#include "qdebug.h"
#include "qlogging.h"
#include "qobject.h"
#include "qpixmap.h"
#include "qtextdocument.h"
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
    virtual bool isConcave() {
        return true;
    };

    void setAntiAliasing(bool enabled) { antiAliasing = enabled; }
    bool isAntiAliasingEnabled() const { return antiAliasing; }

    QColor getBorderColor() const { return borderColor; } // Getter for color
    void setBorderColor(const QColor& newColor) { borderColor = newColor; } // Setter for color
    QColor getFillColor() const { return fillColor; } // Getter for color
    void setFillColor(const QColor& newColor) { fillColor = newColor; } // Setter for color
    void setThickness(int newThickness) { thickness = newThickness; }
    int getThickness() const { return thickness; } // Getter for thickness
    void setTexturePath(const QString& newPath) {
        if (texture != nullptr)
            delete texture;
        texture = new QPixmap(newPath);
        texturePath = newPath;
        qDebug() << "Set new texture";
    };
    void removeTexture() {
        if (texture != nullptr)
            delete texture;
        texture = nullptr;
        texturePath = nullptr;
    }

    const int radius = 3; // Radius for selection
    int thickness = 1;

protected:
    QColor borderColor = Qt::black; // Default color
    QColor fillColor = nullptr;
    QPixmap* texture = nullptr;
    QString texturePath = nullptr;

private:
    bool antiAliasing = false; // Tracks anti-aliasing state
};

void FillFigure(QPainter& painter, std::vector<QPoint> vertices, QColor fillColor);
void FillFigureWithTexture(QPainter &painter, std::vector<QPoint> vertices, const QPixmap& texture);

#endif // FIGURE_H
