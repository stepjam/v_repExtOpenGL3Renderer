#ifndef OPENGLOFFSCREEN_H
#define OPENGLOFFSCREEN_H

#include "openglBase.h"
#include "frameBufferObject.h"
#include "offscreenGlContext.h"

class COpenglOffscreen : public COpenglBase
{
public:
    COpenglOffscreen(int associatedObjectHandle,int resX,int resY);
    ~COpenglOffscreen();

    void initGL();
    void makeContextCurrent();
    void doneCurrentContext();
    void bindFramebuffer();
    bool isResolutionSame(int resX,int resY);

protected:
    COffscreenGlContext* _offscreenContext;
    CFrameBufferObject* _frameBufferObject;
};
#endif // OPENGLOFFSCREEN_H
