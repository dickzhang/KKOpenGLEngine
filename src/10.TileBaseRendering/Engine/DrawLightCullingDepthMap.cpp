#include "DrawLightCullingDepthMap.h"

#include "EngineUtils.h"
#include "Window.h"
#include "Texture.h"

DrawLightCullingDepthMap::DrawLightCullingDepthMap(const DrawLightCullingDepthMap& drawLightCullingDepthMap)
    : DrawableObject(drawLightCullingDepthMap)
    , deptMap_(drawLightCullingDepthMap.deptMap_)
{
}

DrawLightCullingDepthMap& DrawLightCullingDepthMap::operator=(const DrawLightCullingDepthMap& drawLightCullingDepthMap)
{
    DrawableObject::operator=(drawLightCullingDepthMap);
    this->deptMap_ = drawLightCullingDepthMap.deptMap_;
    return *this;
}

DrawLightCullingDepthMap::DrawLightCullingDepthMap(Texture& deptMap)
    : deptMap_(deptMap)
{
    struct ShadowDBGVertex
    {
        ShadowDBGVertex(glm::vec3 p, glm::vec2 t) : position(p), texCoord(t) {}
        glm::vec3 position;
        glm::vec2 texCoord;
    };
    std::vector<ShadowDBGVertex> quadVertices;
    quadVertices.push_back(ShadowDBGVertex(glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)));
    quadVertices.push_back(ShadowDBGVertex(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)));
    quadVertices.push_back(ShadowDBGVertex(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
    quadVertices.push_back(ShadowDBGVertex(glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)));

    vao_.Init();
    vao_.Bind();
    std::vector<VertexAttributeDescription> attribDescriptions;
    attribDescriptions.push_back(VertexAttributeDescription(3, false, VertexAttributeType::POSITION));
    attribDescriptions.push_back(VertexAttributeDescription(2, false, VertexAttributeType::TEX_COORDS));
    vbo_.Init(quadVertices, attribDescriptions);
    vbo_.Bind();
}

void DrawLightCullingDepthMap::Draw(ShaderProgram& shader)
{
    const Window& window = Window::Get();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window.GetWidth(), window.GetHeight());
    shader.UseProgram();
    deptMap_.BindTexture();
    vao_.Bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
