#include "openglBase.h"
#include "MyMath.h"
#include <iostream>
#include <QOpenGLExtraFunctions>
#include "utils.h"


COpenglBase::COpenglBase(int associatedObjectHandle)
{
    _associatedObjectHandle=associatedObjectHandle;
    m_shader = new QOpenGLShaderProgram();
}

COpenglBase::~COpenglBase()
{
    delete m_shader;
}

int COpenglBase::getAssociatedObjectHandle()
{
    return(_associatedObjectHandle);
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
//    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    initializeOpenGLFunctions();

    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Prepare a complete shader program…
    if ( !prepareShaderProgram(m_shader, ":/default.vert", ":/default.frag", "") )
        return;

    glGenTextures(1,&blankTexture);
    glGenTextures(1,&blankTexture2);
}

void COpenglBase::clearBuffers(float viewAngle,float orthoViewSize,float nearClippingPlane,float farClippingPlane,bool perspectiveOperation,const float* backColor)
{
//    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    m_shader->bind();
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
    m_shader->setUniformValue(m_shader->uniformLocation("projection"), m_proj);
}

void COpenglBase::clearViewport()
{
//    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    glViewport(0,0,_resX,_resY);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
