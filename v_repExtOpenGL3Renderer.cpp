#include "v_repExtOpenGL3Renderer.h"
#include "v_repLib.h"
#include "4X4Matrix.h"
#include <iostream>
#include "openglWidget.h"
#include "openglOffscreen.h"
#include "ocLight.h"
#include "lightShaders.h"
#include <QCoreApplication>
#include <algorithm>


#ifdef _WIN32
    #include <direct.h>
#endif /* _WIN32 */

#if defined (__linux) || defined (__APPLE__)
    #include <unistd.h>
    #include <string.h>
    #define _stricmp(x,y) strcasecmp(x,y)
#endif

#define MAX_LIGHTS 5

LIBRARY vrepLib;

int stepsSinceLastShadowMapRender = 99;
COcContainer<COcLight>* lightContainer = NULL;

QOpenGLContext* _qContext = NULL;

int resolutionX;
int resolutionY;
float nearClippingPlane;
float farClippingPlane;
bool perspectiveOperation;
int visionSensorOrCameraId;

int activeDirLightCounter, activePointLightCounter, activeSpotLightCounter;

bool _simulationRunning=false;
bool _cleanedUp=true;

std::vector<COpenglWidget*> oglWidgets;
std::vector<COpenglOffscreen*> oglOffscreens;

COpenglBase* activeBase = NULL;
LightShaders* lightShaders = NULL;

QVector3D sceneAmbientLight;

std::vector<COcLight*> lightsToRender;
std::vector<COcMesh*> meshesToRender;

void simulationAboutToStart()
{
    _simulationRunning=true;
    _cleanedUp = false;
}

void simulationEnded()
{
    // This is called from the Sim thread, and so we will have to cleanup later when the UI thread comes through.
    _simulationRunning=false;
    _cleanedUp = false;
}

void simulationGuiPass()
{
    if (!_simulationRunning && !_cleanedUp){
        // Windowed views can only run while simulation is running
        for (size_t i=0;i<oglWidgets.size();i++){
            delete oglWidgets[i];
        }
        oglWidgets.clear();
        delete lightShaders;
        lightShaders = NULL;
        delete lightContainer;
        lightContainer = NULL;

        _cleanedUp = true;
    }
}

COpenglWidget* getWidget(int objectHandle)
{
    for (size_t i=0;i<oglWidgets.size();i++)
    {
        if (oglWidgets[i]->getAssociatedObjectHandle()==objectHandle)
            return(oglWidgets[i]);
    }
    return(NULL);
}

COpenglOffscreen* getOffscreen(int objectHandle)
{
    for (size_t i=0;i<oglOffscreens.size();i++)
    {
        if (oglOffscreens[i]->getAssociatedObjectHandle()==objectHandle)
            return(oglOffscreens[i]);
    }
    return(NULL);
}

void removeOffscreen(int objectHandle)
{
    for (size_t i=0;i<oglOffscreens.size();i++)
    {
        if (oglOffscreens[i]->getAssociatedObjectHandle()==objectHandle)
        {
            delete oglOffscreens[i];
            oglOffscreens.erase(oglOffscreens.begin()+i);
            return;
        }
    }
}

VREP_DLLEXPORT unsigned char v_repStart(void* reservedPointer,int reservedInt)
{ // This is called just once, at the start of V-REP.

    // Dynamically load and bind V-REP functions:
     // ******************************************
     // 1. Figure out this plugin's directory:
     char curDirAndFile[1024];
 #ifdef _WIN32
     _getcwd(curDirAndFile, sizeof(curDirAndFile));
 #elif defined (__linux) || defined (__APPLE__)
     getcwd(curDirAndFile, sizeof(curDirAndFile));
 #endif
     std::string currentDirAndPath(curDirAndFile);
     // 2. Append the V-REP library's name:
     std::string temp(currentDirAndPath);
 #ifdef _WIN32
     temp+="/v_rep.dll";
 #elif defined (__linux)
     temp+="/libv_rep.so";
 #elif defined (__APPLE__)
     temp+="/libv_rep.dylib";
 #endif /* __linux || __APPLE__ */
     // 3. Load the V-REP library:
     vrepLib=loadVrepLibrary(temp.c_str());
     if (vrepLib==NULL)
     {
         std::cout << "Error, could not find or correctly load the V-REP library. Cannot start 'OpenGL3Renderer' plugin.\n";
         return(0); // Means error, V-REP will unload this plugin
     }
     if (getVrepProcAddresses(vrepLib)==0)
     {
         std::cout << "Error, could not find all required functions in the V-REP library. Cannot start 'OpenGL3Renderer' plugin.\n";
         unloadVrepLibrary(vrepLib);
         return(0); // Means error, V-REP will unload this plugin
     }
     // ******************************************

     // Check the version of V-REP:
     // ******************************************
     int vrepVer;
     simGetIntegerParameter(sim_intparam_program_version,&vrepVer);
     if (vrepVer<30201) // if V-REP version is smaller than 3.02.01
     {
         std::cout << "Sorry, your V-REP copy is somewhat old. Cannot start 'OpenGL3Renderer' plugin.\n";
         unloadVrepLibrary(vrepLib);
         return(0); // Means error, V-REP will unload this plugin
     }

     // Request Opengl 3.2
     QSurfaceFormat glFormat;
     glFormat.setVersion( 3, 2 );
     glFormat.setProfile( QSurfaceFormat::CoreProfile );
     glFormat.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
     glFormat.setRenderableType(QSurfaceFormat::OpenGL);
     glFormat.setRedBufferSize(8);
     glFormat.setGreenBufferSize(8);
     glFormat.setBlueBufferSize(8);
     glFormat.setAlphaBufferSize(0);
     glFormat.setStencilBufferSize(8);
     glFormat.setDepthBufferSize(24);
     QSurfaceFormat::setDefaultFormat(glFormat);

     return(2);  // initialization went fine, return the version number of this plugin!
}

VREP_DLLEXPORT void v_repEnd()
{ // This is called just once, at the end of V-REP
    for (size_t i=0;i<oglOffscreens.size();i++)
        delete oglOffscreens[i];
    oglOffscreens.clear();
    if (_qContext != NULL)
        delete _qContext;
    unloadVrepLibrary(vrepLib); // release the library
}

VREP_DLLEXPORT void* v_repMessage(int message,int* auxiliaryData,void* customData,int* replyData)
{ // This is called quite often. Just watch out for messages/events you want to handle

    // This function should not generate any error messages:
    int errorModeSaved;
    simGetIntegerParameter(sim_intparam_error_report_mode,&errorModeSaved);
    simSetIntegerParameter(sim_intparam_error_report_mode,sim_api_errormessage_ignore);

    void* retVal=NULL;

    if (message==sim_message_eventcallback_simulationabouttostart)
        simulationAboutToStart();
    if (message==sim_message_eventcallback_simulationended)
        simulationEnded();
    if(message==sim_message_eventcallback_guipass)
        simulationGuiPass();

    simSetIntegerParameter(sim_intparam_error_report_mode,errorModeSaved); // restore previous settings
    return(retVal);
}

void executeRenderCommands(bool windowed,int message,void* data)
{
    if (message==sim_message_eventcallback_extrenderer_start)
    {
        // Collect camera and environment data from V-REP:
        void** valPtr=(void**)data;
        resolutionX=((int*)valPtr[0])[0];
        resolutionY=((int*)valPtr[1])[0];
        float* backgroundColor=((float*)valPtr[2]);
        float viewAngle=((float*)valPtr[8])[0];
        perspectiveOperation=(((int*)valPtr[5])[0]==0);
        nearClippingPlane=((float*)valPtr[9])[0];
        farClippingPlane=((float*)valPtr[10])[0];
        float* amb=(float*)valPtr[11];
        C7Vector cameraTranformation(C4Vector((float*)valPtr[4]),C3Vector((float*)valPtr[3]));
        C4X4Matrix m4(cameraTranformation.getMatrix());
        float* fogBackgroundColor=(float*)valPtr[12];
        int fogType=((int*)valPtr[13])[0];
        float fogStart=((float*)valPtr[14])[0];
        float fogEnd=((float*)valPtr[15])[0];
        float fogDensity=((float*)valPtr[16])[0];
        bool fogEnabled=((bool*)valPtr[17])[0];
        float orthoViewSize=((float*)valPtr[18])[0];
        visionSensorOrCameraId=((int*)valPtr[19])[0];
        int posX=0;
        int posY=0;
        if ((valPtr[20]!=NULL)&&(valPtr[21]!=NULL))
        {
            posX=((int*)valPtr[20])[0];
            posY=((int*)valPtr[21])[0];
        }
        float fogDistance=((float*)valPtr[22])[0]; // pov-ray
        float fogTransp=((float*)valPtr[23])[0]; // pov-ray
        bool povFocalBlurEnabled=((bool*)valPtr[24])[0]; // pov-ray
        float povFocalDistance=((float*)valPtr[25])[0]; // pov-ray
        float povAperture=((float*)valPtr[26])[0]; // pov-ray
        int povBlurSamples=((int*)valPtr[27])[0]; // pov-ray

        if (_qContext == NULL){
            _qContext = new QOpenGLContext();
            _qContext->create();
        }

        if (lightContainer == NULL)
            lightContainer = new COcContainer<COcLight>();

        COpenglBase* oglItem=NULL;
        if (windowed&&_simulationRunning)
        {
            COpenglWidget* oglWidget=getWidget(visionSensorOrCameraId);
            if (oglWidget==NULL)
            {
                oglWidget=new COpenglWidget(visionSensorOrCameraId, _qContext);
                oglWidget->showAtGivenSizeAndPos(resolutionX,resolutionY,posX,posY);

                oglWidgets.push_back(oglWidget);

                oglWidget->makeContextCurrent();

                std::string glVersion = std::string((const char*)glGetString(GL_VERSION));
                float version = std::stof(glVersion);
                if (version < 3.2)
                    std::cout << "This renderer requires atleast OpenGL 3.2. The version available is: " << glVersion << std::endl;

                oglWidget->initGL();
            }
            // the window size can change, we return those values:
            oglWidget->getWindowResolution(resolutionX,resolutionY);
            ((int*)valPtr[0])[0]=resolutionX;
            ((int*)valPtr[1])[0]=resolutionY;

            oglItem=oglWidget;
        }
        else
        { // non-windowed
            COpenglOffscreen* oglOffscreen=getOffscreen(visionSensorOrCameraId);
            if (oglOffscreen!=NULL)
            {
                if (!oglOffscreen->isResolutionSame(resolutionX,resolutionY))
                {
                    removeOffscreen(visionSensorOrCameraId);
                    oglOffscreen=NULL;
                }
            }
            if (oglOffscreen==NULL)
            {
                oglOffscreen=new COpenglOffscreen(visionSensorOrCameraId,resolutionX,resolutionY, _qContext);
                oglOffscreens.push_back(oglOffscreen);

                oglOffscreen->makeContextCurrent();
                std::string glVersion = std::string((const char*)glGetString(GL_VERSION));
                float version = std::stof(glVersion);
                if (version < 3.2)
                    std::cout << "This renderer requires atleast OpenGL 3.2. The version available is: " << glVersion << std::endl;

                oglOffscreen->initGL();
            }
            oglItem=oglOffscreen;
        }

        activeBase = oglItem;

        if (oglItem!=NULL)
        {
            oglItem->makeContextCurrent();
            oglItem->clearBuffers(viewAngle,orthoViewSize,nearClippingPlane,farClippingPlane,perspectiveOperation,backgroundColor);

            // The following instructions have the same effect as gluLookAt()
            m4.inverse();
            m4.rotateAroundY(3.14159265359f);
            float m4_[4][4];
            m4.copyTo(m4_);
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
            QMatrix4x4 m44 = QMatrix4x4(
                        m4_[0][0], m4_[1][0], m4_[2][0], m4_[3][0],
                        m4_[0][1], m4_[1][1], m4_[2][1], m4_[3][1],
                        m4_[0][2], m4_[1][2], m4_[2][2], m4_[3][2],
                        m4_[0][3], m4_[1][3], m4_[2][3], m4_[3][3]);

            oglItem->m_shader->setUniformValue(
                        oglItem->m_shader->uniformLocation("view"), m44);

            QVector3D sceneAmbientLight = QVector3D(amb[0],amb[1],amb[2]);
            oglItem->m_shader->setUniformValue(
                        oglItem->m_shader->uniformLocation("sceneAmbient"), sceneAmbientLight);

            activeDirLightCounter=0;
            activePointLightCounter=0;
            activeSpotLightCounter=0;
        }
        if (lightShaders == NULL)
            lightShaders = new LightShaders();
    }

    if (message==sim_message_eventcallback_extrenderer_light)
    {
        // Collect light data from V-REP (one light at a time):
        void** valPtr=(void**)data;
        int lightType=((int*)valPtr[0])[0];
        float cutoffAngle=((float*)valPtr[1])[0];
        int spotExponent=((int*)valPtr[2])[0];
        float* colors=((float*)valPtr[3]);
        float constAttenuation=((float*)valPtr[4])[0];
        float linAttenuation=((float*)valPtr[5])[0];
        float quadAttenuation=((float*)valPtr[6])[0];
        C7Vector lightTranformation(C4Vector((float*)valPtr[8]),C3Vector((float*)valPtr[7]));
        float lightSize=((float*)valPtr[9])[0];
        float FadeXDistance=((float*)valPtr[10])[0]; // Pov-ray
        bool lightIsVisible=((bool*)valPtr[11])[0];
        bool noShadow=((bool*)valPtr[12])[0]; // Pov-ray
        int lightHandle=((int*)valPtr[13])[0];

        float nearPlane=0.01f;
        float farPlane=10.0f;
        float orthoSize=8.0f;
        int shadowTextureSize=2048;

        char* str=simGetExtensionString(lightHandle,-1,"nearPlane@lightProjection@openGL3");
        if (str!=nullptr)
        {
            nearPlane=strtof(str,nullptr);
            simReleaseBuffer(str);
        }
        str=simGetExtensionString(lightHandle,-1,"farPlane@lightProjection@openGL3");
        if (str!=nullptr)
        {
            farPlane=strtof(str,nullptr);
            simReleaseBuffer(str);
        }
        str=simGetExtensionString(lightHandle,-1,"orthoSize@lightProjection@openGL3");
        if (str!=nullptr)
        {
            orthoSize=strtof(str,nullptr);
            simReleaseBuffer(str);
        }
        str=simGetExtensionString(lightHandle,-1,"shadowTextureSize@lightProjection@openGL3");
        if (str!=nullptr)
        {
            shadowTextureSize=atoll(str);
            simReleaseBuffer(str);
        }

        if (_simulationRunning||(!windowed))
        { // Now set-up that light in OpenGl:
            C4X4Matrix m(lightTranformation.getMatrix());

            int counter = 0;
            if (lightType == sim_light_directional_subtype)
            {
                counter = activeDirLightCounter;
                activeDirLightCounter++;
            }
            else if (lightType == sim_light_omnidirectional_subtype)
            {
                counter = activePointLightCounter;
                activePointLightCounter++;
            }
            else if (lightType == sim_light_spot_subtype)
            {
                counter = activeSpotLightCounter;
                activeSpotLightCounter++;
            }

            //TODO: Will need to check if a light has been removed. If so, we need to re-set the values of the uniforms. (e.g. light len). Do check on the light handle.

            int totalCount = activeDirLightCounter + activePointLightCounter + activeSpotLightCounter;
            COcLight* light = lightContainer->getFromId(lightHandle);
            if(light == NULL){
                light = new COcLight(lightHandle, lightType, m, counter, totalCount, colors, constAttenuation, linAttenuation, quadAttenuation, cutoffAngle, spotExponent, nearPlane, farPlane, orthoSize, shadowTextureSize);
                lightContainer->add(light);
            } else {
                // We have the light, but check that it is in the expected position in the list.
                int i = lightContainer->getIndex(lightHandle);
                if (i + 1 != totalCount) {
                    // A light has been deleted...
                    // Means we need to rebuild all lights for this render. Just invalidate the rest of the lights...
                    lightContainer->removeAllFromIndex(i);
                    light = new COcLight(lightHandle, lightType, m, counter, totalCount, colors, constAttenuation, linAttenuation, quadAttenuation, cutoffAngle, spotExponent, nearPlane, farPlane, orthoSize, shadowTextureSize);
                    lightContainer->add(light);
                }
            }
            bool found = light->seenCamIds.size() != 0 && std::find(light->seenCamIds.begin(), light->seenCamIds.end(), activeBase->getAssociatedObjectHandle()) != light->seenCamIds.end();
            if (!found){
                light->seenCamIds.push_back(activeBase->getAssociatedObjectHandle());
                light->initForCamera(lightHandle, lightType, m, counter, totalCount, colors, constAttenuation, linAttenuation, quadAttenuation, cutoffAngle, spotExponent, nearPlane, farPlane, orthoSize, shadowTextureSize, activeBase->m_shader);
            }
            light->setPose(lightType, m, activeBase->m_shader);
            lightsToRender.push_back(light);
        }
    }

    if (message==sim_message_eventcallback_extrenderer_mesh)
    {
        // verticies come in as local space. What is tr?
        // Collect mesh data from V-REP:
        void** valPtr=(void**)data;
        float* vertices=((float*)valPtr[0]);
        int verticesCnt=((int*)valPtr[1])[0];
        int* indices=((int*)valPtr[2]);
        int triangleCnt=((int*)valPtr[3])[0];
        float* normals=((float*)valPtr[4]);
        int normalsCnt=((int*)valPtr[5])[0];
        float* colors=((float*)valPtr[8]);
        C7Vector tr(C4Vector((float*)valPtr[7]),C3Vector((float*)valPtr[6]));
        bool textured=((bool*)valPtr[18])[0];
        float shadingAngle=((float*)valPtr[19])[0];
        unsigned int meshId=((unsigned int*)valPtr[20])[0];
        bool translucid=((bool*)valPtr[21])[0];
        float opacityFactor=((float*)valPtr[22])[0];
        bool backfaceCulling=((bool*)valPtr[23])[0];
        int geomId=((int*)valPtr[24])[0];
        int texId=((int*)valPtr[25])[0];
        unsigned char* edges=((unsigned char*)valPtr[26]);
        bool visibleEdges=((bool*)valPtr[27])[0];
        // valPtr[28] is reserved
        int povPatternType=((int*)valPtr[29])[0]; // pov-ray
        int displayAttrib=((int*)valPtr[30])[0];
        const char* colorName=((char*)valPtr[31]);

        if (_simulationRunning||(!windowed))
        {
            float* texCoords=NULL;
            int texCoordCnt=0;
            bool repeatU=false;
            bool repeatV=false;
            bool interpolateColors=false;
            int applyMode=0;
            COcTexture* theTexture=NULL;
            if (textured)
            {
                // Read some additional data from V-REP (i.e. texture data):
                texCoords=((float*)valPtr[9]);
                texCoordCnt=((int*)valPtr[10])[0];
                unsigned char* textureBuff=((unsigned char*)valPtr[11]); // RGBA
                int textureSizeX=((int*)valPtr[12])[0];
                int textureSizeY=((int*)valPtr[13])[0];
                repeatU=((bool*)valPtr[14])[0];
                repeatV=((bool*)valPtr[15])[0];
                interpolateColors=((bool*)valPtr[16])[0];
                applyMode=((int*)valPtr[17])[0];

                theTexture=lightShaders->textureContainer->getFromId(texId);
                if (theTexture==NULL)
                {
                    theTexture=new COcTexture(texId,textureBuff,textureSizeX,textureSizeY);
                    lightShaders->textureContainer->add(theTexture);
                }
            }
            COcMesh* mesh=lightShaders->meshContainer->getFromId(geomId);
            if (mesh==NULL)
            {
                mesh=new COcMesh(geomId,vertices,verticesCnt*3,indices,triangleCnt*3,normals,normalsCnt*3,texCoords,texCoordCnt*2, edges);
                lightShaders->meshContainer->add(mesh);
            }
            mesh->store(tr,colors,textured,shadingAngle,translucid,opacityFactor,backfaceCulling,repeatU,repeatV,interpolateColors,applyMode,theTexture,visibleEdges);
            meshesToRender.push_back(mesh);
        }
    }

    if (message==sim_message_eventcallback_extrenderer_stop)
    {
        void** valPtr=(void**)data;
        unsigned char* rgbBuffer=((unsigned char*)valPtr[0]);
        float* depthBuffer=((float*)valPtr[1]);
        bool readRgb=((bool*)valPtr[2])[0];
        bool readDepth=((bool*)valPtr[3])[0];

#ifdef _WIN32
        static PFNGLACTIVETEXTUREPROC glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
#endif

        if(activeDirLightCounter == 0)
            activeBase->m_shader->setUniformValue(
                        activeBase->m_shader->uniformLocation("dirLightLen"), 0);
        if(activePointLightCounter == 0)
            activeBase->m_shader->setUniformValue(
                        activeBase->m_shader->uniformLocation("pointLightLen"), 0);
        if(activeSpotLightCounter == 0)
            activeBase->m_shader->setUniformValue(
                        activeBase->m_shader->uniformLocation("spotLightLen"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, activeBase->blankTexture);

        // Bind all of the Sampler2D and SampleCubes to an empty texture.
        int totalCount = activeDirLightCounter + activePointLightCounter + activeSpotLightCounter;
        for (int i = 0; i < MAX_LIGHTS; i++){
            QString depthCubeMaps = "depthCubeMap";
            depthCubeMaps.append(QString::number(i));
            activeBase->m_shader->setUniformValue(
                        activeBase->m_shader->uniformLocation(depthCubeMaps), 1);
        }

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, activeBase->blankTexture2);

        for (int i = 0; i < MAX_LIGHTS; i++){
            QString lightName = "spotLight";
            lightName.append(QString::number(i));
            lightName.append(".shadowMap");
            activeBase->m_shader->setUniformValue(
                        activeBase->m_shader->uniformLocation(lightName), 2);
        }

        for (int i = 0; i < MAX_LIGHTS; i++){
            QString lightName = "dirLight";
            lightName.append(QString::number(i));
            lightName.append(".shadowMap");
            activeBase->m_shader->setUniformValue(
                        activeBase->m_shader->uniformLocation(lightName), 2);
        }

        if (stepsSinceLastShadowMapRender >= oglWidgets.size() + oglOffscreens.size()){
            for (int i=0;i<int(lightsToRender.size());i++)
            {
                QOpenGLShaderProgram* depthSh = lightShaders->depthShader;
                if (lightsToRender[i]->lightType == sim_light_omnidirectional_subtype)
                    depthSh = lightShaders->omniDepthShader;

                lightsToRender[i]->renderDepthFromLight(depthSh, &meshesToRender);
            }
            stepsSinceLastShadowMapRender = 0;
        }
        stepsSinceLastShadowMapRender++;

        activeBase->makeContextCurrent();
        activeBase->bindFramebuffer();
        activeBase->clearViewport();
        activeBase->m_shader->bind();

        // It seems Sampler2Ds and SamplerCubes cant match to the same ID
        int omnis_seen = 0;
        int spots_seen = 0;
        int dir_seen = 0;

        // Bind all the lighting textures
        for (int i=0;i<int(lightsToRender.size());i++)
        {
            glActiveTexture(GL_TEXTURE3 + i);
            if (lightsToRender[i]->lightType == sim_light_omnidirectional_subtype){
                glBindTexture(GL_TEXTURE_CUBE_MAP, lightsToRender[i]->depthMap);
                QString depthCubeMaps = "depthCubeMap";
                depthCubeMaps.append(QString::number(omnis_seen));
                activeBase->m_shader->setUniformValue(
                            activeBase->m_shader->uniformLocation(depthCubeMaps), 3+i);
                omnis_seen += 1;
            }else{
                glBindTexture(GL_TEXTURE_2D, lightsToRender[i]->depthMap);
                if(lightsToRender[i]->lightType == sim_light_spot_subtype){
                    QString lightName = "spotLight";
                    lightName.append(QString::number(spots_seen));
                    lightName.append(".shadowMap");
                    activeBase->m_shader->setUniformValue(
                                activeBase->m_shader->uniformLocation(lightName), 3+i);
                    spots_seen += 1;
                }else if (lightsToRender[i]->lightType == sim_light_directional_subtype) {
                    QString lightName = "dirLight";
                    lightName.append(QString::number(dir_seen));
                    lightName.append(".shadowMap");
                    activeBase->m_shader->setUniformValue(
                                activeBase->m_shader->uniformLocation(lightName), 3+i);
                    dir_seen += 1;
                }
            }
        }

        glActiveTexture(GL_TEXTURE0);

        for (size_t i=0;i<meshesToRender.size();i++)
        {
            meshesToRender[i]->render(activeBase->m_shader);
        }

        lightsToRender.clear();
        meshesToRender.clear();

        if (windowed)
        {
            if (_simulationRunning)
            {
                COpenglWidget* oglWidget=getWidget(visionSensorOrCameraId);
                if (oglWidget!=NULL)
                {
                    if(oglWidget->isExposed())
                        oglWidget->swapBuffers();
                    oglWidget->doneCurrentContext();
                }
            }
        }
        else
        {
            COpenglOffscreen* oglOffscreen=getOffscreen(visionSensorOrCameraId);
            if (oglOffscreen!=NULL)
            {
                if (readRgb)
                {
                    glPixelStorei(GL_PACK_ALIGNMENT,1);
                    glReadPixels(0,0,resolutionX,resolutionY,GL_RGB,GL_UNSIGNED_BYTE,rgbBuffer);
                    glPixelStorei(GL_PACK_ALIGNMENT,4);
                }
                if (readDepth)
                {
                    glReadPixels(0,0,resolutionX,resolutionY,GL_DEPTH_COMPONENT,GL_FLOAT,depthBuffer);
                    // Convert this depth info into values corresponding to linear depths (if perspective mode):
                    if (perspectiveOperation)
                    {
                        float farMinusNear= farClippingPlane-nearClippingPlane;
                        float farDivFarMinusNear=farClippingPlane/farMinusNear;
                        float nearTimesFar=nearClippingPlane*farClippingPlane;
                        int v=resolutionX*resolutionY;
                        for (int i=0;i<v;i++)
                            depthBuffer[i]=((nearTimesFar/(farMinusNear*(farDivFarMinusNear-depthBuffer[i])))-nearClippingPlane)/farMinusNear;
                    }
                }
                oglOffscreen->doneCurrentContext();
//                oglOffscreen->unbindFramebuffer();
            }
        }

        if (_simulationRunning||(!windowed))
        {
            lightShaders->meshContainer->decrementAllUsedCount();
            lightShaders->meshContainer->removeAllUnused();
            lightShaders->textureContainer->decrementAllUsedCount();
            lightShaders->textureContainer->removeAllUnused();
            lightContainer->decrementAllUsedCount();
            lightContainer->removeAllUnused();
        }
    }
}

VREP_DLLEXPORT void v_repOpenGL3Renderer(int message,void* data)
{
    executeRenderCommands(false,message,data);
}

VREP_DLLEXPORT void v_repOpenGL3RendererWindowed(int message,void* data)
{
    executeRenderCommands(true,message,data);
}

