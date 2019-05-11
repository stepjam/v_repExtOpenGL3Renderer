#include "ocTexture.h"
#include <QOpenGLExtraFunctions>

#define TEXTURE_INIT_USED_COUNT 10

COcTexture::COcTexture(int id,const unsigned char* textureBuff,int textureSizeX,int textureSizeY)
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    _id=id;
    _textureSizeX=textureSizeX;
    _textureSizeY=textureSizeY;
    for (int i=0;i<4*textureSizeX*textureSizeY;i++)
        _textureBuff.push_back(textureBuff[i]);
    f->glGenTextures(1,&_textureName);
    _usedCount=TEXTURE_INIT_USED_COUNT;
}

COcTexture::~COcTexture()
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    f->glDeleteTextures(1,&_textureName);
}

void COcTexture::decrementUsedCount()
{
    _usedCount--;
}

int COcTexture::getUsedCount()
{
    return(_usedCount);
}

int COcTexture::getId()
{
    return(_id);
}

void COcTexture::startTexture(bool repeatU,bool repeatV,bool interpolateColors,int applyMode)
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    _usedCount=TEXTURE_INIT_USED_COUNT;
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D,_textureName);

    f->glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,_textureSizeX,_textureSizeY,0,GL_RGBA,GL_UNSIGNED_BYTE,&_textureBuff[0]);
    f->glGenerateMipmap(GL_TEXTURE_2D);

    GLint colInt=GL_NEAREST;
    if (interpolateColors)
        colInt=GL_LINEAR;
    f->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,colInt);
    GLint repS=GL_CLAMP;
    if (repeatU)
        repS=GL_REPEAT;
    f->glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,repS);
    GLint repT=GL_CLAMP;
    if (repeatV)
        repT=GL_REPEAT;
    f->glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,repT);
    f->glBindTexture(GL_TEXTURE_2D,_textureName);
}

void COcTexture::endTexture()
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    f->glBindTexture(GL_TEXTURE_2D, 0);
}

