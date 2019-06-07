#ifndef OCLIGHT_H
#define OCLIGHT_H

#include <vector>
#include "4X4Matrix.h"
#include <QVector3D>
#include <QOpenGLShaderProgram>
#include "ocMesh.h"

class COcLight
{
public:
     COcLight(int lightType, C4X4Matrix m, int counter, int totalcount, float* colors, float constAttenuation, float linAttenuation, float quadAttenuation, float cutoffAngle, int spotExponent, float near_plane, float far_plane, float orthoWidth, int shadowTextureSize, QOpenGLShaderProgram* camShader);
    ~COcLight();
     void renderDepthFromLight(QOpenGLShaderProgram* depthSh, std::vector<COcMesh*>* meshesToRender);
     unsigned int depthMapFBO;
     unsigned int depthMap;
     int lightType;
private:
     QVector3D lightPos;
     QMatrix4x4 lightSpaceMat;
     QMatrix4x4 lightSpaceMats[6];
     float farPlane;
     float shadowTexSize;
};
#endif // OCLIGHT_H
