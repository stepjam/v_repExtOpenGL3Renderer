#include "mesh.h"
#include <qopengl.h>
#include <iostream>

#define MESH_INIT_USED_COUNT 10

unsigned int blankTexture;

Mesh::Mesh(int id,float* vert,int vertL,int* ind,int indL,float* norm,int normL,float* tex,int texL,unsigned char* ed)
{
    initializeOpenGLFunctions();
    for (int i=0;i<indL;i++)
    {
        Vertex vertex;
        vertex.Position = C3Vector(vert[0+3*ind[i]], vert[1+3*ind[i]], vert[2+3*ind[i]]);
        vertex.Normal = C3Vector(norm[0+3*i], norm[1+3*i], norm[2+3*i]);
        vertex.TexCoords[0] = 0.0f;
        vertex.TexCoords[1] = 0.0f;
        if (tex!=NULL)
        {
            vertex.TexCoords[0] = tex[0+2*i];
            vertex.TexCoords[1] = tex[1+2*i];
        }
        vertices.push_back(vertex);
    }

    indices.assign(ind,ind+indL);

    _id=id;
    _usedCount=MESH_INIT_USED_COUNT;

    setupMesh();

    // create a white texture if there is no texture
    glGenTextures(1,&blankTexture);
}

Mesh::~Mesh()
{
    glDeleteTextures(1, &blankTexture);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void Mesh::setupMesh()
{
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glBindVertexArray(0);
}

void Mesh::decrementUsedCount()
{
    _usedCount--;
}

int Mesh::getUsedCount()
{
    return(_usedCount);
}

int Mesh::getId()
{
    return(_id);
}

void Mesh::store(const C7Vector& tr,float* colors,bool textured,float shadingAngle,bool translucid,float opacityFactor,bool backfaceCulling,bool repeatU,bool repeatV,bool interpolateColors,int applyMode,Texture* texture,bool visibleEdges)
{
    this->tr = tr;
    this->colors = colors;
    this->textured = textured;
    this->shadingAngle = shadingAngle;
    this->translucid = translucid;
    this->opacityFactor = opacityFactor;
    this->backfaceCulling = backfaceCulling;
    this->repeatU = repeatU;
    this->repeatV = repeatV;
    this->interpolateColors = interpolateColors;
    this->applyMode = applyMode;
    this->texture = texture;
    this->visibleEdges = visibleEdges;
}

void Mesh::renderDepth(ShaderProgram* depthShader)
{
    // Set the model matrix
    C4Vector axis=tr.Q.getAngleAndAxisNoChecking();
    QMatrix4x4 mm;
    mm.setToIdentity();
    mm.translate(tr.X(0),tr.X(1),tr.X(2));
    mm.rotate(axis(0)*radToDeg,axis(1),axis(2),axis(3));
    depthShader->setUniformValue("model", mm);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glBindVertexArray(0);
}

void Mesh::render(ShaderProgram* m_shader)
{
    _usedCount=MESH_INIT_USED_COUNT;

    float shininess = 48.0f;

    // Set the model matrix
    C4Vector axis=tr.Q.getAngleAndAxisNoChecking();
    QMatrix4x4 mm;
    mm.setToIdentity();
    mm.translate(tr.X(0),tr.X(1),tr.X(2));
    mm.rotate(axis(0)*radToDeg,axis(1),axis(2),axis(3));
    m_shader->setUniformValue("model", mm);

    QVector3D ambientDiffuse = QVector3D(colors[0],colors[1],colors[2]);
    QVector3D specular = QVector3D(colors[6],colors[7],colors[8]);

    m_shader->setUniformValue("material.ambient", QVector4D(ambientDiffuse, translucid ? opacityFactor : 1.0));
    m_shader->setUniformValue("material.diffuse", ambientDiffuse);
    m_shader->setUniformValue("material.specular", specular);
    m_shader->setUniformValue("material.shininess", shininess);

    glActiveTexture(GL_TEXTURE0);
    if (textured&&(texture!=0))
    {
        texture->startTexture(repeatU,repeatV,interpolateColors,applyMode);
    }
    else
    {
        applyMode = -1;
        glBindTexture(GL_TEXTURE_2D, blankTexture);
        GLubyte texData[] = { 255, 255, 255, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    }
    m_shader->setUniformValue("texture0", 0);
    m_shader->setUniformValue("textureApplyMode", applyMode);

    m_shader->bind();

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0);
}

