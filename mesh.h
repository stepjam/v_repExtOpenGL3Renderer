#ifndef MESH_H
#define MESH_H

#include <vector>
#include "7Vector.h"
#include "texture.h"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLExtraFunctions>
#include "shaderProgram.h"

struct Vertex {
    // position
    C3Vector Position;
    // normal
    C3Vector Normal;
    // texCoords
    float TexCoords[2];
};

class Mesh: protected QOpenGLExtraFunctions
{
public:
    Mesh(int id,float* vert,int vertL,int* ind,int indL,float* norm,int normL,float* tex,int texL,unsigned char* ed);
    ~Mesh();

    void renderDepth(ShaderProgram* depthShader);
    void render(ShaderProgram* m_shader);
    void store(const C7Vector& tr, float* colors,bool textured,float shadingAngle,bool translucid,float opacityFactor,bool backfaceCulling,bool repeatU,bool repeatV,bool interpolateColors,int applyMode,Texture* texture,bool visibleEdges);

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
    Texture* texture;
    bool visibleEdges;

};
#endif // MESH_H
