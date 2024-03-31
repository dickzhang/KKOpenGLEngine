#include "GraphicsEngine.h"
#include "EngineUtils.h"
#include "Window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "InputProcessor.h"

GraphicsEngine::GraphicsEngine()
	: camera_(glm::vec3(0.0f,100.0f,-24.0f))
	,graphicsUtils_(camera_)
	,drawPass_(graphicsUtils_.SetupMainPass())
	,csmDepthPrepass_(graphicsUtils_.SetupCSMDepthPrepass())
	,depthPrepass_(graphicsUtils_.SetupLightCullingDepthPrepass())
	,computeShader_(graphicsUtils_.SetupLightCullingComputeShader(depthPrepass_))
	,debugLights_(graphicsUtils_.SetupDrawDebugLights())
	,drawDepthMap_(graphicsUtils_.SetupLightCullingDepthMapDrawPass(depthPrepass_))
{

}

void GraphicsEngine::Run()
{
	Window& mainWindow = Window::Get();
	InputProcessor inputProcessor;
	EngineUtils::SetupOpenGl();
	float lastFrame = 0.0f;

	while(!mainWindow.WindowShouldClose())
	{
		mainWindow.HandleResizeEvent();
		float currentFrame = glfwGetTime();
		camera_.ProcessInputForMovement(currentFrame-lastFrame);
		bool shouldRebuild = inputProcessor.ProcessInput();
		if(shouldRebuild)
		{
			Rebuild();
		}
		lastFrame = currentFrame;
		csmDepthPrepass_.Draw();

		if(EngineUtils::UseLightCulling)
		{
			PerformLightCulling();
		}

		if(EngineUtils::DrawLightCullingDepthMap)
		{
			drawDepthMap_.Draw();
		}
		else
		{
			drawPass_.Draw();
			if(EngineUtils::DrawDebugLights)
			{
				debugLights_.Draw();
			}
		}
		mainWindow.SwapBuffers();
		glfwPollEvents();
		EngineUtils::PrintFrameTime();
	}
	glfwTerminate();
}

void GraphicsEngine::PerformLightCulling()
{
	graphicsUtils_.GetPointLightBuffer().Bind();
	depthPrepass_.Draw();
	computeShader_.UseProgram();
	computeShader_.SetUniformValue("invViewProj",glm::inverse(camera_.GetProjectionMatrix()*camera_.GetViewMatrix()));
	depthPrepass_.GetDepthMap().BindTexture();
	computeShader_.Execute();
	computeShader_.Wait();
}

void GraphicsEngine::Rebuild()
{
	drawPass_ = graphicsUtils_.SetupMainPass();
	csmDepthPrepass_ = graphicsUtils_.SetupCSMDepthPrepass();
	depthPrepass_ = graphicsUtils_.SetupLightCullingDepthPrepass();
	computeShader_ = graphicsUtils_.SetupLightCullingComputeShader(depthPrepass_);
	debugLights_ = graphicsUtils_.SetupDrawDebugLights();
	drawDepthMap_ = graphicsUtils_.SetupLightCullingDepthMapDrawPass(depthPrepass_);
}
