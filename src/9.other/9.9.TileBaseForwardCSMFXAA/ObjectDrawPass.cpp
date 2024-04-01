#include "ObjectDrawPass.h"

#include <assert.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "EngineUtils.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "Window.h"
#include "CSMShadowMaps.h"

ObjectDrawPass::ObjectDrawPass(const ObjectDrawPass& objectDrawPass)
    : camera_(objectDrawPass.camera_)
    , light_(objectDrawPass.light_)
    , pointLights_(objectDrawPass.pointLights_)
    , shadowMaps_(objectDrawPass.shadowMaps_)
{
    Copy(objectDrawPass);
}

ObjectDrawPass& ObjectDrawPass::operator=(const ObjectDrawPass& objectDrawPass)
{
    Copy(objectDrawPass);
    return *this;
}

ObjectDrawPass::ObjectDrawPass(
    const std::function<void(ShaderProgram&)>& Draw, ShaderProgram shader,
    Camera& camera,
    DirectionalLight& light,
    std::vector<PointLight>& pointLights,
    const CSMShadowMaps& shadowMaps
)
: RenderPass(Draw, shader)
, camera_(camera)
, light_(light)
, pointLights_(pointLights)
, shadowMaps_(shadowMaps)
{
}

ObjectDrawPass::~ObjectDrawPass()
{
}

void ObjectDrawPass::PreDraw()
{
    if (isUsingFXAA())
    {
        framebuffer_->Bind();
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        Framebuffer::BindDefault();
    }
    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader_.UseProgram();
    shader_.SetUniformValue("viewPos", camera_.GetPosition());
    shader_.SetUniformValue("lightSpaceMatrices", shadowMaps_.GetMatrices());
    shader_.SetStorageBuffer("LightBuffer", 1);
    shader_.SetStorageBuffer("VisibleLightIndicesBuffer", 2);
    shader_.SetUniformValue("shadowMap", (int)shadowMaps_.GetShadowMapTextureId());
    shader_.SetUniformValue("light", light_);
    shader_.SetUniformValue("pointLights", pointLights_);
}

void ObjectDrawPass::PostDraw()
{
    if (isUsingFXAA())
    {
        fxaa_->Apply();
    }

    Framebuffer::BindDefault();
}

void ObjectDrawPass::Copy(const ObjectDrawPass& objectDrawPass)
{
    this->drawFunc_ = objectDrawPass.drawFunc_;
    this->shader_ = objectDrawPass.shader_;
    this->camera_ = objectDrawPass.camera_;
    this->light_ = objectDrawPass.light_;
    this->pointLights_ = objectDrawPass.pointLights_;
    this->framebuffer_ = objectDrawPass.framebuffer_;
    this->targetTexture_ = objectDrawPass.targetTexture_;
    this->depthTexture_ = objectDrawPass.depthTexture_;
    this->depthBuffer_ = objectDrawPass.depthBuffer_;
    this->fxaa_ = objectDrawPass.fxaa_;
}

void ObjectDrawPass::SetupCustomFramebuffer()
{
    const Window& window = Window::Get();
    targetTexture_ = std::make_shared<Texture>(Texture());
    targetTexture_->InitForWrite(window.GetWidth(), window.GetHeight(), GL_RGB, 3);
    
    depthTexture_ = std::make_shared<Texture>(Texture());
    depthTexture_->InitForWrite(window.GetWidth(), window.GetHeight(), GL_DEPTH_COMPONENT, 4);
    

    depthBuffer_ = Renderbuffer();
    depthBuffer_->Init(window.GetWidth(), window.GetHeight(), GL_DEPTH24_STENCIL8);

    framebuffer_ = Framebuffer();
    framebuffer_->Init();
    framebuffer_->Bind();

    framebuffer_->AttachTexture(GL_COLOR_ATTACHMENT0, targetTexture_->GetId(), GL_TEXTURE_2D);
    framebuffer_->AttachTexture(GL_DEPTH_ATTACHMENT, depthTexture_->GetId());
    
    assert(("Framebuffer of the Draw pass not complete!", framebuffer_->IsComplete()));


    Framebuffer::BindDefault();
}

void ObjectDrawPass::UseFXAA()
{
    SetupCustomFramebuffer();
    
    fxaa_ = FXAA();
    fxaa_->Init();
    fxaa_->SetSceneTexture(targetTexture_);
    fxaa_->SetDepthTexture(depthTexture_);
}


inline bool ObjectDrawPass::isUsingFXAA() const
{
    return fxaa_.has_value();
}