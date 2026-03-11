#ifndef SHADER_H
#define SHADER_H

#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>
#include <QString>

class Shader : public QOpenGLShaderProgram {
public:
    Shader(QObject* parent = nullptr);

    // Load and link vertex/fragment shaders from file paths
    bool load(const QString& vertexPath, const QString& fragmentPath);

    // Set uniforms (Qt types)
    void setMat4(const char* name, const QMatrix4x4& mat);
    void setVec3(const char* name, const QVector3D& vec);
};

#endif