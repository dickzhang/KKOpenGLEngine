#include "TextureArray.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

TextureArray::TextureArray(/* args */)
: id_(0)
, textureUnit_(0)
{
}

void TextureArray::Init(unsigned int resolution, unsigned int textureUnit, int numLevels)
{
    textureUnit_ = textureUnit;
    glGenTextures(1, &id_);
    glBindTexture(GL_TEXTURE_2D_ARRAY, id_);
    glTexImage3D(
        GL_TEXTURE_2D_ARRAY,
        0,
        GL_DEPTH_COMPONENT32F,
        resolution,
        resolution,
        numLevels,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr);
        
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        
    constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);
}

void TextureArray::Bind()
{
    glActiveTexture(GL_TEXTURE0 + textureUnit_);
    glBindTexture(GL_TEXTURE_2D_ARRAY, id_);
}

TextureArray::~TextureArray()
{
}