#ifndef LIGHT_H
#define LIGHT_H

#include <Vector.h>
#include "4X4Matrix.h"
#include <QVector3D>
#include "shaderProgram.h"
#include "light.h"
#include "mesh.h"


class Light: protected QOpenGLExtraFunctions
{
public:
     Light(int lightType, int shadowTextureSize);
    ~Light();
     void initForCamera(int handle, int lightType, C4X4Matrix m, int counter, int totalcount, float* colors, float constAttenuation, float linAttenuation, float quadAttenuation, float cutoffAngle, int spotExponent, float near_plane, float far_plane, float orthoWidth, int shadowTextureSize, float bias, float normalBias, ShaderProgram* camShader);
     void renderDepthFromLight(ShaderProgram* depthSh, std::vector<Mesh*> meshesToRender);
     unsigned int depthMapFBO;
     unsigned int depthMap;
     int lightType;
     void decrementUsedCount();
     int getUsedCount();
     int getId();

     void setPose(int lightType, C4X4Matrix m, ShaderProgram* camShader);

protected:
     int _usedCount;
     int _id;
private:
     void prepareDepthMapFBO(int lightType, int shadowTextureSize);

     QVector3D lightPos;
     QMatrix4x4 lightSpaceMat;
     QMatrix4x4 lightSpaceMats[6];
     float farPlane;
     float shadowTexSize;

     QMatrix4x4 lightProjection;

     QString lightName;

};
#endif // LIGHT_H
