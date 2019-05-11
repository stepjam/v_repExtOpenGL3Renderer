#include "openglWidget.h"
#include "MyMath.h"
#include <QResizeEvent>
#include <iostream>

COpenglWidget::COpenglWidget(int associatedObjectHandle,const QGLFormat& format, QWidget *parent) : QGLWidget(format ,parent), COpenglBase(associatedObjectHandle)
{
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
    QGLWidget::makeCurrent();
}

void COpenglWidget::doneCurrentContext()
{
    QGLWidget::doneCurrent();
}

void COpenglWidget::paintEvent(QPaintEvent* event)
{
  QGLWidget::paintEvent(event);
}

void COpenglWidget::resizeEvent(QResizeEvent* rEvent)
{
    _resX=rEvent->size().width();
    _resY=rEvent->size().height();
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
}


