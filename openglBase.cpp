#include "openglBase.h"
#include "MyMath.h"
#include <iostream>
#include <QOpenGLExtraFunctions>

COpenglBase::COpenglBase(int associatedObjectHandle)
{
    _associatedObjectHandle=associatedObjectHandle;
    m_shader = new QOpenGLShaderProgram();
    depthShader = new QOpenGLShaderProgram();
    omniDepthShader = new QOpenGLShaderProgram();
    meshContainer=new COcContainer<COcMesh>();
    textureContainer=new COcContainer<COcTexture>();
}

COpenglBase::~COpenglBase()
{
    delete m_shader;
    delete depthShader;
    delete omniDepthShader;
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
    makeContextCurrent();

    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Prepare a complete shader program…
    if ( !prepareShaderProgram(m_shader, ":/simple.vert", ":/simple.frag", "") )
        return;

    if ( !prepareShaderProgram(depthShader, ":/shadows/depth.vert", ":/shadows/depth.frag", "") )
        return;

    if ( !prepareShaderProgram(omniDepthShader, ":/shadows/omni_depth.vert", ":/shadows/omni_depth.frag", ""))
        return;
}

bool COpenglBase::prepareShaderProgram(QOpenGLShaderProgram* sh, const QString& vertexShaderPath, const QString& fragmentShaderPath, const QString& geomShaderPath)
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

void COpenglBase::clearBuffers(float viewAngle,float orthoViewSize,float nearClippingPlane,float farClippingPlane,bool perspectiveOperation,const float* backColor)
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    m_shader->bind();
    f->glViewport(0,0,_resX,_resY);
    f->glClearColor(backColor[0],backColor[1],backColor[2],0.0f);
    f->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    f->glEnable(GL_DEPTH_TEST);

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
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    f->glViewport(0,0,_resX,_resY);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


