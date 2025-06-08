#ifndef RASTERIZER_H
#define RASTERIZER_H

#include "model.h"
#include "qdebug.h"
#include "qevent.h"
#include "qlogging.h"
#include "qnamespace.h"
#include "qpainter.h"
#include "qpoint.h"
#include "qtmetamacros.h"
#include "qvectornd.h"
#include "qwidget.h"
#include "scene.h"
#include <QMouseEvent>
#include <QResizeEvent>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

class Rasterizer : public QWidget {
    Q_OBJECT
    QImage *target;                  // Target widget for rendering
    Scene *scene;                    // Scene to render
    QVector<QVector<float>> zBuffer; // Z-buffer for depth testing
    QVector3D perspectiveProjection; // Perspective projection parameters

  signals:
    void mousePositionChanged(int x, int y);

  public:
    // Constructor
    Rasterizer(QImage *target, Scene *scene) : target(target), scene(scene) {
        if (!target) {
            throw std::invalid_argument("Target widget cannot be null.");
        }
        if (!scene) {
            throw std::invalid_argument("Scene cannot be null.");
        }
        perspectiveProjection =
            QVector3D(0, 0, 300); // Initialize perspective projection
        setMouseTracking(true);   // Enable mouse tracking
        initializeZBuffer();
    }

    // Initialize Z-buffer with far values
    void initializeZBuffer() {
        if (!target || target->isNull())
            return;

        int width = target->width();
        int height = target->height();

        zBuffer.clear();
        zBuffer.resize(height);
        for (int y = 0; y < height; ++y) {
            zBuffer[y].resize(width);
            for (int x = 0; x < width; ++x) {
                zBuffer[y][x] = std::numeric_limits<float>::max(); // Far plane
            }
        }
    }

    // Destructor
    virtual ~Rasterizer() = default;

    // Method to render a model
    void renderModel(const Model &model, const QVector<QColor> colors) {
        if (target) {
            qDebug() << "Rendering model with"
                     << model.getTrianglePoints().size() << "points.";
            const QVector<QVector3D> &points = model.getTrianglePoints();

            // Define colors for different triangles - more distinct colors
            int triangleCount = 0;
            for (int i = 0; i < points.size(); i += 3) {
                if (i + 2 < points.size()) {
                    QColor triangleColor =
                        colors[triangleCount % colors.size()];

                    // qDebug() << "Triangle" << triangleCount << "color:" <<
                    // triangleColor.name(); qDebug() << "  Point 1:" <<
                    // points[i]; qDebug() << "  Point 2:" << points[i + 1];
                    // qDebug() << "  Point 3:" << points[i + 2];

                    // fillTriangleWithDepth(points[i], points[i + 1],
                    //                       points[i + 2], triangleColor.name());


                    // Convert to screen space first
                    QVector3D p1 = PointToScreen(points[i]);
                    QVector3D p2 = PointToScreen(points[i + 1]);
                    QVector3D p3 = PointToScreen(points[i + 2]);

                    fillTriangleScanLine(p1, p2, p3, triangleColor);


                    triangleCount++;
                }
            }
            qDebug() << "Total triangles rendered:" << triangleCount;
        } else {
            qWarning() << "Target widget is null, cannot render model.";
        }
    }

    float Dot(const QPointF &a, const QPointF &b) {
        return a.x() * b.x() + a.y() * b.y();
    }

    QPointF Perpendicular(const QPointF &a) { return QPointF(-a.y(), a.x()); }

    float SignedAreaOfTriangle(const QPointF &p, const QPointF &a,
                               const QPointF &b) {
        QPointF abPerpendicular = Perpendicular(b - a);
        QPointF ap = p - a;
        return Dot(ap, abPerpendicular) / 2;
    }

    bool isPointInTriangle(const QPointF &p, const QPointF &a, const QPointF &b,
                           const QPointF &c) {
        float sideAB = SignedAreaOfTriangle(p, a, b);
        float sideBC = SignedAreaOfTriangle(p, b, c);
        float sideCA = SignedAreaOfTriangle(p, c, a);
        return sideAB >= 0 && sideBC >= 0 && sideCA >= 0;
    }

    bool isPointInTriangle3D(const QPointF &p, const QPointF &a,
                             const QPointF &b, const QPointF &c, float &w1,
                             float &w2, float &w3) {
        float sideAB = SignedAreaOfTriangle(p, a, b);
        float sideBC = SignedAreaOfTriangle(p, b, c);
        float sideCA = SignedAreaOfTriangle(p, c, a);

        float totalArea = sideAB + sideBC + sideCA;
        float invAreaSum = 1.0f / totalArea;
        w1 = sideBC * invAreaSum;
        w2 = sideCA * invAreaSum;
        w3 = sideAB * invAreaSum;
        return sideAB >= 0 && sideBC >= 0 && sideCA >= 0 && totalArea > 0.0f;
    }

  private:
    // Triangle filling with Z-buffer depth testing
    void fillTriangleWithDepth(const QVector3D &v1, const QVector3D &v2,
                               const QVector3D &v3, QColor color) {
        if (!target || target->isNull()) {
            qWarning() << "Target image is invalid in fillTriangleWithDepth";
            return;
        }

        QVector3D p1 = PointToScreen(v1);
        QVector3D p2 = PointToScreen(v2);
        QVector3D p3 = PointToScreen(v3);

        // Get bounding box of the triangle
        int minX = std::max(0, (int)std::min({p1.x(), p2.x(), p3.x()}));
        int maxX = std::min(target->width() - 1,
                            (int)std::max({p1.x(), p2.x(), p3.x()}));
        int minY = std::max(0, (int)std::min({p1.y(), p2.y(), p3.y()}));
        int maxY = std::min(target->height() - 1,
                            (int)std::max({p1.y(), p2.y(), p3.y()}));

        // Check if bounding box is valid
        qDebug() << "Bounding box:"
                 << QRect(minX, minY, maxX - minX + 1, maxY - minY + 1);
        if (minX > maxX || minY > maxY) {
            qDebug() << "Invalid bounding box - triangle outside screen bounds";
            return;
        }

        // Check each pixel in the bounding box
        for (int x = minX; x <= maxX; ++x) {
            for (int y = minY; y <= maxY; ++y) {
                QPointF p(x, y);

                // Check if point is inside triangle
                float w1, w2, w3;
                if (isPointInTriangle3D(p, QPointF(p1.x(), p1.y()),
                                        QPointF(p2.x(), p2.y()),
                                        QPointF(p3.x(), p3.y()), w1, w2, w3)) {
                    // Interpolate Z value using barycentric coordinates
                    float z = w1 * p1.z() + w2 * p2.z() + w3 * p3.z();

                    // Skip pixels behind the camera (negative Z values)
                    if (z <= 0.0f)
                        continue;

                    // Z-buffer test: only draw if this pixel is closer (smaller
                    // Z = closer)
                    if (z >= zBuffer[y][x])
                        continue;

                    zBuffer[y][x] = z;
                    target->setPixelColor(x, y, color);
                }
            }
        }
    }

    void fillTriangle(QVector3D p1, QVector3D p2, QVector3D p3, QColor color) {
        // Check if target is valid
        if (!target || target->isNull()) {
            qWarning() << "Target image is invalid in fillTriangle";
            return;
        }

        // Get bounding box of the triangle
        int minX = std::max(0, (int)std::min({p1.x(), p2.x(), p3.x()}));
        int maxX = std::min(target->width() - 1,
                            (int)std::max({p1.x(), p2.x(), p3.x()}));
        int minY = std::max(0, (int)std::min({p1.y(), p2.y(), p3.y()}));
        int maxY = std::min(target->height() - 1,
                            (int)std::max({p1.y(), p2.y(), p3.y()}));

        qDebug() << "Bounding box:"
                 << QRect(minX, minY, maxX - minX + 1, maxY - minY + 1);

        // Check if bounding box is valid
        if (minX > maxX || minY > maxY) {
            qWarning()
                << "Invalid bounding box - triangle outside screen bounds";
            return;
        }

        // Check each pixel in the bounding box
        for (int x = minX; x <= maxX; ++x) {
            for (int y = minY; y <= maxY; ++y) {
                QPointF p(x, y);
                if (isPointInTriangle(p, QPointF(p1.x(), p1.y()),
                                      QPointF(p2.x(), p2.y()),
                                      QPointF(p3.x(), p3.y()))) {
                    target->setPixelColor(x, y, color);
                }
            }
        }
    }

    void fillTriangleScanLine(const QVector3D &v1, const QVector3D &v2, const QVector3D &v3, QColor fillColor) {
        if (!target || target->isNull()) {
            qWarning() << "Target image is invalid in fillTriangleScanLine";
            return;
        }

        qDebug() << "Triangle vertices:" << v1 << v2 << v3;

        // Build edge table for triangle (all 3 edges)
        struct Edge {
            double x_start, z_start;
            double dx_dy, dz_dy;
            int ymin, ymax;

            Edge(const QVector3D& p1, const QVector3D& p2) {
                // Ensure p1 is the lower point (smaller y)
                QVector3D lower = (p1.y() <= p2.y()) ? p1 : p2;
                QVector3D upper = (p1.y() <= p2.y()) ? p2 : p1;
                
                x_start = lower.x();
                z_start = lower.z();
                ymin = (int)ceil(lower.y());
                ymax = (int)floor(upper.y());
                
                double dy = upper.y() - lower.y();
                if (dy != 0) {
                    dx_dy = (upper.x() - lower.x()) / dy;
                    dz_dy = (upper.z() - lower.z()) / dy;
                } else {
                    dx_dy = 0;
                    dz_dy = 0;
                }
                
                qDebug() << "Edge from" << lower << "to" << upper 
                         << "ymin:" << ymin << "ymax:" << ymax
                         << "dx_dy:" << dx_dy << "dz_dy:" << dz_dy;
            }

            std::pair<double, double> getIntersection(int y) const {
                double x = x_start + dx_dy * (y - ymin + 0.5); // Add 0.5 for pixel center
                double z = z_start + dz_dy * (y - ymin + 0.5);
                return {x, z};
            }
        };

        // Create all three edges of the triangle
        std::vector<Edge> edges;
        
        // Only add non-horizontal edges
        if (abs(v1.y() - v2.y()) > 0.01) {
            edges.emplace_back(v1, v2);
        }
        if (abs(v2.y() - v3.y()) > 0.01) {
            edges.emplace_back(v2, v3);
        }
        if (abs(v3.y() - v1.y()) > 0.01) {
            edges.emplace_back(v3, v1);
        }

        if (edges.empty()) {
            qDebug() << "No valid edges, triangle is degenerate or horizontal";
            return;
        }

        // Find Y range
        int ymin = (int)ceil(std::min({v1.y(), v2.y(), v3.y()}));
        int ymax = (int)floor(std::max({v1.y(), v2.y(), v3.y()}));
        
        // qDebug() << "Y range:" << ymin << "to" << ymax << "Total edges:" << edges.size();

        // Process each scanline
        for (int y = ymin; y <= ymax; ++y) {
            if (y < 0 || y >= target->height()) continue;

            std::vector<std::pair<double, double>> intersections; // x, z pairs

            for (const auto& edge : edges) {
                if (y >= edge.ymin && y <= edge.ymax) {
                    auto intersection = edge.getIntersection(y);
                    intersections.push_back(intersection);
                    // qDebug() << "Y=" << y << "edge intersection at x=" << intersection.first << "z=" << intersection.second;
                }
            }

            // qDebug() << "Y=" << y << "total intersections:" << intersections.size();

            if (intersections.size() >= 2) {
                // Sort intersections by X
                std::sort(intersections.begin(), intersections.end());

                // Fill between pairs of intersections
                for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
                    int x1 = (int)round(intersections[i].first);
                    int x2 = (int)round(intersections[i + 1].first);
                    double z1 = intersections[i].second;
                    double z2 = intersections[i + 1].second;

                    if (x1 > x2) {
                        std::swap(x1, x2);
                        std::swap(z1, z2);
                    }

                    x1 = std::max(0, x1);
                    x2 = std::min(target->width() - 1, x2);

                    // qDebug() << "Filling scanline y=" << y << "from x=" << x1 << "to x=" << x2;

                    for (int x = x1; x <= x2; ++x) {
                        double t = (x2 == x1) ? 0.0 : (double)(x - x1) / (x2 - x1);
                        double z = z1 + t * (z2 - z1);

                        // Z-buffer test
                        if (z > 0.0 && z < zBuffer[y][x]) {
                            zBuffer[y][x] = z;
                            target->setPixelColor(x, y, fillColor);
                        }
                    }
                }
            }
        }
    }



    QVector3D PointToScreen(const QVector3D &point) {
        if (!target || target->isNull()) {
            qWarning() << "Target image is invalid in PointToScreen";
            return QVector3D(0, 0, 0);
        }

        const Camera *camera = scene->getCamera();
        float fov = camera->getFov();

        // Transform point to camera space
        QVector3D cameraPoint = point - camera->getPosition();

        // Apply inverse camera rotation
        QMatrix4x4 inverseRotation = camera->getRotationMatrix().transposed();
        QVector4D cameraPoint4D(cameraPoint, 1.0f);
        cameraPoint4D = inverseRotation * cameraPoint4D;
        cameraPoint = cameraPoint4D.toVector3D();

        QVector3D perspectivePoint = cameraPoint + perspectiveProjection;
        float z = perspectivePoint.z();

        if (z <= 0.1f) {
            return QVector3D(0, 0, 0);
        }
        // Perspective projection
        float aspectRatio = (float)target->width() / target->height();
        float f = 1.0f / tan(fov / 2.0f);

        // Project to normalized device coordinates (-1 to 1)
        float x_ndc = (cameraPoint.x() / z) / (aspectRatio * f);
        float y_ndc = (cameraPoint.y() / z) / f;

        // Convert to screen coordinates (0 to width/height)
        float x_screen = (x_ndc + 1.0f) * target->width() / 2.0f;
        float y_screen = (1.0f - y_ndc) * target->height() / 2.0f; // Flip Y

        return QVector3D(x_screen, y_screen, z);
    }

  public:
    void resizeEvent(QResizeEvent *event) override {
        QWidget::resizeEvent(event);
        if (target) {
            // Check for valid size
            if (event->size().width() <= 0 || event->size().height() <= 0) {
                qWarning() << "Invalid resize dimensions:" << event->size();
                return;
            }

            *target = QImage(event->size(), QImage::Format_ARGB32);
            qDebug() << "New image size:" << target->size();

            renderScene(); // Re-render the scene to the new image
        }
        update();
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        QWidget::mouseMoveEvent(event);
        emit mousePositionChanged(event->pos().x(), event->pos().y());
    }

    void renderScene() {
        if (!scene) {
            qWarning() << "Scene is null, cannot render.";
            return;
        }
        qDebug() << "Rendering scene with" << scene->getModels().size()
                 << "models.";
        target->fill(Qt::white); // Clear the target image

        initializeZBuffer();
        QVector<QColor> colors = scene->getColors();
        int i = 0;
        for (const Model &model : scene->getModels()) {
            // Offset colors by i for each model
            QVector<QColor> modelColors;
            int triangleCount = model.getTriangleCount();
            for (int t = 0; t < triangleCount; ++t) {
                modelColors.append(colors[(i + t) % colors.size()]);
            }
            renderModel(model, modelColors);
            i += triangleCount;
        }
        update();
    }

    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.drawImage(0, 0,
                          *target); // Draw the target image onto the widget
    }

    void TranslateCamera(const QVector3D &translationVector) {
        if (scene && scene->getCamera()) {
            qDebug() << "Translating camera by:" << translationVector;
            scene->getCamera()->Translate(translationVector);
            qDebug() << scene->getCamera()->getPosition();
            renderScene();
        } else {
            qWarning() << "Scene or camera is null, cannot translate camera.";
        }
    }

    void RotateCamera(float yaw, float pitch, float roll = 0) {
        if (scene && scene->getCamera()) {
            qDebug() << "Rotating camera by yaw:" << yaw << "pitch:" << pitch
                     << "roll:" << roll;
            scene->getCamera()->Rotate(yaw, pitch);
            qDebug() << "Camera angles:"
                     << scene->getCamera()->getEulerAngles();
            renderScene();
        } else {
            qWarning() << "Scene or camera is null, cannot rotate camera.";
        }
    }
};

#endif // RASTERIZER_H
