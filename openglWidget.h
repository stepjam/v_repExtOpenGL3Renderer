#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include "openglBase.h"
#include <QGLWidget>

#include <QGLBuffer>
#include <QOpenGLShaderProgram>

class COpenglWidget : public QGLWidget, public COpenglBase
{
    Q_OBJECT
public:
    explicit COpenglWidget(int associatedObjectHandle, const QGLFormat& format, QWidget *parent = 0);
    ~COpenglWidget();

    void initGL();
    void makeContextCurrent();
    void doneCurrentContext();
    void bindFramebuffer();

    void showAtGivenSizeAndPos(int resX,int resY,int posX,int posY);
    void getWindowResolution(int& resX,int& resY);

protected:
    void resizeEvent(QResizeEvent* rEvent);

    void paintGL();
//    void initializeGL();
    void paintEvent(QPaintEvent* event);

};
#endif // OPENGLWIDGET_H
