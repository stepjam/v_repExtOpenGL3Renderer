TARGET = v_repExtOpenGL3Renderer
TEMPLATE = lib
DEFINES -= UNICODE
CONFIG += shared
QT     += widgets opengl printsupport #printsupport required from MacOS, otherwise crashes strangely ('This CONFIG += shared

win32 {
    DEFINES += WIN_VREP
    greaterThan(QT_MAJOR_VERSION,4) {
        greaterThan(QT_MINOR_VERSION,4) {
            LIBS += -lopengl32
        }
    }
}

macx {
    INCLUDEPATH += "/usr/local/include"
    DEFINES += MAC_VREP
}

unix:!macx {
    DEFINES += LIN_VREP
}


*-msvc* {
        QMAKE_CXXFLAGS += -O2
        QMAKE_CXXFLAGS += -W3
}
*-g++* {
        QMAKE_CXXFLAGS += -O3
        QMAKE_CXXFLAGS += -Wall
        QMAKE_CXXFLAGS += -Wno-unused-parameter
        QMAKE_CXXFLAGS += -Wno-strict-aliasing
        QMAKE_CXXFLAGS += -Wno-empty-body
        QMAKE_CXXFLAGS += -Wno-write-strings

        QMAKE_CXXFLAGS += -Wno-unused-but-set-variable
        QMAKE_CXXFLAGS += -Wno-unused-local-typedefs
        QMAKE_CXXFLAGS += -Wno-narrowing

        QMAKE_CFLAGS += -O3
        QMAKE_CFLAGS += -Wall
        QMAKE_CFLAGS += -Wno-strict-aliasing
        QMAKE_CFLAGS += -Wno-unused-parameter
        QMAKE_CFLAGS += -Wno-unused-but-set-variable
        QMAKE_CFLAGS += -Wno-unused-local-typedefs
}

INCLUDEPATH += "../include"
INCLUDEPATH += "../v_repMath"

SOURCES += \
    ../common/v_repLib.cpp \
    light.cpp \
    mesh.cpp \
    openglWindow.cpp \
    shaderProgram.cpp \
    texture.cpp \
    v_repExtOpenGL3Renderer.cpp \
    frameBufferObject.cpp \
    offscreenGlContext.cpp \
    openglOffscreen.cpp \
    openglBase.cpp \
    ../v_repMath/MyMath.cpp \
    ../v_repMath/3Vector.cpp \
    ../v_repMath/4Vector.cpp \
    ../v_repMath/6Vector.cpp \
    ../v_repMath/7Vector.cpp \
    ../v_repMath/3X3Matrix.cpp \
    ../v_repMath/4X4Matrix.cpp \
    ../v_repMath/6X6Matrix.cpp

HEADERS +=\
    ../include/v_repLib.h \
    container.h \
    light.h \
    mesh.h \
    openglWindow.h \
    shaderProgram.h \
    texture.h \
    v_repExtOpenGL3Renderer.h \
    frameBufferObject.h \
    offscreenGlContext.h \
    openglOffscreen.h \
    openglBase.h \
    ../v_repMath/MyMath.h \
    ../v_repMath/mathDefines.h \
    ../v_repMath/3Vector.h \
    ../v_repMath/4Vector.h \
    ../v_repMath/6Vector.h \
    ../v_repMath/7Vector.h \
    ../v_repMath/3X3Matrix.h \
    ../v_repMath/4X4Matrix.h \
    ../v_repMath/6X6Matrix.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

DISTFILES += \
    default.vert \
    default.frag \
    depth.frag \
    depth.vert \
    omni_depth.vert \
    omni_depth.frag

RESOURCES += \
    res.qrc



















