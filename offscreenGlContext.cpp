#include "offscreenGlContext.h"

COffscreenGlContext::COffscreenGlContext(int resX,int resY, QOpenGLContext* qCont) : QObject()
{
    _qOffscreenSurface=new QOffscreenSurface();
    QSurfaceFormat f;
    f.setSwapBehavior(QSurfaceFormat::SingleBuffer);
    f.setRenderableType(QSurfaceFormat::OpenGL);
    f.setRedBufferSize(8);
    f.setGreenBufferSize(8);
    f.setBlueBufferSize(8);
    f.setAlphaBufferSize(0);
    f.setStencilBufferSize(8);
    f.setDepthBufferSize(24);
    _qOffscreenSurface->setFormat(f);
    _qOffscreenSurface->create();
    _qContext = qCont;

#ifdef __APPLE__
    if (_qOffscreenSurface->isValid())
    {
        _qContext= new QOpenGLContext();
        _qContext->setShareContext(qCont);
        _qContext->create();
    }
#endif
    makeCurrent();
}

COffscreenGlContext::~COffscreenGlContext()
{
    _qOffscreenSurface->destroy();
    delete _qOffscreenSurface;
#ifdef __APPLE__
    delete _qContext;
#endif
}

bool COffscreenGlContext::makeCurrent()
{
    _qContext->makeCurrent(_qOffscreenSurface);
    return(true);
}

bool COffscreenGlContext::doneCurrent()
{
    _qContext->doneCurrent();
    return(true);
}

