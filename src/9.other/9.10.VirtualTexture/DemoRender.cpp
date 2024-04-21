#include "DemoRender.h"
#include "Common.h"

ModuleBase* CreateAppModule()
{
	return new DemoRender();
}

DemoRender::DemoRender()
{

}

DemoRender::~DemoRender()
{

}

void DemoRender::Init()
{
	InitShader();
	GenerateMesh();
}

void DemoRender::InitShader()
{
}

void DemoRender::PreRender(Camera* camera)
{

}

void DemoRender::Render(Camera* camera,glm::vec2 mouseuv)
{
	float cameraNear = 0.1f;
	float cameraFar = 1000.0f;
	glm::mat4 projection = glm::perspective(glm::radians(60.0f),(float)WindowSize::SCR_WIDTH/(float)WindowSize::SCR_HEIGHT,cameraNear,cameraFar);
	glm::mat4 view = camera->GetViewMatrix();
}

void DemoRender::Exit()
{
}

void DemoRender::GenerateMesh()
{
	CreateVAO();
}

void DemoRender::CreateVAO()
{
}


