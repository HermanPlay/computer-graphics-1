#include "clippingManager.h"
#include "figure.h"
#include "line.h"
#include "qpoint.h"
#include <QDebug>
#include <cmath>

ClippingManager::ClippingManager() : highlightColor(Qt::red) {
}

void ClippingManager::addClippingPair(Figure* source, Figure* clip) {
    // Check if this pair already exists
    for (auto& pair : clippingPairs) {
        if (pair.sourcePolygon == source && pair.clipPolygon == clip) {
            qDebug() << "Clipping pair already exists";
            return;
        }
    }

    ClippingPair newPair;
    newPair.sourcePolygon = source;
    newPair.clipPolygon = clip;
    clippingPairs.push_back(newPair);

    // Perform the initial clipping
    updateClipping(source, clip);
}

void ClippingManager::removeClippingPair(Figure* source, Figure* clip) {
    for (auto it = clippingPairs.begin(); it != clippingPairs.end(); ++it) {
        if (it->sourcePolygon == source && it->clipPolygon == clip) {
            clippingPairs.erase(it);
            return;
        }
    }
}

bool ClippingManager::isInClippingPair(Figure* figure) const {
    for (const auto& pair : clippingPairs) {
        if (pair.sourcePolygon == figure || pair.clipPolygon == figure) {
            return true;
        }
    }
    return false;
}

void ClippingManager::setHighlightColor(const QColor& color) {
    highlightColor = color;
}

void ClippingManager::reset() {
    clippingPairs.clear();
}

void ClippingManager::updateAllClipping() {
    for (auto& pair : clippingPairs) {
        updateClipping(pair.sourcePolygon, pair.clipPolygon);
    }
}

void ClippingManager::updateClipping(Figure* source, Figure* clip) {
    // Find the pair
    for (auto& pair : clippingPairs) {
        if (pair.sourcePolygon == source && pair.clipPolygon == clip) {
            pair.clippedEdges.clear();

            // Get vertices from both polygons
            std::vector<QPoint> sourceVertices = source->getVertices();
            std::vector<QPoint> clipVertices = clip->getVertices();

            // For each edge in the source polygon
            for (size_t i = 0; i < sourceVertices.size(); ++i) {
                QPoint p1 = sourceVertices[i];
                QPoint p2 = sourceVertices[(i + 1) % sourceVertices.size()];

                QPoint clippedP1, clippedP2;
                if (clipLine(p1, p2, clipVertices, clippedP1, clippedP2)) {
                    pair.clippedEdges.push_back(std::make_pair(clippedP1, clippedP2));
                }
            }

            break;
        }
    }
}

// Function to check if polygon vertices are in clockwise order
bool isClockwise(const std::vector<QPoint>& vertices) {
    if (vertices.size() < 3) {
        return true;
    }

    double signedArea = 0.0;
    for (size_t i = 0; i < vertices.size(); ++i) {
        const QPoint& current = vertices[i];
        const QPoint& next = vertices[(i + 1) % vertices.size()];

        signedArea += (current.x() * next.y() - next.x() * current.y());
    }

    return signedArea < 0;
}

bool ClippingManager::clipLine(const QPoint& p1, const QPoint& p2, const std::vector<QPoint>& clipVertices, QPoint& outP1, QPoint& outP2) {
    if (p1 == p2) {
        outP1 = outP2 = p1;
        return true;
    }

    QPoint directionVec = QPoint(p2.x() - p1.x(), p2.y() - p1.y());

    float tE = 0.0f;
    float tL = 1.0f;
    bool isClock = isClockwise(clipVertices);

    // For each edge of the clipping polygon
    for (size_t i = 0; i < clipVertices.size(); ++i) {
        QPoint PEi = clipVertices[i];
        QPoint edgeEnd = clipVertices[(i + 1) % clipVertices.size()];

        QPoint edge = QPoint(edgeEnd.x() - PEi.x(), edgeEnd.y() - PEi.y());

        QPointF Ni = QPointF(edge.y(), -edge.x());
        if (isClock)
            Ni *= -1;

        float length = std::sqrt(Ni.x() * Ni.x() + Ni.y() * Ni.y());
        if (length > 0) {
            Ni.setX(Ni.x() / length);
            Ni.setY(Ni.y() / length);
        }

        // Calculate P1-PEi vector
        QPointF W = QPointF(p1.x() - PEi.x(), p1.y() - PEi.y());

        // Calculate Ni·D (dot product)
        float NiD = Ni.x() * directionVec.x() + Ni.y() * directionVec.y();

        // Calculate Ni·(P1-PEi) (dot product)
        float NiW = Ni.x() * W.x() + Ni.y() * W.y();

        float t = -NiW / NiD;

        if (NiD < 0) {
            tE = std::max(tE, t);
        } else if (NiD == 0) {
            if (NiW > 0) {
                return false;
            }
        } else {
            tL = std::min(tL, t);
        }
    }

    if (tE > tL) {
        return false;
    }

    outP1.setX(p1.x() + tE * directionVec.x());
    outP1.setY(p1.y() + tE * directionVec.y());

    outP2.setX(p1.x() + tL * directionVec.x());
    outP2.setY(p1.y() + tL * directionVec.y());

    return true;
}
void ClippingManager::drawClippedSegments(QPainter& painter) {
    painter.save();

    for (const auto& pair : clippingPairs) {
        for (const auto& edge : pair.clippedEdges) {
            Line l = Line(edge.first, edge.second);
            l.drawTrash = false;
            l.setBorderColor(highlightColor);
            l.setThickness(3);
            l.draw(painter);
        }
    }

    painter.restore();
}

void ClippingManager::deletePairs(Figure* figure) {
    auto newEnd = std::remove_if(clippingPairs.begin(), clippingPairs.end(),
        [figure](const ClippingPair& pair) {
            return pair.sourcePolygon == figure || pair.clipPolygon == figure;
        });

    if (newEnd != clippingPairs.end()) {
        clippingPairs.erase(newEnd, clippingPairs.end());
    }
}
