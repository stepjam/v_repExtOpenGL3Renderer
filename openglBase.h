#ifndef OPENGLBASE_H
#define OPENGLBASE_H

#include <QGLWidget>
#include <QGLBuffer>
#include <QOpenGLShaderProgram>
#include "ocMesh.h"
#include "ocTexture.h"
#include "ocContainer.h"

class COpenglBase: protected QOpenGLExtraFunctions
{
public:
    COpenglBase(int associatedObjectHandle);
    virtual ~COpenglBase();

    void clearBuffers(float viewAngle,float orthoViewSize,float nearClippingPlane,float farClippingPlane,bool perspectiveOperation,const float* backColor);
    int getAssociatedObjectHandle();
    void clearViewport();
    virtual void bindFramebuffer();
    virtual void initGL();
    virtual void makeContextCurrent();
    virtual void doneCurrentContext();

    QOpenGLShaderProgram* m_shader;
//    QOpenGLShaderProgram* depthShader;
//    QOpenGLShaderProgram* omniDepthShader;

//    COcContainer<COcMesh>* meshContainer;
//    COcContainer<COcTexture>* textureContainer;

    unsigned int blankTexture, blankTexture2;

protected:
    int _associatedObjectHandle;
    int _resX;
    int _resY;
//    bool prepareShaderProgram( QOpenGLShaderProgram* sh,
//                               const QString& vertexShaderPath,
//                               const QString& fragmentShaderPath,
//                               const QString& geomShaderPath);

};
#endif // OPENGLBASE_H
