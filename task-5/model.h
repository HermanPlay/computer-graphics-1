#ifndef MODEL_H
#define MODEL_H
#include "qcontainerfwd.h"
#include "qvectornd.h"
#include <QVector>

class Model {
  public:
    Model(QVector<QVector3D> points) : trianglePoints(points) {}

    // Virtual destructor to ensure proper cleanup of derived classes
    ~Model() = default;

    QVector<QVector3D> getTrianglePoints() const { return trianglePoints; }

    int getTriangleCount() const { return trianglePoints.size() / 3; }

  protected:
    // Vector of 3D points representing the vertices of triangles. Each 3 points
    // form a triangle.
    QVector<QVector3D> trianglePoints;
};

#endif // MODEL_H