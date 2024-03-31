#include "CSMDepthPrepass.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "CSMShadowMaps.h"
#include "Window.h"

CSMDepthPrepass::CSMDepthPrepass(const CSMDepthPrepass& csmDepthPrepass)
    : shadowMaps_(csmDepthPrepass.shadowMaps_)
{
    Copy(csmDepthPrepass);
}

CSMDepthPrepass& CSMDepthPrepass::operator=(const CSMDepthPrepass& csmDepthPrepass)
{
    Copy(csmDepthPrepass);
    return *this;
}

CSMDepthPrepass::CSMDepthPrepass(
    const std::function<void(ShaderProgram&)>& Draw, ShaderProgram shader,
    CSMShadowMaps& shadowMaps,
    unsigned int resolution)
: RenderPass(Draw, shader)
, shadowResolution_(resolution)
, shadowMaps_(shadowMaps)
{
    framebuffer_.Init();
    framebuffer_.AttachTexture(GL_DEPTH_ATTACHMENT, shadowMaps_.GetShadowMapTextureId(), GL_TEXTURE_3D);
    assert(framebuffer_.IsComplete());
    Framebuffer::BindDefault();
}


void CSMDepthPrepass::PreDraw()
{
    shadowMaps_.GenerateShadows();
    shadowMaps_.BindShadowMapTexture();

    framebuffer_.Bind();
    glViewport(0, 0, shadowResolution_, shadowResolution_);
    glCullFace(GL_FRONT);
    glClear(GL_DEPTH_BUFFER_BIT);

    shader_.UseProgram();
    shader_.SetUniformValue("lightSpaceMatrices", shadowMaps_.GetMatrices());
}

void CSMDepthPrepass::PostDraw()
{
    const Window& window = Window::Get();
    glViewport(0, 0, window.GetWidth(), window.GetHeight());
    glCullFace(GL_BACK);
    Framebuffer::BindDefault();
}

void CSMDepthPrepass::Copy(const CSMDepthPrepass& csmDepthPrepass)
{
    this->drawFunc_ = csmDepthPrepass.drawFunc_;
    this->shader_ = csmDepthPrepass.shader_;
    this->framebuffer_ = csmDepthPrepass.framebuffer_;
    this->shadowResolution_ = csmDepthPrepass.shadowResolution_;
    this->shadowMaps_ = csmDepthPrepass.shadowMaps_;
}

CSMDepthPrepass::~CSMDepthPrepass()
{
}

