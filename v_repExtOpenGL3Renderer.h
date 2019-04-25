#ifndef V_REPEXTEXTERNALRENDERER_H
#define V_REPEXTEXTERNALRENDERER_H

#include <QtCore/qglobal.h>

#ifdef _WIN32
    #define VREP_DLLEXPORT extern "C" __declspec(dllexport)
#endif /* _WIN32 */
#if defined (__linux) || defined (__APPLE__)
    #define VREP_DLLEXPORT extern "C"
#endif /* __linux || __APPLE__ */

// The 3 required entry points of the plugin:
VREP_DLLEXPORT unsigned char v_repStart(void* reservedPointer,int reservedInt);
VREP_DLLEXPORT void v_repEnd();
VREP_DLLEXPORT void* v_repMessage(int message,int* auxiliaryData,void* customData,int* replyData);

VREP_DLLEXPORT void v_repExtRenderer(int message,void* data);
VREP_DLLEXPORT void v_repExtRendererWindowed(int message,void* data);

#endif // V_REPEXTEXTERNALRENDERER_H
