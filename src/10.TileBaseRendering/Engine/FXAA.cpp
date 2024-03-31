#include "FXAA.h"

#include <vector>

#include "EngineUtils.h"
#include "Texture.h"

FXAA::FXAA(/* args */)
{
}

FXAA::~FXAA()
{
}

void FXAA::Init()
{
    vao_.Init();
    CreateVBO();
    CreateShaderProgram();
}

void FXAA::SetSceneTexture(const std::shared_ptr<Texture>& texture)
{
    sceneTexture_ = texture;
}

void FXAA::SetDepthTexture(const std::shared_ptr<Texture>& texture)
{
    depthTexture_ = texture;
}


void FXAA::Settup()
{

}

void FXAA::Apply()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    shader_.UseProgram();
    shader_.SetUniformValue("sceneTexture", (int)sceneTexture_->GetTextureUnit());
    shader_.SetUniformValue("depthTexture", (int)depthTexture_->GetTextureUnit());
    vao_.Bind();
    glDisable(GL_DEPTH_TEST);
    sceneTexture_->BindTexture();
    depthTexture_->BindTexture();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
}

void FXAA::CreateShaderProgram()
{
    ShaderData vertexShaderData;
    vertexShaderData.sourceCode = EngineUtils::ReadFile("./Shaders/FXAA.vert");
    ShaderData fragmentShaderData;
    fragmentShaderData.sourceCode = EngineUtils::ReadFile("./Shaders/FXAA.frag");

    shader_.Init(&vertexShaderData, &fragmentShaderData);
}

void FXAA::CreateVBO()
{
    // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    
    std::vector<FXAAVertex> vertices;
    vertices.push_back(FXAAVertex(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 1.0f)));
    vertices.push_back(FXAAVertex(glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 0.0f)));
    vertices.push_back(FXAAVertex(glm::vec2(1.0f, -1.0f), glm::vec2(1.0f, 0.0f)));
    vertices.push_back(FXAAVertex(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 1.0f)));
    vertices.push_back(FXAAVertex(glm::vec2(1.0f, -1.0f), glm::vec2(1.0f, 0.0f)));
    vertices.push_back(FXAAVertex(glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f)));
    vao_.Bind();

    std::vector<VertexAttributeDescription> attribDescriptions;
    attribDescriptions.push_back(VertexAttributeDescription(2, false, VertexAttributeType::POSITION));
    attribDescriptions.push_back(VertexAttributeDescription(2, false, VertexAttributeType::TEX_COORDS));

    vbo_.Init(vertices, attribDescriptions);
    vbo_.Bind();
}