#pragma once

#include <string>
#include <climits>

class Texture
{
public:
    Texture();
    ~Texture();

    void Init(const char* texturePath, unsigned int textureUnit);
    void InitForWrite(unsigned int width, unsigned int height, unsigned int format, unsigned int textureUnit);
    void BindTexture();
    unsigned int GetTextureUnit() const { return textureUnit_; }
    bool IsValid() const { return texture_ != UINT_MAX; }
    unsigned int GetId() const { return texture_; }

    const std::string& GetName() const { return name_; }

private:
    unsigned int texture_ = UINT_MAX;
    unsigned int textureUnit_ = 0;

    std::string name_;
};