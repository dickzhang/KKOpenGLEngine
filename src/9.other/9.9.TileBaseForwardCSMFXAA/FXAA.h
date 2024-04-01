#pragma once

#include <memory>

#include "ShaderProgram.h"
#include "VAO.h"
#include "VertexBuffer.h"

class Texture;

class FXAA
{
public:
    FXAA();

    void Init();
    void SetSceneTexture(const  std::shared_ptr<Texture>& texture);
    void SetDepthTexture(const std::shared_ptr<Texture>& texture);

    void Settup();
    void Apply();

    ~FXAA();

private:
    
    void CreateShaderProgram();
    void CreateVBO();

    ShaderProgram shader_;
    VAO vao_;
    VertexBuffer vbo_;
    
    std::shared_ptr<Texture> sceneTexture_;
    std::shared_ptr<Texture> depthTexture_;
};