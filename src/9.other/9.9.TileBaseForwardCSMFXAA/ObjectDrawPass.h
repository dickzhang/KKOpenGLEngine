#pragma once

#include <optional>
#include <memory>

#include "RenderPass.h"
#include "ShaderProgram.h"
#include "Framebuffer.h"
#include "Texture.h"
#include "Renderbuffer.h"
#include "FXAA.h"
#include "PointLight.h"

class Camera;
class DirectionalLight;
class Window;
class CSMShadowMaps;

class ObjectDrawPass : public RenderPass
{
public:
    ObjectDrawPass() = default;
    ObjectDrawPass(const ObjectDrawPass&);
    ObjectDrawPass& operator=(const ObjectDrawPass&);

    ObjectDrawPass(
        const std::function<void(ShaderProgram&)>& Draw,
        ShaderProgram shader,
        Camera& camera,
        DirectionalLight& light,
        std::vector<PointLight>& pointLights,
        const CSMShadowMaps& shadowMaps
    );

    virtual ~ObjectDrawPass() override;

    virtual void PreDraw() override;
    virtual void PostDraw() override;

    void UseFXAA();


private: 

    void Copy(const ObjectDrawPass&);

    void SetupCustomFramebuffer();
    inline bool isUsingFXAA() const;
    
    Camera& camera_;
    DirectionalLight& light_;
    std::vector<PointLight>& pointLights_;
    
    std::optional<Framebuffer> framebuffer_;
    std::shared_ptr<Texture> targetTexture_;
    std::shared_ptr<Texture> depthTexture_;
    std::optional<Renderbuffer> depthBuffer_;

    std::optional<FXAA> fxaa_;

    const CSMShadowMaps& shadowMaps_;
};