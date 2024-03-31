#pragma once

#include <functional>

#include "RenderPass.h"
#include "Framebuffer.h"

class CSMShadowMaps;

class CSMDepthPrepass : public RenderPass
{
public:
    CSMDepthPrepass(const CSMDepthPrepass&);
    CSMDepthPrepass& operator=(const CSMDepthPrepass&);


    CSMDepthPrepass(
        const std::function<void(ShaderProgram&)>& Draw, ShaderProgram shader,
        CSMShadowMaps& shadowMaps,
        unsigned int resolution);

    virtual ~CSMDepthPrepass() override;

    virtual void PreDraw() override;
    virtual void PostDraw() override;

private:
    void Copy(const CSMDepthPrepass&);

    Framebuffer framebuffer_;
    unsigned int shadowResolution_;
    CSMShadowMaps& shadowMaps_;
};