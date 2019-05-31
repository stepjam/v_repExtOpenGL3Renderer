#include "ocLight.h"
#include <QOpenGLExtraFunctions>
#include "v_repLib.h"
#include <iostream>

#define TEXTURE_INIT_USED_COUNT 10

COcLight::COcLight(int lightType, C4X4Matrix m, int counter, int totalcount, float* colors, float constAttenuation, float linAttenuation, float quadAttenuation, float cutoffAngle, int spotExponent, QOpenGLShaderProgram* camShader)
{
    this->lightType = lightType;

    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    f->glGenFramebuffers(1, &depthMapFBO);
    f->glGenTextures(1, &depthMap);

    if(lightType == sim_light_omnidirectional_subtype){
        // We treat the depthMap as a cube map.
        f->glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap);
        f->glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        for (unsigned int i = 0; i < 6; ++i)
        {
            f->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, depthMap, 0);
        }
        f->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        f->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        f->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        f->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        f->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    } else {
        f->glBindTexture(GL_TEXTURE_2D, depthMap);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
        f->glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        f->glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    }

    glDrawBuffer(GL_NONE);
    f->glReadBuffer(GL_NONE);
    f->glBindFramebuffer(GL_FRAMEBUFFER, 0);


    lightPos = QVector3D(m.X(0), m.X(1), m.X(2));
    QVector3D lightDir = QVector3D(m.M.axis[2](0), m.M.axis[2](1), m.M.axis[2](2));
    QVector3D diffuseLight = QVector3D(colors[3],colors[4],colors[5]);
    QVector3D specularLight = QVector3D(colors[6],colors[7],colors[8]);

    QString lightName = "";
    QString direction = ".direction";
    QString position = ".position";
    QString ambient = ".ambient";
    QString diffuse = ".diffuse";
    QString specular = ".specular";
    QString constant = ".constant";
    QString linear = ".linear";
    QString quadratic = ".quadratic";

    QString lightSpaceMatrix = ".lightSpaceMatrix";
    QString shadowMap = ".shadowMap";

    camShader->bind();

    // The following instructions have the same effect as gluLookAt()
    m.inverse();
    m.rotateAroundY(3.14159265359f);
    float m4_[4][4];
    m.copyTo(m4_);
    #define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}
    float temp;
    SWAP(m4_[0][1],m4_[1][0]);
    SWAP(m4_[0][2],m4_[2][0]);
    SWAP(m4_[0][3],m4_[3][0]);
    SWAP(m4_[1][2],m4_[2][1]);
    SWAP(m4_[1][3],m4_[3][1]);
    SWAP(m4_[2][3],m4_[3][2]);
    #undef SWAP
    // Set the view matrix
    QMatrix4x4 lightView = QMatrix4x4(
                m4_[0][0], m4_[1][0], m4_[2][0], m4_[3][0],
                m4_[0][1], m4_[1][1], m4_[2][1], m4_[3][1],
                m4_[0][2], m4_[1][2], m4_[2][2], m4_[3][2],
                m4_[0][3], m4_[1][3], m4_[2][3], m4_[3][3]);



    float near_plane = 0.10f, far_plane = 10.0f;
    this->farPlane = far_plane;

    float aspect = (float)SHADOW_WIDTH/(float)SHADOW_HEIGHT;
    QMatrix4x4 lightProjection;
    lightProjection.setToIdentity();

    if (lightType == sim_light_directional_subtype)
    {
        lightName.append("dirLight");
        lightName.append(QString::number(counter));
        camShader->setUniformValue(camShader->uniformLocation("dirLightLen"), counter+1);
        lightProjection.ortho(-2.0f, 2.0f, -2.0f, 2.0f, near_plane, far_plane);
    } else if (lightType == sim_light_omnidirectional_subtype) {
        lightName.append("pointLight");
        lightName.append(QString::number(counter));
        camShader->setUniformValue(camShader->uniformLocation("pointLightLen"), counter+1);

        // 90 degrees making the viewing field large enough to properly fill a single face of the cubemap
        lightProjection.perspective(90, aspect, near_plane, far_plane);

        QString farplane = ".farPlane";
        farplane.prepend(lightName);
        camShader->setUniformValue(camShader->uniformLocation(farplane), far_plane);

        for(int i=0; i < 6; i++)
            lightSpaceMats[i].setToIdentity();
        lightSpaceMats[0].lookAt(lightPos, lightPos + QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f));
        lightSpaceMats[1].lookAt(lightPos, lightPos + QVector3D(-1.0f, 0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f));
        lightSpaceMats[2].lookAt(lightPos, lightPos + QVector3D(0.0f, 1.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f));
        lightSpaceMats[3].lookAt(lightPos, lightPos + QVector3D(0.0f, -1.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f));
        lightSpaceMats[4].lookAt(lightPos, lightPos + QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0.0f, -1.0f, 0.0f));
        lightSpaceMats[5].lookAt(lightPos, lightPos + QVector3D(0.0f, 0.0f, -1.0f), QVector3D(0.0f, -1.0f, 0.0f));
        for(int i=0; i < 6; i++)
            lightSpaceMats[i] = lightProjection * lightSpaceMats[i];

    } else if (lightType == sim_light_spot_subtype) {
        lightName.append("spotLight");
        lightName.append(QString::number(counter));
        camShader->setUniformValue(camShader->uniformLocation("spotLightLen"), counter+1);

        lightProjection.perspective(cutoffAngle*radToDeg, aspect, near_plane, far_plane);

        QString cutOff = ".cutOff";
        QString outerCutOff = ".outerCutOff";
        cutOff.prepend(lightName);
        outerCutOff.prepend(lightName);

        // Do some checks to make sure we font break anything in the shaders
        // Make the outer cut-off a little less than the cut off
        camShader->setUniformValue(camShader->uniformLocation(cutOff), (GLfloat) cos(0.0001f));
        camShader->setUniformValue(camShader->uniformLocation(outerCutOff), (GLfloat) cos(cutoffAngle));
    }

    ambient.prepend(lightName);
    diffuse.prepend(lightName);
    specular.prepend(lightName);
    direction.prepend(lightName);
    position.prepend(lightName);
    constant.prepend(lightName);
    linear.prepend(lightName);
    quadratic.prepend(lightName);
    lightSpaceMatrix.prepend(lightName);
    shadowMap.prepend(lightName);

    // In vrep, you cant set the ambient light per light source.
    camShader->setUniformValue(camShader->uniformLocation(diffuse), diffuseLight);
    camShader->setUniformValue(camShader->uniformLocation(specular), specularLight);
    camShader->setUniformValue(camShader->uniformLocation(direction), lightDir);
    camShader->setUniformValue(camShader->uniformLocation(position), lightPos);
    camShader->setUniformValue(camShader->uniformLocation(constant), constAttenuation);
    camShader->setUniformValue(camShader->uniformLocation(linear), linAttenuation);
    camShader->setUniformValue(camShader->uniformLocation(quadratic), quadAttenuation);

    if (lightType == sim_light_omnidirectional_subtype) {
        // Hack in and set the other maps also equal to this so the shader works
        for (int i = counter; i < 5; i++){
            QString depthCubeMaps = "depthCubeMap";
            depthCubeMaps.append(QString::number(i));
            camShader->setUniformValue(
                        camShader->uniformLocation(depthCubeMaps), totalcount);
        }
    } else {
        camShader->setUniformValue(
                    camShader->uniformLocation(shadowMap), totalcount);
        lightSpaceMat = lightProjection * lightView;
        camShader->setUniformValue(camShader->uniformLocation(lightSpaceMatrix), lightSpaceMat);
    }

}

COcLight::~COcLight()
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    f->glDeleteFramebuffers(1, &depthMapFBO);
    f->glDeleteTextures(1, &depthMap);
}

void COcLight::renderDepthFromLight(QOpenGLShaderProgram* depthShader, std::vector<COcMesh*>* meshesToRender)
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    depthShader->bind();

    f->glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    f->glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    f->glClear(GL_DEPTH_BUFFER_BIT);

    if (lightType == sim_light_omnidirectional_subtype) {
        f->glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap);
        depthShader->setUniformValue(
                    depthShader->uniformLocation("lightPos"), lightPos);
        depthShader->setUniformValue(
                    depthShader->uniformLocation("far_plane"), farPlane);
        for(int i = 0; i < 6; i++){
            depthShader->setUniformValue(
                        depthShader->uniformLocation("lightSpaceMatrix"), lightSpaceMats[i]);

            GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
            f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, depthMap, 0);
            f->glClear(GL_DEPTH_BUFFER_BIT);
            // render depth of scene to texture (from light's perspective)
            for (size_t i=0;i<meshesToRender->size();i++)
            {
                (*meshesToRender)[i]->renderDepth(depthShader);
            }
        }
    } else {
        f->glBindTexture(GL_TEXTURE_2D, depthMap);
        depthShader->setUniformValue(
                    depthShader->uniformLocation("lightSpaceMatrix"), lightSpaceMat);
        // render depth of scene to texture (from light's perspective)
        for (size_t i=0;i<meshesToRender->size();i++)
        {
            (*meshesToRender)[i]->renderDepth(depthShader);
        }
    }
    f->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


