#pragma once
#include "RenderPass.h"
#include "DrawDebugLights.h"

class Camera;

class DebugLightsPass : public RenderPass
{
public:
	DebugLightsPass(const DebugLightsPass&);
	DebugLightsPass& operator=(const DebugLightsPass&);
	DebugLightsPass(const std::vector<PointLight>& pointLights, Camera & camera);

	virtual void PreDraw() override;
	virtual void PostDraw() override;

private:
	Camera& camera_;
	DrawDebugLights debugLights_;
};

