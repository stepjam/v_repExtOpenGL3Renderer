#include "openglWindow.h"
#include "MyMath.h"
#include <QResizeEvent>
#include <iostream>
#include <QOpenGLFramebufferObject>

OpenglWindow::OpenglWindow(int associatedObjectHandle, QOpenGLContext* qCont, QWindow *parent) : QWindow(parent), COpenglBase(associatedObjectHandle)
{
    setSurfaceType(QWindow::OpenGLSurface);
    this->qCont = qCont;
}

OpenglWindow::~OpenglWindow()
{
}

void OpenglWindow::initGL()
{
    makeContextCurrent();
    COpenglBase::initGL();
}

void OpenglWindow::makeContextCurrent()
{
    qCont->makeCurrent(this);
}

void OpenglWindow::doneCurrentContext()
{
    qCont->doneCurrent();
}

void OpenglWindow::paintEvent(QPaintEvent* event)
{
}

void OpenglWindow::resizeEvent(QResizeEvent* rEvent)
{
    _resX=rEvent->size().width() * devicePixelRatio();
    _resY=rEvent->size().height() * devicePixelRatio();
}

void OpenglWindow::paintGL()
{
}

void OpenglWindow::showAtGivenSizeAndPos(int resX,int resY,int posX,int posY)
{
    if ( (resX<=0)||(resY<=0) )
        showMaximized();
    else
    {
        showNormal();
        setGeometry(posX,posY,resX,resY);
    }
}

void OpenglWindow::getWindowResolution(int& resX,int& resY)
{
    resX=_resX;
    resY=_resY;
}

void OpenglWindow::bindFramebuffer()
{
    QOpenGLFramebufferObject::bindDefault();
}

void OpenglWindow::swapBuffers()
{
    qCont->swapBuffers(this);
}


