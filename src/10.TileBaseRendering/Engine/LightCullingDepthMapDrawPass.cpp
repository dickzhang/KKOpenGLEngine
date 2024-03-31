#include "LightCullingDepthMapDrawPass.h"
#include "PCH.h"
#include "EngineUtils.h"
#include "Camera.h"
#include "Texture.h"

LightCullingDepthMapDrawPass::LightCullingDepthMapDrawPass(Texture& deptMap)
	: RenderPass()
	, drawDebugLights_(deptMap)
{
    ShaderData vertexShader;
    vertexShader.sourceCode = EngineUtils::ReadFile(FileSystem::getPath("resources/Shaders/depthDBG.vert"));
    ShaderData fragmentShader;
    fragmentShader.sourceCode = EngineUtils::ReadFile(FileSystem::getPath("resources/Shaders/depthDBG.frag"));

    shader_.Init(&vertexShader, &fragmentShader);
    shader_.UseProgram();
    shader_.SetUniformValue("nearPlane", Camera::NEAR_PLANE);
    shader_.SetUniformValue("farPlane", Camera::FAR_PLANE);
    shader_.SetUniformValue("depthMap", (int)deptMap.GetTextureUnit());

    drawFunc_ = [&](ShaderProgram& shader) {
        drawDebugLights_.Draw(shader);
    };
}

void LightCullingDepthMapDrawPass::PreDraw()
{
}

void LightCullingDepthMapDrawPass::PostDraw()
{
}
