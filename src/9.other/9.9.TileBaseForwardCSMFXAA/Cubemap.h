#pragma once

#include <string>
#include <vector>

#include "DrawableObject.h"

class ShaderProgram;

class Cubemap : public DrawableObject
{
public:
    Cubemap(/* args */);
    virtual ~Cubemap() override;
    
    template<typename T>
    void Init(const std::vector<T>& vertices,
        const std::vector<VertexAttributeDescription>& attribDescriptions,
        const std::string& cubemapFolder);

    void Bind();
    virtual void Draw(ShaderProgram& shader) override;

private:

    void BindCubemapFaces(const std::string& cubemapFolder);
    void SpecifyWrappingAndFiltering();

    unsigned int id_;
};

template<typename T>
void Cubemap::Init(const std::vector<T>& vertices,
    const std::vector<VertexAttributeDescription>& attribDescriptions,
    const std::string& cubemapFolder)
{
    InitBuffers(vertices, attribDescriptions);
    glGenTextures(1, &id_);
    Bind();
    BindCubemapFaces(cubemapFolder);
    SpecifyWrappingAndFiltering(); 
}