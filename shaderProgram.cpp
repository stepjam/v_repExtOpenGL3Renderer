#include "shaderProgram.h"

ShaderProgram::ShaderProgram(const QString& vertexShaderPath, const QString& fragmentShaderPath, const QString& geomShaderPath)
{
    shader = new QOpenGLShaderProgram();
    if ( !prepareShaderProgram(shader, vertexShaderPath, fragmentShaderPath, geomShaderPath) )
        return; // TODO: Raise exception.
}

ShaderProgram::~ShaderProgram()
{
    delete shader;
}

void ShaderProgram::bind()
{
    shader->bind();
}

int ShaderProgram::getUniformLocation(QString value)
{
    int result = -1;
    std::string s = value.toStdString();
    std::unordered_map<std::string, int>::iterator itr = uniformLocations.find(s);
    if(itr == uniformLocations.end())
    {
        result = shader->uniformLocation(value);
        uniformLocations[s] = result;
    }
    else
    {
        result = itr->second;
    }
    return result;
}

bool ShaderProgram::prepareShaderProgram(QOpenGLShaderProgram* sh, const QString& vertexShaderPath, const QString& fragmentShaderPath, const QString& geomShaderPath)
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

void ShaderProgram::setUniformValue(QString name, GLfloat value)
{
    shader->setUniformValue(getUniformLocation(name), value);
}

void ShaderProgram::setUniformValue(QString name, GLint value)
{
    shader->setUniformValue(getUniformLocation(name), value);
}

void ShaderProgram::setUniformValue(QString name, GLuint value)
{
    shader->setUniformValue(getUniformLocation(name), value);
}

void ShaderProgram::setUniformValue(QString name, const QVector2D& value)
{
    shader->setUniformValue(getUniformLocation(name), value);
}

void ShaderProgram::setUniformValue(QString name, const QVector3D& value)
{
    shader->setUniformValue(getUniformLocation(name), value);
}

void ShaderProgram::setUniformValue(QString name, const QVector4D& value)
{
    shader->setUniformValue(getUniformLocation(name), value);
}

void ShaderProgram::setUniformValue(QString name, const QMatrix4x4& value)
{
    shader->setUniformValue(getUniformLocation(name), value);
}

