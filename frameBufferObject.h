#pragma once

#include <QObject>
#include <QOpenGLFramebufferObject>

class CFrameBufferObject : public QObject
{
    Q_OBJECT
public:

    CFrameBufferObject(int resX,int resY);
    virtual ~CFrameBufferObject();

    void switchToFbo();
    void switchToNonFbo();

protected:
    QOpenGLFramebufferObject* _frameBufferObject;
};
