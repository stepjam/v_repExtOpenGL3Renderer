#ifndef OPENGLBASE_H
#define OPENGLBASE_H

#include <QGLWidget>
#include <QGLBuffer>
#include <QOpenGLShaderProgram>
#include "mesh.h"
#include "texture.h"
#include "container.h"
#include "shaderProgram.h"


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

    ShaderProgram* shader = NULL;
    unsigned int blankTexture, blankTexture2;

//protected:
    int associatedObjectHandle;
    int _resX;
    int _resY;

};
#endif // OPENGLBASE_H
