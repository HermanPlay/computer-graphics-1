#ifndef CUBE_H
#define CUBE_H

#include "qmatrix4x4.h"
#include "qvectornd.h"
#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QPainter>
#include <QMatrix4x4> // Add for 3D transformations

class Cube : public QWidget
{
    Q_OBJECT

public:
    explicit Cube(QWidget *parent = nullptr);
    void rotateX(int angle);
    void rotateY(int angle);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<QVector3D> baseVertices; // Store original vertices
    QVector<QVector3D> transformedVertices; // Store rotated vertices
    QMatrix4x4 rotationMatrix;
    QMatrix4x4 translationMatrix;
    float currentXAngle = 0.0f;
    float currentYAngle = 0.0f;
    float w = 600; 
    void applyRotation();
    void applyTranslations();
};

#endif // CUBE_H
