#include "lightShaders.h"

LightShaders::LightShaders()
{
    depthShader = new QOpenGLShaderProgram();
    omniDepthShader = new QOpenGLShaderProgram();
    if ( !prepareShaderProgram(depthShader, ":/shadows/depth.vert", ":/shadows/depth.frag", "") )
        return;
    if ( !prepareShaderProgram(omniDepthShader, ":/shadows/omni_depth.vert", ":/shadows/omni_depth.frag", ""))
        return;

    meshContainer = new COcContainer<Mesh>();
    textureContainer = new COcContainer<Texture>();
}

LightShaders::~LightShaders()
{
    delete meshContainer;
    delete textureContainer;

    delete depthShader;
    delete omniDepthShader;
}
