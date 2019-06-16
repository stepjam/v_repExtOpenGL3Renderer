#ifndef LIGHTSHADERS_H
#define LIGHTSHADERS_H

#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "container.h"

class LightShaders
{
public:
    LightShaders();
    ~LightShaders();
    QOpenGLShaderProgram* depthShader;
    QOpenGLShaderProgram* omniDepthShader;

    COcContainer<Mesh>* meshContainer;
    COcContainer<Texture>* textureContainer;


protected:
    int _associatedObjectHandle;
    int _resX;
    int _resY;
};
#endif // LIGHTSHADERS_H
