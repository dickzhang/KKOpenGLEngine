#include "DebugLightsPass.h"
#include "PCH.h"
#include "Camera.h"
#include "EngineUtils.h"

DebugLightsPass::DebugLightsPass(const DebugLightsPass& debugLightsPass)
	: camera_(debugLightsPass.camera_)
	, debugLights_(debugLightsPass.debugLights_)
{
}

DebugLightsPass& DebugLightsPass::operator=(const DebugLightsPass& debugLightsPass)
{
	this->camera_ = debugLightsPass.camera_;
	this->debugLights_ = debugLightsPass.debugLights_;
	return *this;
}

DebugLightsPass::DebugLightsPass(const std::vector<PointLight>& pointLights, Camera & camera)
	: RenderPass()
	, camera_(camera)
	, debugLights_(pointLights)
{
	ShaderData debugLightsVertexShaderData;
	debugLightsVertexShaderData.sourceCode = EngineUtils::ReadFile(FileSystem::getPath("resources/Shaders/lightDebug.vert"));

	ShaderData debugLightsfragmentShaderData;
	debugLightsfragmentShaderData.sourceCode = EngineUtils::ReadFile(FileSystem::getPath("resources/Shaders/lightDebug.frag"));

	ShaderProgram debugLightsShader;
	debugLightsShader.Init(&debugLightsVertexShaderData, &debugLightsfragmentShaderData);

	shader_ = debugLightsShader;
	drawFunc_ = [&](ShaderProgram& shader) {
		debugLights_.SetModel(glm::mat4(1.f));
		debugLights_.SetView(camera_.GetViewMatrix());
		debugLights_.SetProj(camera_.GetProjectionMatrix());
		debugLights_.Draw(shader_);
	};
}

void DebugLightsPass::PreDraw()
{
}

void DebugLightsPass::PostDraw()
{
}
