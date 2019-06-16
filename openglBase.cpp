#include "openglBase.h"
#include "MyMath.h"
#include <iostream>
#include <QOpenGLExtraFunctions>

COpenglBase::COpenglBase(int associatedObjectHandle)
{
    this->associatedObjectHandle = associatedObjectHandle;
}

COpenglBase::~COpenglBase()
{
    delete shader;
}

int COpenglBase::getAssociatedObjectHandle()
{
    return(associatedObjectHandle);
}

void COpenglBase::makeContextCurrent()
{
}

void COpenglBase::doneCurrentContext()
{
}

void COpenglBase::bindFramebuffer()
{
}

void COpenglBase::initGL()
{
    initializeOpenGLFunctions();

    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

     // Each surface has its own shader.
    shader = new ShaderProgram(":/default.vert", ":/default.frag", "");

    glGenTextures(1,&blankTexture);
    glGenTextures(1,&blankTexture2);
}

void COpenglBase::clearBuffers(float viewAngle,float orthoViewSize,float nearClippingPlane,float farClippingPlane,bool perspectiveOperation,const float* backColor)
{
    shader->bind();
    glViewport(0,0,_resX,_resY);
    glClearColor(backColor[0],backColor[1],backColor[2],0.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    QMatrix4x4 m_proj;
    m_proj.setToIdentity();
    float ratio=float(_resX)/float(_resY);
    if (perspectiveOperation)
    {
        if (ratio>1.0f)
        {
            float a=2.0f*(float)atan(tan(viewAngle/2.0f)/ratio)*radToDeg;
            m_proj.perspective(a, ratio, nearClippingPlane,farClippingPlane);
        }
        else
        {
            m_proj.perspective(viewAngle*radToDeg, ratio, nearClippingPlane,farClippingPlane);
        }
    }
    else
    {
        if (ratio>1.0f)
            m_proj.ortho(-orthoViewSize*0.5f,orthoViewSize*0.5f,-orthoViewSize*0.5f/ratio,orthoViewSize*0.5f/ratio,nearClippingPlane,farClippingPlane);
        else
            m_proj.ortho(-orthoViewSize*0.5f*ratio,orthoViewSize*0.5f*ratio,-orthoViewSize*0.5f,orthoViewSize*0.5f,nearClippingPlane,farClippingPlane);
    }
    shader->setUniformValue("projection", m_proj);
}

void COpenglBase::clearViewport()
{
    glViewport(0,0,_resX,_resY);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
