#include "shader.h"
#include <QFile>
#include <QTextStream>

Shader::Shader(QObject* parent)
    : QOpenGLShaderProgram(parent)
{
}

bool Shader::load(const QString& vertexPath, const QString& fragmentPath)
{
    // Load and compile vertex shader
    if (!addShaderFromSourceFile(QOpenGLShader::Vertex, vertexPath))
        return false;
    // Load and compile fragment shader
    if (!addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentPath))
        return false;
    // Link shader program
    return link();
}

void Shader::setMat4(const char* name, const QMatrix4x4& mat)
{
    setUniformValue(name, mat);
}

void Shader::setVec3(const char* name, const QVector3D& vec)
{
    setUniformValue(name, vec);
}