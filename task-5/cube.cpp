#include "cube.h"
#include "qnamespace.h"
#include "qvectornd.h"
#include <QPainter>
#include <QVector3D>
#include <QMatrix4x4> 
#include <cmath>

Cube::Cube(QWidget *parent)
    : QWidget(parent)
{
    // Define the 8 original vertices of the cube
    baseVertices << QVector3D(-50, -50, -50); 
    baseVertices << QVector3D( 50, -50, -50); 
    baseVertices << QVector3D( 50,  50, -50); 
    baseVertices << QVector3D(-50,  50, -50); 
    baseVertices << QVector3D(-50, -50,  50); 
    baseVertices << QVector3D( 50, -50,  50); 
    baseVertices << QVector3D( 50,  50,  50); 
    baseVertices << QVector3D(-50,  50,  50); 

    transformedVertices = baseVertices; 
    translationMatrix.setToIdentity();
    translationMatrix.translate(0, 0, 100); 
    rotationMatrix.setToIdentity(); 
    applyRotation(); 
}

void Cube::applyTranslations() 
{
    // This function now applies the translation to already rotated vertices
    for (int i = 0; i < transformedVertices.size(); ++i)
    {
        // Important: Apply translation to the ALREADY rotated vertices stored in transformedVertices
        transformedVertices[i] = translationMatrix * transformedVertices[i];
    }
    // update(); // Update is called in applyRotation after translations
}

void Cube::applyRotation()
{
    rotationMatrix.setToIdentity();
    rotationMatrix.rotate(currentYAngle, 0, 1, 0);
    rotationMatrix.rotate(currentXAngle, 1, 0, 0);

    transformedVertices.clear();
    for (const QVector3D &vertex : baseVertices)
    {
        transformedVertices.append(rotationMatrix * vertex);
    }

    for (int i = 0; i < transformedVertices.size(); ++i)
    {
        transformedVertices[i] = translationMatrix * transformedVertices[i];
    }
    update(); 
}

void Cube::rotateX(int angle)
{
    currentXAngle = angle; // Store the angle in degrees
    applyRotation();
}

void Cube::rotateY(int angle)
{
    currentYAngle = angle; // Store the angle in degrees
    applyRotation();
}

void Cube::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPointF offset(width() / 2.0, height() / 2.0);
    float theta = M_PI / 3; // Set a fixed angle for the projection

    auto project = [&](const QVector3D &pt3d) {
        float d = w/2*tan(theta/2);
        float x = pt3d.x()/pt3d.z() * d;
        float y = pt3d.y()/pt3d.z() * d;

        return QPointF(x + offset.x(), y + offset.y());
    };

    painter.setPen(Qt::black); 

    int edges[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0}, 
        {4, 5}, {5, 6}, {6, 7}, {7, 4}, 
        {0, 4}, {1, 5}, {2, 6}, {3, 7}  
    };

    for (const auto& edge : edges) {
        QPointF p1 = project(transformedVertices[edge[0]]);
        QPointF p2 = project(transformedVertices[edge[1]]);
        painter.drawLine(p1, p2);
    }
}
