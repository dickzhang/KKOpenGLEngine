#pragma once

#include "RenderPass.h"
#include "Framebuffer.h"
#include "Texture.h"

class LightCullingDepthPrepass : public RenderPass
{
public:
    LightCullingDepthPrepass(const std::function<void(ShaderProgram&)>& Draw, ShaderProgram shader);
    virtual ~LightCullingDepthPrepass() override;

    Texture& GetDepthMap() { return depthTexture_; }

    virtual void PreDraw() override;
    virtual void PostDraw() override;

private:

    Framebuffer framebuffer_;
    Texture depthTexture_;

};