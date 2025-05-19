#ifndef CLIPPINGMANAGER_H
#define CLIPPINGMANAGER_H

#include "figure.h"
#include "qcolor.h"
#include "qpainter.h"
#include "qpoint.h"
#include <vector>
#include <map>

class ClippingManager {
private:
    // Map to store source polygons and their clipped edges
    struct ClippingPair {
        Figure* sourcePolygon;
        Figure* clipPolygon;
        std::vector<std::pair<QPoint, QPoint>> clippedEdges;
    };

    std::vector<ClippingPair> clippingPairs;
    QColor highlightColor;

    // Helper function to check if a point is inside a polygon
    bool isPointInside(const QPoint& point, const std::vector<QPoint>& polyVertices);

    // Cyrus-Beck line clipping against convex polygon
    bool clipLine(const QPoint& p1, const QPoint& p2,
                 const std::vector<QPoint>& clipVertices,
                 QPoint& outP1, QPoint& outP2);

public:
    ClippingManager();
    ~ClippingManager() = default;

    // Add a new clipping pair
    void addClippingPair(Figure* source, Figure* clip);

    // Remove a clipping pair
    void removeClippingPair(Figure* source, Figure* clip);

    // Check if a figure is already part of a clipping pair
    bool isInClippingPair(Figure* figure) const;

    void setHighlightColor(const QColor& color);

    // Update all clipping calculations
    void updateAllClipping();

    // Update specific clipping pair
    void updateClipping(Figure* source, Figure* clip);

    // Draw all clipped segments
    void drawClippedSegments(QPainter& painter);

    // Reset the clipping state
    void reset();

    void deletePairs(Figure* figure);
};

#endif // CLIPPINGMANAGER_H
