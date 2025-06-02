#ifndef CAMERA_H
#define CAMERA_H

#include "qvectornd.h"
#include "qmatrix4x4.h"
#include <QtMath>

class Camera {
    float fov; // Field of view in radians
    QMatrix4x4 translation; // Translation matrix
    QMatrix4x4 rotation; // Rotation matrix
    QVector3D position; 
    QVector3D rotationAngles; 
public:
    Camera(float fov) : fov(fov), position(0,0,0), rotationAngles(0,0,0) {
        translation.setToIdentity(); 
        rotation.setToIdentity();
    }

    void Translate(const QVector3D& translationVector) {
        position += translationVector; 
        translation.translate(translationVector);
    }

    void Rotate(float yaw, float pitch) {
        rotationAngles.setX(rotationAngles.x() + yaw);   // Yaw (Y-axis rotation)
        rotationAngles.setY(rotationAngles.y() + pitch); // Pitch (X-axis rotation)
        
        // Rebuild rotation matrix from euler angles
        rotation.setToIdentity();
        rotation.rotate(rotationAngles.x(), QVector3D(0, 1, 0)); // Yaw around Y
        rotation.rotate(rotationAngles.y(), QVector3D(1, 0, 0)); // Pitch around X
    }

    void RotateYaw(float degrees) {
        Rotate(degrees, 0);
    }

    void RotatePitch(float degrees) {
        Rotate(0, degrees);
    }

    const QVector3D& getPosition() const {
        return position; 
    }

    const QVector3D& getEulerAngles() const {
        return rotationAngles;
    }

    const QMatrix4x4& getRotationMatrix() const {
        return rotation;
    }

    float getFov() const {
        return fov; 
    }
};

#endif // CAMERA_H