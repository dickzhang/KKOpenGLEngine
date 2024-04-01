#pragma once

#include <vector>
#include <type_traits>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

enum VertexAttributeType
{
    POSITION,
    NORMALS,
    TEX_COORDS
};

struct VertexAttributeDescription
{
    VertexAttributeDescription(unsigned int numElements, float isNormalised, VertexAttributeType vType)
    : numOfElements(numElements)
    , normalised(isNormalised)
    , type(vType)
    {}

    unsigned int numOfElements;
    bool normalised;
    VertexAttributeType type;
};

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct FXAAVertex
{
    glm::vec2 Position;
    glm::vec2 TexCoords;

    FXAAVertex(const glm::vec2& position, const glm::vec2& texCoord)
    : Position(position)
    , TexCoords(texCoord)
    {}
};


class VertexBuffer
{
public:
    VertexBuffer();
    ~VertexBuffer();

    template <typename T>
    void Init(
        const std::vector<T>& vertices,
        const std::vector<VertexAttributeDescription>& attribDescriptions);

    unsigned int GetId() const { return id_; }
    unsigned int GetNumVertices() const { return numVertices_; }
    void Bind();

    static void AddVertexAtributeDescription(unsigned int index, unsigned int numElements,
 	                                  unsigned int elementType, bool normalized,
 	                                  int stride, const void * pointer);

private:

    unsigned int id_;
    unsigned long numVertices_;
};

template<typename T>
void VertexBuffer::Init(
    const std::vector<T>& vertices,
    const std::vector<VertexAttributeDescription>& attribDescriptions)
{
    glGenBuffers(1, &id_);
    glBindBuffer(GL_ARRAY_BUFFER, id_);

    glBufferData(GL_ARRAY_BUFFER, sizeof(T) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    int vertexAttribSize = 0;
    for (const auto& desc : attribDescriptions)
    {
        vertexAttribSize += desc.numOfElements;
    }

    vertexAttribSize *= sizeof(float);

    int currentStride = 0;
    for (int i = 0; i < attribDescriptions.size(); i++)
    {
        const auto& desc = attribDescriptions[i];
        int strideCpy = currentStride;
        AddVertexAtributeDescription(i, desc.numOfElements, GL_FLOAT, desc.normalised, vertexAttribSize, (void*)(strideCpy));
        currentStride += desc.numOfElements * sizeof(float);
    }

    numVertices_ = vertices.size();
}