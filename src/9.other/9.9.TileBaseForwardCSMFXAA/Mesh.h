#pragma once

#include <vector>

#include "Texture.h"
#include "VAO.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "MaterialData.h"
#include "DrawableObject.h"

class ShaderProgram;

class Mesh : public DrawableObject
{
public:
    Mesh();
    virtual ~Mesh() override;

    template <typename T>
    void Init(const std::vector<T>& vertices,
        const std::vector<VertexAttributeDescription>& attribDescriptions,
        const std::vector<unsigned int>& indices,
        const MaterialData& material);

    template<typename T>
    void Init(const std::vector<T>& vertices,
        const std::vector<VertexAttributeDescription>& attribDescriptions,
        const MaterialData& material);

    virtual void Draw(ShaderProgram& shader) override;

    const MaterialData& GetMaterial() const { return material_; }


private:
    MaterialData material_;
};

template<typename T>
void Mesh::Init(const std::vector<T>& vertices,
        const std::vector<VertexAttributeDescription>& attribDescriptions,
        const std::vector<unsigned int>& indices,
        const MaterialData& material)
{
    InitBuffers(vertices, attribDescriptions, indices);
    material_ = material;
    VAO::Unbid();
}

template<typename T>
void Mesh::Init(const std::vector<T>& vertices,
        const std::vector<VertexAttributeDescription>& attribDescriptions,
        const MaterialData& material)
{
    InitBuffers(vertices, attribDescriptions);
    material_ = material;
    VAO::Unbid();
}