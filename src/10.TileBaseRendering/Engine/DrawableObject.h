#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "VAO.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

class ShaderProgram;

class DrawableObject
{
public:
    DrawableObject();
    DrawableObject(const DrawableObject&);
    DrawableObject& operator=(const DrawableObject&);
    virtual ~DrawableObject() = 0;

    void SetModel(const glm::mat4& model);
    void SetView(const glm::mat4& view);
    void SetProj(const glm::mat4& proj);
    void Translate(const glm::vec3& translation);
    void Scale(const glm::vec3& scale);

    virtual void Draw(ShaderProgram& shader) = 0;

protected:
    template<typename T>
    void InitBuffers(const std::vector<T>& vertices,
        const std::vector<VertexAttributeDescription>& attribDescriptions);

    template<typename T>
    void InitBuffers(const std::vector<T>& vertices,
        const std::vector<VertexAttributeDescription>& attribDescriptions,
        const std::vector<unsigned int>& indices);

    void SetupMatrices(ShaderProgram& shader);

    VAO vao_;
    VertexBuffer vbo_;
    IndexBuffer ibo_;

private:
    void CalculateMVP();
    void CalculateViewProj();
    void Copy(const DrawableObject&);

    glm::mat4 model_;
    glm::mat4 view_;
    glm::mat4 proj_;
    glm::mat4 modelViewProj_;
    glm::mat4 viewProj_;
};

template<typename T>
void DrawableObject::InitBuffers(const std::vector<T>& vertices,
        const std::vector<VertexAttributeDescription>& attribDescriptions)
{
    vao_.Init();
    vao_.Bind();

    vbo_.Init(vertices, attribDescriptions);
    vbo_.Bind(); 
}

template<typename T>
void DrawableObject::InitBuffers(const std::vector<T>& vertices,
        const std::vector<VertexAttributeDescription>& attribDescriptions,
        const std::vector<unsigned int>& indices)
{
    InitBuffers(vertices, attribDescriptions);
    ibo_.Init(indices);
    ibo_.Bind();
}