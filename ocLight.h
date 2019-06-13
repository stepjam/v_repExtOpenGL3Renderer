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
     COcLight(int handle, int lightType, C4X4Matrix m, int counter, int totalcount, float* colors, float constAttenuation, float linAttenuation, float quadAttenuation, float cutoffAngle, int spotExponent, float near_plane, float far_plane, float orthoWidth, int shadowTextureSize);
    ~COcLight();
     void initForCamera(int handle, int lightType, C4X4Matrix m, int counter, int totalcount, float* colors, float constAttenuation, float linAttenuation, float quadAttenuation, float cutoffAngle, int spotExponent, float near_plane, float far_plane, float orthoWidth, int shadowTextureSize, QOpenGLShaderProgram* camShader);
     void renderDepthFromLight(QOpenGLShaderProgram* depthSh, std::vector<COcMesh*>* meshesToRender);
     unsigned int depthMapFBO;
     unsigned int depthMap;
     int lightType;
     void decrementUsedCount();
     int getUsedCount();
     int getId();

     void setPose(int lightType, C4X4Matrix m, QOpenGLShaderProgram* camShader);

     std::vector<int> seenCamIds;
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
#endif // OCLIGHT_H
