#include "LightCullingDepthPrepass.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "EngineUtils.h"
#include "Camera.h"
#include "Window.h"

LightCullingDepthPrepass::LightCullingDepthPrepass(const std::function<void(ShaderProgram&)>& Draw, ShaderProgram shader)
: RenderPass(Draw, shader)
{
    const Window& window = Window::Get();
    depthTexture_.InitForWrite(window.GetWidth(), window.GetHeight(), GL_DEPTH_COMPONENT, 2);
    depthTexture_.BindTexture();
    framebuffer_.Init(true);
    framebuffer_.Bind();
    framebuffer_.AttachTexture(GL_DEPTH_ATTACHMENT, depthTexture_.GetId(), GL_TEXTURE_2D);
    Framebuffer::BindDefault();
    assert(framebuffer_.IsComplete());
}

void LightCullingDepthPrepass::PreDraw()
{
    const Window& window = Window::Get();   
    framebuffer_.Bind();
    depthTexture_.BindTexture();
    glViewport(0, 0, window.GetWidth(), window.GetHeight());
    glClear(GL_DEPTH_BUFFER_BIT);

    shader_.UseProgram();
}

void LightCullingDepthPrepass::PostDraw()
{
    Framebuffer::BindDefault();
}

LightCullingDepthPrepass::~LightCullingDepthPrepass()
{
}

