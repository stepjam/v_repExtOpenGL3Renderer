#ifndef OCMESH_H
#define OCMESH_H

#include <vector>
#include "7Vector.h"
#include "ocTexture.h"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>

struct Vertex {
    // position
    C3Vector Position;
    // normal
    C3Vector Normal;
    // texCoords
    float TexCoords[2];
};

class COcMesh
{
public:
    COcMesh(int id,float* vert,int vertL,int* ind,int indL,float* norm,int normL,float* tex,int texL,unsigned char* ed);
    ~COcMesh();

    void renderDepth(QOpenGLShaderProgram* depthShader);
    void render(QOpenGLShaderProgram* m_shader);
    void store(const C7Vector& tr, float* colors,bool textured,float shadingAngle,bool translucid,float opacityFactor,bool backfaceCulling,bool repeatU,bool repeatV,bool interpolateColors,int applyMode,COcTexture* texture,bool visibleEdges);

    void decrementUsedCount();
    int getUsedCount();
    int getId();
protected:
    int _usedCount;
    int _id;
    unsigned int VAO, VBO, EBO;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
private:
    void setupMesh();

    C7Vector tr;
    float* colors;
    bool textured;
    float shadingAngle;
    bool translucid;
    float opacityFactor;
    bool backfaceCulling;
    bool repeatU;
    bool repeatV;
    bool interpolateColors;
    int applyMode;
    COcTexture* texture;
    bool visibleEdges;

};
#endif // OCMESH_H
