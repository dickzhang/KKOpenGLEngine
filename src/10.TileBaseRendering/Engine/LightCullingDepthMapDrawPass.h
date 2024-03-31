#pragma once

#include "RenderPass.h"
#include "ShaderProgram.h"
#include "DrawLightCullingDepthMap.h"

class Texture;

class LightCullingDepthMapDrawPass : public RenderPass
{
public:
	LightCullingDepthMapDrawPass(const LightCullingDepthMapDrawPass&) = default;
	LightCullingDepthMapDrawPass& operator=(const LightCullingDepthMapDrawPass&) = default;

	LightCullingDepthMapDrawPass(Texture& deptMap);

private:

	virtual void PreDraw() override;
	virtual void PostDraw() override;

	DrawLightCullingDepthMap drawDebugLights_;

};

