#include "frameBufferObject.h"
#include <iostream>

CFrameBufferObject::CFrameBufferObject(int resX,int resY) : QObject()
{
    QOpenGLFramebufferObject::Attachment attachment=QOpenGLFramebufferObject::Depth;
    _frameBufferObject = new QOpenGLFramebufferObject(resX,resY,attachment,GL_TEXTURE_2D,GL_RGBA8); // GL_RGB);
}

CFrameBufferObject::~CFrameBufferObject()
{
    switchToNonFbo();
    delete _frameBufferObject;
}

void CFrameBufferObject::switchToFbo()
{
    _frameBufferObject->bind();
}

void CFrameBufferObject::switchToNonFbo()
{
    _frameBufferObject->release();
}
