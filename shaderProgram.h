#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <unordered_map>
#include <iostream>
#include <QString>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>

class ShaderProgram
{
public:
    ShaderProgram(const QString& vertexShaderPath, const QString& fragmentShaderPath, const QString& geomShaderPath);
    ~ShaderProgram();
    int getUniformLocation(QString value);
    void bind();

    void setUniformValue(QString name, GLfloat value);
    void setUniformValue(QString name, GLint value);
    void setUniformValue(QString name, GLuint value);
    void setUniformValue(QString name, const QVector2D& value);
    void setUniformValue(QString name, const QVector3D& value);
    void setUniformValue(QString name, const QVector4D& value);
    void setUniformValue(QString name, const QMatrix4x4& value);

    QOpenGLShaderProgram* shader;
private:
    std::unordered_map<std::string, int> uniformLocations;
    bool prepareShaderProgram( QOpenGLShaderProgram* sh,
                               const QString& vertexShaderPath,
                               const QString& fragmentShaderPath,
                               const QString& geomShaderPath);
};
#endif // SHADERPROGRAM_H
