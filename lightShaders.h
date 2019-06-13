#ifndef LIGHTSHADERS_H
#define LIGHTSHADERS_H

#include "utils.h"
#include "ocMesh.h"
#include "ocTexture.h"
#include "ocContainer.h"

class LightShaders
{
public:
    LightShaders();
    ~LightShaders();
    QOpenGLShaderProgram* depthShader;
    QOpenGLShaderProgram* omniDepthShader;

    COcContainer<COcMesh>* meshContainer;
    COcContainer<COcTexture>* textureContainer;


protected:
    int _associatedObjectHandle;
    int _resX;
    int _resY;
};
#endif // LIGHTSHADERS_H
