#include "figure.h"
#include "circle.h"
#include "polygon.h"
#include "line.h"
#include "rectangle.h"
#include "qdebug.h"
#include "qpoint.h"

Figure* Figure::fromString(const QString& data) {
    QStringList parts = data.split(",");
    if (parts.isEmpty()) {
        return nullptr;
    }

    if (parts[0] == "Circle") {
        if (parts.size() != 6) return nullptr;

        QPoint center(parts[1].toInt(), parts[2].toInt());
        QPoint pointOnCircle(parts[3].toInt(), parts[4].toInt());
        QColor borderColor(parts[5]);

        Circle* circle = new Circle(center, pointOnCircle);
        circle->setBorderColor(borderColor);


        return circle;
    }
    else if (parts[0] == "Polygon") {
        if (parts.size() < 3) return nullptr;

        Polygon* polygon = new Polygon();
        QColor borderColor(parts[1]);
        polygon->setBorderColor(borderColor);

        // Set fill color if present
        if (parts.size() > 2 && parts[2] != "none") {
            polygon->setFillColor(QColor(parts[2]));
        }

        // Set texture if present
        if (parts.size() > 3 && parts[3] != "none") {
            polygon->setTexturePath(parts[3]);
        }

        // Parse vertices starting after texture info
        for (int i = 4; i < parts.size(); i++) {
            QStringList coords = parts[i].split(';');
            if (coords.size() == 2) {
                polygon->addVertex(QPoint(coords[0].toInt(), coords[1].toInt()));
            }
        }

        return polygon;
    }
    else if (parts[0] == "Rectangle") {
        if (parts.size() < 6) return nullptr;

        QPoint corner1(parts[1].toInt(), parts[2].toInt());
        QPoint corner2(parts[3].toInt(), parts[4].toInt());
        QColor borderColor(parts[5]);

        Rectangle* rectangle = new Rectangle(corner1, corner2);
        rectangle->setBorderColor(borderColor);

        // Set fill color if present
        if (parts.size() > 6 && parts[6] != "none") {
            rectangle->setFillColor(QColor(parts[6]));
        }

        // Set texture if present
        if (parts.size() > 7 && parts[7] != "none") {
            rectangle->setTexturePath(parts[7]);
        }

        return rectangle;
    }
    else if (parts[0] == "Line" && parts.size() >= 6) {
        QPoint start(parts[1].toInt(), parts[2].toInt());
        QPoint end(parts[3].toInt(), parts[4].toInt());
        QColor color(parts[5]);
        Line* line = new Line(start, end);
        line->setBorderColor(color);
        return line;
    }

    return nullptr;
}

void FillFigure(QPainter& painter, std::vector<QPoint> vertices, QColor fillColor) {
    if (vertices.size() < 3) {
        return; // Need at least 3 vertices for a polygon
    }

    std::vector<int> indices(vertices.size());
    for (int i = 0; i < vertices.size(); i++) {
        indices[i] = i;
    }

    std::sort(indices.begin(), indices.end(), [&vertices](int a, int b) {
        return vertices[a].y() < vertices[b].y();
    });

    struct Edge {
        double x;
        double invM;
        int ymax;      // Maximum y value for this edge

        Edge(const QPoint& start, const QPoint& end) {
            x = start.x();
            int dy = end.y() - start.y();
            int dx = end.x() - start.x();
            invM = (dy != 0) ? (double)dx / dy : 0;
            ymax = end.y();
        }
    };

    std::vector<Edge> aet;

    int k = 0;
    int i = indices[k];
    int y = vertices[indices[0]].y();  // ymin
    int ymax = vertices[indices[vertices.size()-1]].y();

    painter.setPen(fillColor);
    while (y < ymax) {
        // Process vertices at current scanline
        while (k < indices.size() && vertices[indices[k]].y() == y) {
            i = indices[k];
            int prev = (i-1 + vertices.size()) % vertices.size();
            int next = (i + 1) % vertices.size();

            if (vertices[prev].y() > vertices[i].y()) {
                aet.emplace_back(vertices[i], vertices[prev]);
            }
            if (vertices[next].y() > vertices[i].y()) {
                aet.emplace_back(vertices[i], vertices[next]);
            }

            ++k;
        }

        std::sort(aet.begin(), aet.end(), [](const Edge& a, const Edge& b) {
            return a.x < b.x;
        });

        for (size_t j = 0; j < aet.size() - 1; j += 2) {
            if (j + 1 < aet.size()) {
                int x1 = std::round(aet[j].x);
                int x2 = std::round(aet[j + 1].x);
                Line l = Line(QPoint(x1, y), QPoint(x2, y));
                l.drawTrash = false;
                l.setBorderColor(fillColor);
                l.draw(painter);
            }
        }

        ++y;

        aet.erase(std::remove_if(aet.begin(), aet.end(),
            [y](const Edge& e) { return e.ymax == y; }), aet.end());

        for (Edge& edge : aet) {
            edge.x += edge.invM;
        }
    }
}

void FillFigureWithTexture(QPainter& painter, std::vector<QPoint> vertices, const QPixmap& texture) {
    if (vertices.size() < 3 || texture.isNull()) {
        return; // Need at least 3 vertices and a valid texture
    }

    // Sort vertices by y-coordinate
    std::vector<int> indices(vertices.size());
    for (int i = 0; i < vertices.size(); i++) {
        indices[i] = i;
    }

    std::sort(indices.begin(), indices.end(), [&vertices](int a, int b) {
        return vertices[a].y() < vertices[b].y();
    });

    struct Edge {
        double x;
        double invM;
        int ymax;      // Maximum y value for this edge

        Edge(const QPoint& start, const QPoint& end) {
            x = start.x();
            int dy = end.y() - start.y();
            int dx = end.x() - start.x();
            invM = (dy != 0) ? (double)dx / dy : 0;
            ymax = end.y();
        }
    };

    std::vector<Edge> aet;

    // Calculate polygon bounds for texture mapping
    int minX = vertices[0].x();
    int maxX = vertices[0].x();
    int minY = vertices[0].y();
    int maxY = vertices[0].y();

    for (const auto& vertex : vertices) {
        minX = std::min(minX, vertex.x());
        minY = std::min(minY, vertex.y());
        maxX = std::max(maxX, vertex.x());
        maxY = std::max(maxY, vertex.y());
    }


    // Convert texture to image for pixel access
    QImage textureImage = texture.toImage();
    int textureWidth = texture.width();
    int textureHeight = texture.height();

    int k = 0;
    int i = indices[k];
    int y = vertices[indices[0]].y();  // ymin
    int ymax = vertices[indices[vertices.size()-1]].y();

    while (y < ymax) {
        // Process vertices at current scanline
        while (k < indices.size() && vertices[indices[k]].y() == y) {
            i = indices[k];
            int prev = (i-1 + vertices.size()) % vertices.size();
            int next = (i + 1) % vertices.size();

            if (vertices[prev].y() > vertices[i].y()) {
                aet.emplace_back(vertices[i], vertices[prev]);
            }
            if (vertices[next].y() > vertices[i].y()) {
                aet.emplace_back(vertices[i], vertices[next]);
            }

            ++k;
        }

        // Sort intersections by x coordinate
        std::sort(aet.begin(), aet.end(), [](const Edge& a, const Edge& b) {
            return a.x < b.x;
        });

        // Draw scan lines with textured pixels
        for (size_t j = 0; j < aet.size() - 1; j += 2) {
            if (j + 1 < aet.size()) {
                int x1 = std::round(aet[j].x);
                int x2 = std::round(aet[j + 1].x);

                for (int x = x1; x <= x2; ++x) {
                    // Map coordinates to texture using modulo for tiling/repeating
                    int relativeX = x - minX;
                    int relativeY = y - minY;

                    // Use modulo to create a repeating pattern
                    int texX = relativeX % textureWidth;
                    int texY = relativeY % textureHeight;

                    // Ensure positive values for modulo
                    if (texX < 0) texX += textureWidth;
                    if (texY < 0) texY += textureHeight;

                    // Get color from texture and draw the pixel
                    QColor pixelColor = textureImage.pixelColor(texX, texY);
                    painter.setPen(pixelColor);
                    painter.drawPoint(x, y);
                }
            }
        }

        ++y;

        aet.erase(std::remove_if(aet.begin(), aet.end(),
            [y](const Edge& e) { return e.ymax == y; }), aet.end());

        for (Edge& edge : aet) {
            edge.x += edge.invM;
        }
    }
}
