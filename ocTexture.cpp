#include "ocTexture.h"
#include <QOpenGLExtraFunctions>
#include <iostream>

#define TEXTURE_INIT_USED_COUNT 10

COcTexture::COcTexture(int id,const unsigned char* textureBuff,int textureSizeX,int textureSizeY)
{
//    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    initializeOpenGLFunctions();
    _id=id;
    _textureSizeX=textureSizeX;
    _textureSizeY=textureSizeY;
    for (int i=0;i<4*textureSizeX*textureSizeY;i++)
        _textureBuff.push_back(textureBuff[i]);
    glGenTextures(1,&_textureName);
    _usedCount=TEXTURE_INIT_USED_COUNT;
}

COcTexture::~COcTexture()
{
    glDeleteTextures(1,&_textureName);
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
    _usedCount=TEXTURE_INIT_USED_COUNT;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,_textureName);

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,_textureSizeX,_textureSizeY,0,GL_RGBA,GL_UNSIGNED_BYTE,&_textureBuff[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

    GLint colInt=GL_NEAREST;
    if (interpolateColors)
        colInt=GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,colInt);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,colInt);
    GLint repS=GL_CLAMP_TO_EDGE;
    if (repeatU)
        repS=GL_REPEAT;
    glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,repS);
    GLint repT=GL_CLAMP_TO_EDGE;
    if (repeatV)
        repT=GL_REPEAT;
    glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,repT);

    std::cout << "3: " << glGetError() << std::endl;
    glBindTexture(GL_TEXTURE_2D,_textureName);
}

void COcTexture::endTexture()
{

}

