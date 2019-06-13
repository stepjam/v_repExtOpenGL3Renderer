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
    unsigned int blankTexture, blankTexture2;

//protected:
    int _associatedObjectHandle;
    int _resX;
    int _resY;

};
#endif // OPENGLBASE_H
