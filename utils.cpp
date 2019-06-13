#include "utils.h"

bool prepareShaderProgram(QOpenGLShaderProgram* sh, const QString& vertexShaderPath, const QString& fragmentShaderPath, const QString& geomShaderPath)
{
 // First we load and compile the vertex shader…

    bool result = sh->addShaderFromSourceFile( QOpenGLShader::Vertex, vertexShaderPath);

    if ( !result )
    qWarning() << sh->log();

    // …now the fragment shader…
    result = sh->addShaderFromSourceFile( QOpenGLShader::Fragment, fragmentShaderPath );
    if ( !result )
    qWarning() << sh->log();

    if (geomShaderPath.length() > 0){
        result = sh->addShaderFromSourceFile( QOpenGLShader::Geometry, geomShaderPath );
        if ( !result )
        qWarning() << sh->log();
    }

    // …and finally we link them to resolve any references.
    result = sh->link();
    if ( !result )
    qWarning() << "Could not link shader program:" << sh->log();
    return result;
}
