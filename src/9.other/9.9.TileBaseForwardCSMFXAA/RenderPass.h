#pragma once

#include <functional>

#include "ShaderProgram.h"

class RenderPass
{
public:
    RenderPass();
    RenderPass(const std::function<void(ShaderProgram&)>& Draw, ShaderProgram shader);

    RenderPass(const RenderPass&) = default;
    RenderPass& operator=(const RenderPass&) = default;

    void Draw();

    virtual ~RenderPass();

protected:

    virtual void PreDraw() = 0;
    virtual void PostDraw() = 0;

    ShaderProgram shader_;
    std::function<void(ShaderProgram&)> drawFunc_;
};