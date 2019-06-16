#ifndef TEXTURE_H
#define TEXTURE_H

#include <vector>
#include <QOpenGLExtraFunctions>

class Texture: protected QOpenGLExtraFunctions
{
public:
     Texture(int id,const unsigned char* textureBuff,int textureSizeX,int textureSizeY);
    ~Texture();

     void decrementUsedCount();
     int getUsedCount();
     int getId();

     void startTexture(bool repeatU,bool repeatV,bool interpolateColors,int applyMode);
     void endTexture();

protected:

     std::vector<unsigned char> _textureBuff;
     int _textureSizeX;
     int _textureSizeY;
     unsigned int _textureName;

     int _usedCount;
     int _id;
};
#endif // TEXTURE_H
