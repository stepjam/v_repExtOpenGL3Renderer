#include "openglWidget.h"
#include "MyMath.h"
#include <QResizeEvent>
#include <iostream>
#include <QOpenGLFramebufferObject>

COpenglWidget::COpenglWidget(int associatedObjectHandle, QOpenGLContext* qCont, QWindow *parent) : QWindow(parent), COpenglBase(associatedObjectHandle)
{
    setSurfaceType(QWindow::OpenGLSurface);
    this->qCont = qCont;
}

COpenglWidget::~COpenglWidget()
{
}

void COpenglWidget::initGL()
{
    makeContextCurrent();
    COpenglBase::initGL();
}

void COpenglWidget::makeContextCurrent()
{
    qCont->makeCurrent(this);
}

void COpenglWidget::doneCurrentContext()
{
    qCont->doneCurrent();
}

void COpenglWidget::paintEvent(QPaintEvent* event)
{
}

void COpenglWidget::resizeEvent(QResizeEvent* rEvent)
{
    _resX=rEvent->size().width() * devicePixelRatio();
    _resY=rEvent->size().height() * devicePixelRatio();
}

void COpenglWidget::paintGL()
{
}

void COpenglWidget::showAtGivenSizeAndPos(int resX,int resY,int posX,int posY)
{
    if ( (resX<=0)||(resY<=0) )
        showMaximized();
    else
    {
        showNormal();
        setGeometry(posX,posY,resX,resY);
    }
}

void COpenglWidget::getWindowResolution(int& resX,int& resY)
{
    resX=_resX;
    resY=_resY;
}

void COpenglWidget::bindFramebuffer()
{
    QOpenGLFramebufferObject::bindDefault();
}

void COpenglWidget::swapBuffers()
{
    qCont->swapBuffers(this);
}


