#pragma once
#include <QMatrix4x4>
#include <vector>
#include <cmath> // For std::sin, std::cos

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Default: 7 cameras at various positions
inline std::vector<QMatrix4x4> exampleCameraExtrinsics() {
    std::vector<QMatrix4x4> cams;

    // Camera 1: at (0,2,8), looking at origin
    QMatrix4x4 cam1;
    cam1.lookAt(QVector3D(0,2,8), QVector3D(0,0,0), QVector3D(0,1,0));
    cams.push_back(cam1.inverted()); // Since the final matrix output is the model from the "camera's" to the "world", neccessary invert

    // Camera 2: at (5,3,5), looking at origin
    QMatrix4x4 cam2;
    cam2.lookAt(QVector3D(5,3,5), QVector3D(0,0,0), QVector3D(0,1,0));
    cams.push_back(cam2.inverted());

    // Camera 3: at (-5,2,5), looking at origin
    QMatrix4x4 cam3;
    cam3.lookAt(QVector3D(-5,2,5), QVector3D(0,0,0), QVector3D(0,1,0));
    cams.push_back(cam3.inverted());

    // Camera 4: at (0,8,0), looking down at origin
    QMatrix4x4 cam4;
    cam4.lookAt(QVector3D(0,8,0), QVector3D(0,0,0), QVector3D(0,0,-1));
    cams.push_back(cam4.inverted());

    // Camera 5: at (0,2,-8), looking at origin
    QMatrix4x4 cam5;
    cam5.lookAt(QVector3D(0,2,-8), QVector3D(0,0,0), QVector3D(0,1,0));
    cams.push_back(cam5.inverted());

    // Camera 6: at (8,2,0), looking at origin
    QMatrix4x4 cam6;
    cam6.lookAt(QVector3D(8,2,0), QVector3D(0,0,0), QVector3D(0,1,0));
    cams.push_back(cam6.inverted());

    // Camera 7: at (0,0,8), looking at (0,2,0) (upwards)
    QMatrix4x4 cam7;
    cam7.lookAt(QVector3D(0,0,8), QVector3D(0,2,0), QVector3D(0,1,0));
    cams.push_back(cam7.inverted());

    return cams;
}

// --- New example camera setups ---

// 1. Circular array of cameras around the origin, all looking in
inline std::vector<QMatrix4x4> exampleCameraExtrinsics_Circle(int numCams = 8, float radius = 8.0f, float height = 2.0f) {
    std::vector<QMatrix4x4> cams;
    for (int i = 0; i < numCams; ++i) {
        float angle = float(i) / numCams * 2.0f * float(M_PI);
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);
        QMatrix4x4 cam;
        cam.lookAt(QVector3D(x, height, z), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
        cams.push_back(cam.inverted());
    }
    return cams;
}

// 2. Cameras on a vertical tower, looking at the origin
inline std::vector<QMatrix4x4> exampleCameraExtrinsics_Tower(int numCams = 5, float x = 0.0f, float z = 8.0f, float minY = 1.0f, float maxY = 10.0f) {
    std::vector<QMatrix4x4> cams;
    for (int i = 0; i < numCams; ++i) {
        float y = minY + (maxY - minY) * i / (numCams - 1);
        QMatrix4x4 cam;
        cam.lookAt(QVector3D(x, y, z), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
        cams.push_back(cam.inverted());
    }
    return cams;
}

// 3. Cameras in a grid above the XY plane, looking down
inline std::vector<QMatrix4x4> exampleCameraExtrinsics_Grid(int nx = 3, int nz = 3, float y = 8.0f, float spacing = 6.0f) {
    std::vector<QMatrix4x4> cams;
    for (int ix = 0; ix < nx; ++ix) {
        for (int iz = 0; iz < nz; ++iz) {
            float x = (ix - (nx - 1) / 2.0f) * spacing;
            float z = (iz - (nz - 1) / 2.0f) * spacing;
            QMatrix4x4 cam;
            cam.lookAt(QVector3D(x, y, z), QVector3D(x, 0, z), QVector3D(0, 0, -1));
            cams.push_back(cam.inverted());
        }
    }
    return cams;
}

// 4. Cameras on a spiral ramp, looking at the origin
inline std::vector<QMatrix4x4> exampleCameraExtrinsics_Spiral(int numCams = 10, float radius = 8.0f, float minY = 1.0f, float maxY = 8.0f) {
    std::vector<QMatrix4x4> cams;
    for (int i = 0; i < numCams; ++i) {
        float t = float(i) / (numCams - 1);
        float angle = t * 4.0f * float(M_PI);
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);
        float y = minY + (maxY - minY) * t;
        QMatrix4x4 cam;
        cam.lookAt(QVector3D(x, y, z), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
        cams.push_back(cam.inverted());
    }
    return cams;
}