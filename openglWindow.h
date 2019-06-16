#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include "openglBase.h"
#include <QWindow>

#include <QGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLContext>

class OpenglWindow : public QWindow, public COpenglBase
{
    Q_OBJECT
public:
    explicit OpenglWindow(int associatedObjectHandle, QOpenGLContext* qCont, QWindow *parent = 0);
    ~OpenglWindow();

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
