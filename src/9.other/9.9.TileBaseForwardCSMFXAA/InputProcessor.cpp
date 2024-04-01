#include "InputProcessor.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <thread>
#include <chrono>

#include "Window.h"
#include "EngineUtils.h"

using namespace std::chrono_literals;

bool InputProcessor::ProcessInput()
{
    const auto window = Window::Get();
    bool shouldRebuildRenderPasses = false;

    if (glfwGetKey(window.GetGLFWwindow(), GLFW_KEY_F) == GLFW_PRESS)
    {
        EngineUtils::UseLightCulling = !EngineUtils::UseLightCulling;
        shouldRebuildRenderPasses = true;
    }

    if (glfwGetKey(window.GetGLFWwindow(), GLFW_KEY_G) == GLFW_PRESS)
    {
        EngineUtils::DrawLightCullingDepthMap = !EngineUtils::DrawLightCullingDepthMap;
        std::this_thread::sleep_for(200ms); // hack to avoid flicker
    }

    if (glfwGetKey(window.GetGLFWwindow(), GLFW_KEY_H) == GLFW_PRESS)
    {
        EngineUtils::DrawDebugLights = !EngineUtils::DrawDebugLights;
        std::this_thread::sleep_for(200ms); // hack to avoid flicker
    }

    return shouldRebuildRenderPasses;
}
