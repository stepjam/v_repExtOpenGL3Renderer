#include "v_repExtOpenGL3Renderer.h"
#include "v_repLib.h"
#include "4X4Matrix.h"
#include <iostream>
#include "openglWidget.h"
#include "openglOffscreen.h"
#include "ocLight.h"


#ifdef _WIN32
    #include <direct.h>
#endif /* _WIN32 */

#if defined (__linux) || defined (__APPLE__)
    #include <unistd.h>
    #include <string.h>
    #define _stricmp(x,y) strcasecmp(x,y)
#endif

LIBRARY vrepLib;

int resolutionX;
int resolutionY;
float nearClippingPlane;
float farClippingPlane;
bool perspectiveOperation;
int visionSensorOrCameraId;

int activeDirLightCounter, activePointLightCounter, activeSpotLightCounter;

bool _simulationRunning=false;
std::vector<COpenglWidget*> oglWidgets;
std::vector<COpenglOffscreen*> oglOffscreens;

COpenglBase* activeBase = NULL;
QVector3D sceneAmbientLight;

std::vector<COcLight*> lightsToRender;
std::vector<COcMesh*> meshesToRender;

void simulationAboutToStart()
{
    _simulationRunning=true;
}

void simulationEnded()
{ // Windowed views can only run while simulation is running
    for (size_t i=0;i<oglWidgets.size();i++)
        delete oglWidgets[i];
    oglWidgets.clear();
    _simulationRunning=false;
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

    return(2);  // initialization went fine, return the version number of this plugin!
}

VREP_DLLEXPORT void v_repEnd()
{ // This is called just once, at the end of V-REP
    for (size_t i=0;i<oglOffscreens.size();i++)
        delete oglOffscreens[i];
    oglOffscreens.clear();

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

        COpenglBase* oglItem=NULL;
        if (windowed&&_simulationRunning)
        {
            COpenglWidget* oglWidget=getWidget(visionSensorOrCameraId);
            if (oglWidget==NULL)
            {
                // Request Opengl 3.2
                QGLFormat glFormat;
                glFormat.setVersion( 3, 2 );
                glFormat.setProfile( QGLFormat::CoreProfile );
                glFormat.setSampleBuffers( true );

                oglWidget=new COpenglWidget(visionSensorOrCameraId, glFormat);
                oglWidget->show();
                oglWidgets.push_back(oglWidget);

                oglWidget->makeContextCurrent();
                std::string glVersion = std::string((const char*)glGetString(GL_VERSION));
                float version = std::stof(glVersion);
                if (version < 3.2)
                    std::cout << "This renderer requires atleast OpenGL 3.2. The version available is: " << glVersion << std::endl;

                oglWidget->initGL();
                oglWidget->showAtGivenSizeAndPos(resolutionX,resolutionY,posX,posY);
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
                // Request Opengl 3.2
                QSurfaceFormat glFormat;
                glFormat.setVersion( 3, 2 );
                glFormat.setProfile( QSurfaceFormat::CoreProfile );
                oglOffscreen=new COpenglOffscreen(visionSensorOrCameraId,resolutionX,resolutionY,glFormat);
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

            oglItem->m_shader->setUniformValue(
                        oglItem->m_shader->uniformLocation("dirLightLen"), 0);
            oglItem->m_shader->setUniformValue(
                        oglItem->m_shader->uniformLocation("pointLightLen"), 0);
            oglItem->m_shader->setUniformValue(
                        oglItem->m_shader->uniformLocation("spotLightLen"), 0);
        }
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
            int totalCount = activeDirLightCounter + activePointLightCounter + activeSpotLightCounter;
            COcLight* light = new COcLight(lightType, m, counter, totalCount, colors, constAttenuation, linAttenuation, quadAttenuation, cutoffAngle, spotExponent, activeBase->m_shader);
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

                theTexture=activeBase->textureContainer->getFromId(texId);
                if (theTexture==NULL)
                {
                    theTexture=new COcTexture(texId,textureBuff,textureSizeX,textureSizeY);
                    activeBase->textureContainer->add(theTexture);
                }
            }
            COcMesh* mesh=activeBase->meshContainer->getFromId(geomId);
            if (mesh==NULL)
            {
                mesh=new COcMesh(geomId,vertices,verticesCnt*3,indices,triangleCnt*3,normals,normalsCnt*3,texCoords,texCoordCnt*2, edges);
                activeBase->meshContainer->add(mesh);
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

        for (int i=0;i<int(lightsToRender.size());i++)
        {
            QOpenGLShaderProgram* depthSh = activeBase->depthShader;
            if (lightsToRender[i]->lightType == sim_light_omnidirectional_subtype)
                depthSh = activeBase->omniDepthShader;
            lightsToRender[i]->renderDepthFromLight(depthSh, &meshesToRender);
        }

        activeBase->clearViewport();
        activeBase->m_shader->bind();

        // Bind all the lighting textures
        for (int i=0;i<int(lightsToRender.size());i++)
        {
            glActiveTexture(GL_TEXTURE1 + i);
            if (lightsToRender[i]->lightType == sim_light_omnidirectional_subtype){
                glBindTexture(GL_TEXTURE_CUBE_MAP, lightsToRender[i]->depthMap);
            }else
                glBindTexture(GL_TEXTURE_2D, lightsToRender[i]->depthMap);
        }

        activeBase->bindFramebuffer(); // Only has an effect on offscreen rendering
        for (size_t i=0;i<meshesToRender.size();i++)
        {
            meshesToRender[i]->render(activeBase->m_shader);
        }

        for (int i=0;i<int(lightsToRender.size());i++)
            delete lightsToRender[i];
        lightsToRender.clear();
        meshesToRender.clear();

        if (windowed)
        {
            if (_simulationRunning)
            {
                COpenglWidget* oglWidget=getWidget(visionSensorOrCameraId);
                if (oglWidget!=NULL)
                {
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
            }
        }

        if (_simulationRunning||(!windowed))
        {
            activeBase->meshContainer->decrementAllUsedCount();
            activeBase->meshContainer->removeAllUnused();
            activeBase->textureContainer->decrementAllUsedCount();
            activeBase->textureContainer->removeAllUnused();
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
