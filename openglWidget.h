#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include "openglBase.h"
#include <QWindow>

#include <QGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLContext>

class COpenglWidget : public QWindow, public COpenglBase
{
    Q_OBJECT
public:
    explicit COpenglWidget(int associatedObjectHandle, QOpenGLContext* qCont, QWindow *parent = 0);
    ~COpenglWidget();

    void initGL();
    void makeContextCurrent();
    void doneCurrentContext();
    void bindFramebuffer();
    void swapBuffers();

    void showAtGivenSizeAndPos(int resX,int resY,int posX,int posY);
    void getWindowResolution(int& resX,int& resY);

protected:
    void resizeEvent(QResizeEvent* rEvent);

    void paintGL();
    void paintEvent(QPaintEvent* event);

    QOpenGLContext* qCont;

};
#endif // OPENGLWIDGET_H
