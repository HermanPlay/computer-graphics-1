#ifndef CAMERA_H
#define CAMERA_H

#include "qmatrix4x4.h"
#include "qvectornd.h"
#include <QtMath>

class Camera {
    float fov;              // Field of view in radians
    QMatrix4x4 translation; // Translation matrix
    QMatrix4x4 rotation;    // Rotation matrix
    QVector3D position;
    QVector3D rotationAngles;

  public:
    Camera(float fov) : fov(fov), position(0, 0, 0), rotationAngles(0, 0, 0) {
        translation.setToIdentity();
        rotation.setToIdentity();
    }

    void Translate(const QVector3D &translationVector) {
        position += translationVector;

        // Translation matrix format:
        // [1  0  0  tx]
        // [0  1  0  ty]
        // [0  0  1  tz]
        // [0  0  0  1 ]
        translation(0, 3) += translationVector.x();
        translation(1, 3) += translationVector.y();
        translation(2, 3) += translationVector.z();
    }

    void Rotate(float yaw, float pitch) {
        rotationAngles.setX(rotationAngles.x() + yaw); // Yaw (Y-axis rotation)
        rotationAngles.setY(rotationAngles.y() +
                            pitch); // Pitch (X-axis rotation)

        // Convert degrees to radians
        float yawRad = qDegreesToRadians(rotationAngles.x());
        float pitchRad = qDegreesToRadians(rotationAngles.y());

        float cosYaw = cos(yawRad);
        float sinYaw = sin(yawRad);

        float cosPitch = cos(pitchRad);
        float sinPitch = sin(pitchRad);

        rotation.setToIdentity();
        rotation(0, 0) = cosYaw;
        rotation(0, 1) = sinYaw * sinPitch;
        rotation(0, 2) = sinYaw * cosPitch;
        rotation(0, 3) = 0.0f;

        rotation(1, 0) = 0.0f;
        rotation(1, 1) = cosPitch;
        rotation(1, 2) = -sinPitch;
        rotation(1, 3) = 0.0f;

        rotation(2, 0) = -sinYaw;
        rotation(2, 1) = cosYaw * sinPitch;
        rotation(2, 2) = cosYaw * cosPitch;
        rotation(2, 3) = 0.0f;

        rotation(3, 0) = 0.0f;
        rotation(3, 1) = 0.0f;
        rotation(3, 2) = 0.0f;
        rotation(3, 3) = 1.0f;
    }

    void RotateYaw(float degrees) { Rotate(degrees, 0); }

    void RotatePitch(float degrees) { Rotate(0, degrees); }

    const QVector3D &getPosition() const { return position; }

    const QVector3D &getEulerAngles() const { return rotationAngles; }

    const QMatrix4x4 &getRotationMatrix() const { return rotation; }

    float getFov() const { return fov; }
};

#endif // CAMERA_H