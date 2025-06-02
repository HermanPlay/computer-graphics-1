#ifndef SCENE_H
#define SCENE_H
#include "camera.h"
#include "model.h"
#include "qcolor.h"
#include "qevent.h"
#include "qfile.h"
#include "qvectornd.h"

// Scene class stores objects, and camera
class Scene {
    QVector<Model> models; // List of models in the scene
    Camera *camera;

  public:
    Scene() { camera = new Camera(M_PI / 3); };

    ~Scene() { delete camera; }

    void addModel(const Model &model) { models.append(model); }

    void readFromObjFile(const QString &filePath) {
        qDebug() << "Reading OBJ file from path:" << filePath;
        // Open the file
        QFile file = QFile(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Could not open file:" << filePath;
            return;
        }
        QTextStream in(&file);

        QVector<QVector3D> points;
        QVector<QVector3D> trianglePoints;

        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("v ")) { // Vertex
                QStringList parts = line.split(' ');
                if (parts.size() == 4) {
                    float x = parts[1].toFloat();
                    float y = parts[2].toFloat();
                    float z = parts[3].toFloat();
                    points.append(QVector3D(x, y, z));
                }
            } else if (line.startsWith("f ")) { // Face
                QStringList parts = line.split(' ');
                // I know that OBJ files can have faces with different numbers
                // of vertices, but for simplicity, I assume triangular faces.
                if (parts.size() == 4) {
                    for (int i = 1; i < 4; ++i) {
                        QStringList secondParts = parts[i].split('/');
                        if (secondParts.size() >= 1) {
                            int index = secondParts[0].toInt() -
                                        1; // OBJ indices are 1-based
                            trianglePoints.append(points[index]);
                        }
                    }
                }
            }
        }

        file.close();
        if (!trianglePoints.isEmpty()) {
            Model model(trianglePoints);
            addModel(model);
            qDebug() << "Loaded model with" << trianglePoints.size()
                     << "triangle points from file:" << filePath;
        } else {
            qDebug() << "No valid triangle points found in the file:"
                     << filePath;
        }
    }

    // Get all models in the scene
    const QVector<Model> &getModels() const { return models; }

    Camera *getCamera() { return camera; }

    QVector<QColor> getColors() const {
        // Count number of triangles in all models
        int triangleCount = 0;
        for (const Model &model : models) {
            triangleCount += model.getTrianglePoints().size() /
                             3; // Each triangle has 3 points
        }
        // Generate distinct colors based on the number of triangles
        QVector<QColor> colors;
        for (int i = 0; i < triangleCount; ++i) {
            int r = (i * 123 + 45) % 256; // Simple color generation logic
            int g = (i * 234 + 67) % 256;
            int b = (i * 345 + 89) % 256;
            colors.append(QColor(r, g, b));
        }
        return colors;
    }
};

#endif // SCENE_H
