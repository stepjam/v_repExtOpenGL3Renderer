#ifndef UTILS_H
#define UTILS_H

#include <QOpenGLShaderProgram>
#include <QOpenGLShader>

bool prepareShaderProgram( QOpenGLShaderProgram* sh,
                           const QString& vertexShaderPath,
                           const QString& fragmentShaderPath,
                           const QString& geomShaderPath);

#endif // UTILS_H
