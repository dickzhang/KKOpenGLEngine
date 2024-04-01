
#include "DrawDebugLights.h"
#include <glm/glm.hpp>
#include "EngineUtils.h"
#include "CubedSphere.h"


DrawDebugLights::DrawDebugLights(const DrawDebugLights& drawDebugLights)
    : DrawableObject(drawDebugLights)
{
}

DrawDebugLights& DrawDebugLights::operator=(const DrawDebugLights& drawDebugLights)
{
    DrawableObject::operator=(drawDebugLights);
    return *this;
}

DrawDebugLights::DrawDebugLights(const std::vector<PointLight>& pointLights)
{
    std::vector<DebugLightVertex> vertices;
    std::vector<unsigned int> indices;
    SetupDebugSpheres(pointLights, vertices, indices);
    std::vector<VertexAttributeDescription> attribDescriptions;
    attribDescriptions.push_back(VertexAttributeDescription(3, false, VertexAttributeType::POSITION));
    InitBuffers(vertices, attribDescriptions, indices);
}

DrawDebugLights::~DrawDebugLights()
{
}

void DrawDebugLights::Draw(ShaderProgram& shader)
{
    shader.UseProgram();
    vao_.Bind();
    SetupMatrices(shader);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLES, 0, vbo_.GetNumVertices());
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void DrawDebugLights::SetupDebugSpheres(
    const std::vector<PointLight>& pointLights,
    std::vector<DebugLightVertex>& vertices,
    std::vector<unsigned int>& indices)
{
    for (int i = 0; i < pointLights.size(); i++)
    {
        SetupVertices(pointLights[i], vertices);
    }
}

void DrawDebugLights::SetupVertices(const PointLight& pointLight, std::vector<DebugLightVertex>& vertices)
{
    CubedSphere cubedSphere(pointLight.position, pointLight.radius);
    auto sphereVertices = cubedSphere.GetVertices();
    vertices.insert(vertices.end(), sphereVertices.begin(), sphereVertices.end());
}