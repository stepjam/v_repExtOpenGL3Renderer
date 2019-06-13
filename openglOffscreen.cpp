#include "openglOffscreen.h"
#include <iostream>

COpenglOffscreen::COpenglOffscreen(int associatedObjectHandle,int resX,int resY, QOpenGLContext* qCont) : COpenglBase(associatedObjectHandle)
{
    _resX=resX;
    _resY=resY;
    _offscreenContext=new COffscreenGlContext(resX,resY, qCont);
    _frameBufferObject=new CFrameBufferObject(resX,resY);
    _offscreenContext->doneCurrent();
    _offscreenContext->makeCurrent();
    _frameBufferObject->switchToFbo();
}

COpenglOffscreen::~COpenglOffscreen()
{
    _offscreenContext->makeCurrent();
    delete _frameBufferObject;
    _offscreenContext->doneCurrent();
    delete _offscreenContext;
}

bool COpenglOffscreen::isResolutionSame(int resX,int resY)
{
    return ((resX==_resX)&&(resY=_resY));
}

void COpenglOffscreen::initGL()
{
    makeContextCurrent();
    COpenglBase::initGL();
}

void COpenglOffscreen::makeContextCurrent()
{
    _offscreenContext->makeCurrent();
}

void COpenglOffscreen::doneCurrentContext()
{
    _offscreenContext->doneCurrent();
}

void COpenglOffscreen::bindFramebuffer()
{
    _frameBufferObject->switchToFbo();
}

void COpenglOffscreen::unbindFramebuffer()
{
    _frameBufferObject->switchToNonFbo();
}


