#include "Texture.h"

#include <iostream>
#include <sstream>
#include <assert.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture()
: texture_()
{
}

void Texture::Init(const char* texturePath, unsigned int textureUnit)
{
    int width, height, nrChannels;
    //stbi_set_flip_vertically_on_load(true);  
    unsigned char *data = stbi_load(texturePath, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format;
        GLint param;
        switch (nrChannels)
        {
        case 1:
        {
            param = GL_REPEAT;
            format = GL_RED;
        }
        break;
        case 3:
        {
            param = GL_REPEAT;
            format = GL_RGB;
        }
        break;
        case 4:
        {
            param = GL_CLAMP_TO_EDGE;
            format = GL_RGBA;
        }
        break;
        default:
            break;
        }
        
        textureUnit_ = textureUnit;
        
        glGenTextures(1, &texture_);
        BindTexture();
        
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, param);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, param);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        std::stringstream ss(texturePath);
        std::string token;

        while(std::getline(ss, token, '/'))
        {
            name_ = token;
        }

        name_ = texturePath;

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture" << texturePath << '\n';
    }
}

void Texture::InitForWrite(unsigned int width, unsigned int height, unsigned int format, unsigned int textureUnit)
{
    textureUnit_ = textureUnit;
    glGenTextures(1, &texture_);
    BindTexture();

    GLenum type;

    switch (format)
    {
    case GL_RGB:
    case GL_RGBA:
    {
        type = GL_UNSIGNED_BYTE;
    }
    break;
    case GL_DEPTH_COMPONENT:
    {
        type = GL_FLOAT;
    }
    break;
    
    default:
        assert(("Ivalid texture write format", false));
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::BindTexture()
{
    glActiveTexture(GL_TEXTURE0 + textureUnit_);
    glBindTexture(GL_TEXTURE_2D, texture_);
}

Texture::~Texture()
{
}
